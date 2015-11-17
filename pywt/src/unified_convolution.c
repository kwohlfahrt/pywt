#include "convolution.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

double timediff(struct timespec const start, struct timespec const end){
    return ((double) (end.tv_sec - start.tv_sec)) + ((double) (end.tv_nsec - start.tv_nsec) * 1e-9);
}

static inline size_t ceil_div(size_t numerator, size_t denominator){
    return numerator / denominator + (numerator % denominator != 0);
}

static inline void unified_convolution(const float * const input, const size_t I,
                                       const float * const filter, const size_t F,
                                       float * const output, const size_t O,
                                       const size_t input_upsampling,
                                       const size_t output_downsampling,
                                       const size_t filter_upsampling,
                                       const MODE edge_extension){
#define INPUT_IDX ((o * output_downsampling / input_upsampling) + input_start)
#define FILTER_IDX (f * input_upsampling + o % input_upsampling)

    const size_t input_start = ((output_downsampling - 1));

    size_t o;
    for (o = 0; INPUT_IDX < F * filter_upsampling / input_upsampling
             && INPUT_IDX < I && o < O; o++) {
        for (size_t f = 0; f * filter_upsampling <= INPUT_IDX; ++f)
            output[o] += input[INPUT_IDX - f * filter_upsampling] * filter[FILTER_IDX];

        switch (edge_extension) {
        case MODE_SYMMETRIC:{
            size_t f = INPUT_IDX / filter_upsampling + 1;
            while (FILTER_IDX < F){
                for (size_t k = filter_upsampling - 1 - INPUT_IDX % filter_upsampling;
                     k < I && FILTER_IDX < F; ++f, k += filter_upsampling)
                    output[o] += input[k] * filter[FILTER_IDX];
                for (size_t k = filter_upsampling - 1 - INPUT_IDX % filter_upsampling;
                     k < I && FILTER_IDX < F; ++f, k += filter_upsampling)
                    output[o] += input[I-1-k] * filter[FILTER_IDX];
            }
            break;
        }
        case MODE_CONSTANT_EDGE:
            for (size_t f = INPUT_IDX / filter_upsampling + 1; FILTER_IDX < F; ++f)
                output[o] += input[0] * filter[FILTER_IDX];
            break;
        case MODE_SMOOTH:
            for (size_t f = INPUT_IDX / filter_upsampling + 1, k = 1; FILTER_IDX < F; ++f, ++k)
                output[o] += (input[0] + k * (input[0] - input[1])) * filter[FILTER_IDX];
            break;
        case MODE_PERIODIC:{
            size_t f = INPUT_IDX / filter_upsampling + 1;
            while (FILTER_IDX < F)
                for (size_t k = filter_upsampling - 1 - INPUT_IDX % filter_upsampling;
                     k < I && FILTER_IDX < F; f++, k += filter_upsampling){
                    output[o] += input[I-1-k] * filter[FILTER_IDX];
                }
            break;
        }
        case MODE_ZEROPAD:
        default:
            break;
        }
    }

    for (; INPUT_IDX < I && o < O; o++)
        for (size_t f = 0; FILTER_IDX < F; f++)
            output[o] += input[INPUT_IDX - f * filter_upsampling] * filter[FILTER_IDX];

    // TODO: Handle overlapping on both sides
    // Factor out edge handling to make easier.

    for (; INPUT_IDX < I + F * filter_upsampling / input_upsampling - 1
             && o < O; o++){
        switch (edge_extension) {
        case MODE_SYMMETRIC:
            for (size_t f = 0; INPUT_IDX - f * filter_upsampling >= I; ++f){
                size_t input_idx = (INPUT_IDX - f * filter_upsampling) % I;
                bool mirror = (((INPUT_IDX - f * filter_upsampling) / I) % 2) != 0;
                if (mirror)
                    output[o] += input[I - 1 - input_idx] * filter[FILTER_IDX];
                else
                    output[o] += input[input_idx] * filter[FILTER_IDX];
            }
            break;
        case MODE_CONSTANT_EDGE:
            for (size_t f = 0; INPUT_IDX - f * filter_upsampling >= I; ++f)
                output[o] += input[I-1] * filter[FILTER_IDX];
            break;
        case MODE_SMOOTH:
            for (size_t f = 0, k = INPUT_IDX - (I - 1);
                 INPUT_IDX - f * filter_upsampling >= I; ++f, --k)
                output[o] += (input[I-1] + k * (input[I-1] - input[I-2])) * filter[FILTER_IDX];
            break;
        case MODE_PERIODIC:
            for (size_t f = 0; INPUT_IDX - f * filter_upsampling >= I; ++f)
                output[o] += input[(INPUT_IDX - f * filter_upsampling) % I]
                    * filter[FILTER_IDX];
            break;
        case MODE_ZEROPAD:
        default:
            break;
        }

        for (size_t f = (INPUT_IDX - I) / filter_upsampling + 1; FILTER_IDX < F; f++)
            output[o] += input[INPUT_IDX - f * filter_upsampling] * filter[FILTER_IDX];
    }
#undef INPUT_IDX
#undef FILTER_IDX
}

void printArray(const float * const array, const size_t N){
    for (size_t i = 0; i < N; ++i)
        printf(array[i] ? "% 6.2f " : "% 6.0f ", array[i]);
    puts("");
}

#define NELEMS(x) (sizeof(x) / sizeof(*x))
int main(void){
    const size_t I = 12;
    float * const input = malloc(I * sizeof(*input));
    for (size_t i = 0; i < I; ++i)
        input[i] = i + 1;

    const size_t F = 6;
    float * const filter = malloc(F * sizeof(*filter));
    for (size_t f = 0; f < F; ++f)
        filter[f] = f + 1;

    puts("Input:");
    printArray(input, I);
    puts("Filter:");
    printArray(filter, F);

    const size_t O = 40;
    float * const output_new = malloc(O * sizeof(*output_new));

    const size_t repeats = 1000000;
    struct timespec start, end;
    size_t const output_downsample = 3, input_upsample = 3, filter_upsample = 3;

    clock_gettime(CLOCK_REALTIME, &start);
    for (size_t i = 0; i < repeats; i++)
        unified_convolution(input, I, filter, F, output_new, O,
                            1, output_downsample, 1, MODE_PERIODIC);
    clock_gettime(CLOCK_REALTIME, &end);
    printf("Downsampling convolution (%5.2f s):\n",
           timediff(start, end));
    memset(output_new, 0, O * sizeof(*output_new));
    unified_convolution(input, I, filter, F, output_new, O,
                        1, output_downsample, 1, MODE_PERIODIC);
    printArray(output_new, O);

    clock_gettime(CLOCK_REALTIME, &start);
    for (size_t i = 0; i < repeats; i++)
        unified_convolution(input, I, filter, F, output_new, O,
                            filter_upsample, 1, 1, MODE_PERIODIC);
    clock_gettime(CLOCK_REALTIME, &end);
    memset(output_new, 0, O * sizeof(*output_new));
    unified_convolution(input, I, filter, F, output_new, O,
                        filter_upsample, 1, 1, MODE_PERIODIC);
    printf("Upsampling convolution (%5.2f s):\n",
           timediff(start, end));
    printArray(output_new, O);

    clock_gettime(CLOCK_REALTIME, &start);
    for (size_t i = 0; i < repeats; i++)
        unified_convolution(input, I, filter, F, output_new, O,
                            1, 1, filter_upsample, MODE_PERIODIC);
    clock_gettime(CLOCK_REALTIME, &end);
    printf("Upsampled filter convolution (%5.2f s):\n",
           timediff(start, end));
    memset(output_new, 0, O * sizeof(*output_new));
    unified_convolution(input, I, filter, F, output_new, O,
                        1, 1, filter_upsample, MODE_PERIODIC);
    printArray(output_new, O);

    free(input);
    free(filter);
    free(output_new);

    return 0;
}
#undef NELEMS
