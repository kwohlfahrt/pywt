/* Copyright (c) 2006-2012 Filip Wasilewski <http://en.ig.ma/> */
/* See COPYING for license details. */

#include "templating.h"

#ifndef TYPE
#error TYPE must be defined here.
#else

#include "wt.h"

#if defined _MSC_VER
#define restrict __restrict
#elif defined __GNUC__
#define restrict __restrict__
#endif

/* Decomposition of input with lowpass filter */

int CAT(TYPE, _downcoef_axis)(const TYPE * const restrict input, const ArrayInfo input_info,
                              TYPE * const restrict output, const ArrayInfo output_info,
                              const Wavelet * const restrict wavelet, const size_t axis,
                              const Coefficient coef, const MODE mode){
    size_t i;
    size_t num_loops = 1;
    TYPE * temp_input = NULL, * temp_output = NULL;

    // These are boolean values, but MSVC does not have <stdbool.h>
    int make_temp_input, make_temp_output;

    if (input_info.ndim != output_info.ndim)
        return 1;
    if (axis >= input_info.ndim)
        return 1;

    for (i = 0; i < input_info.ndim; ++i){
        if (i == axis){
            size_t output_len = dwt_buffer_length_o(input_info.shape[i],
                                                    wavelet->dec_len,
                                                    coef, mode);
            if ((output_len == 0) || (output_len != output_info.shape[i]))
                return 1;
        } else {
            if (input_info.shape[i] != output_info.shape[i])
                return 1;
        }
    }

    make_temp_input = input_info.strides[axis] != sizeof(TYPE);
    make_temp_output = output_info.strides[axis] != sizeof(TYPE);
    if (make_temp_input)
        if ((temp_input = malloc(input_info.shape[axis] * sizeof(TYPE))) == NULL)
            goto cleanup;
    if (make_temp_output)
        if ((temp_output = malloc(output_info.shape[axis] * sizeof(TYPE))) == NULL)
            goto cleanup;

    for (i = 0; i < output_info.ndim; ++i){
        if (i != axis)
            num_loops *= output_info.shape[i];
    }

    for (i = 0; i < num_loops; ++i){
        size_t j;
        size_t input_offset = 0, output_offset = 0;
        const TYPE * input_row;
        TYPE * output_row;

        // Calculate offset into linear buffer
        {
            size_t reduced_idx = i;
            for (j = 0; j < output_info.ndim; ++j){
                size_t j_rev = output_info.ndim - 1 - j;
                if (j_rev != axis){
                    size_t axis_idx = reduced_idx % output_info.shape[j_rev];
                    reduced_idx /= output_info.shape[j_rev];

                    input_offset += (axis_idx * input_info.strides[j_rev]);
                    output_offset += (axis_idx * output_info.strides[j_rev]);
                }
            }
        }

        // Copy to temporary input if necessary
        if (make_temp_input)
            for (j = 0; j < input_info.shape[axis]; ++j)
                // Offsets are byte offsets, to need to cast to char and back
                temp_input[j] = *(TYPE *)(((char *) input) + input_offset
                                          + j * input_info.strides[axis]);

        // Select temporary or direct output and input
        input_row = make_temp_input ? temp_input
            : (const TYPE *)((const char *) input + input_offset);
        output_row = make_temp_output ? temp_output
            : (TYPE *)((char *) output + output_offset);

        // Apply along axis
        switch (coef){
        case COEF_APPROX:
            CAT(TYPE, _downsampling_convolution)
                (input_row, input_info.shape[axis],
                 wavelet->CAT(dec_lo_, TYPE), wavelet->dec_len,
                 output_row, 2, mode);
            break;
        case COEF_DETAIL:
            CAT(TYPE, _downsampling_convolution)
                (input_row, input_info.shape[axis],
                 wavelet->CAT(dec_hi_, TYPE), wavelet->dec_len,
                 output_row, 2, mode);
            break;
        }

        // Copy from temporary output if necessary
        if (make_temp_output)
            for (j = 0; j < output_info.shape[axis]; ++j)
                // Offsets are byte offsets, to need to cast to char and back
                *(TYPE *)((char *) output + output_offset
                          + j * output_info.strides[axis]) = output_row[j];
    }

    free(temp_input);
    free(temp_output);
    return 0;

 cleanup:
    free(temp_input);
    free(temp_output);
    return 2;
}


int CAT(TYPE, _downcoef)(const TYPE * const restrict input, const size_t input_len,
                         const Wavelet * const restrict wavelet,
                         TYPE * const restrict output, const size_t output_len,
                         const Coefficient coef, const MODE mode){
    const TYPE * filter;

    if (output_len != dwt_buffer_length_o(input_len, wavelet->dec_len,
                                          coef, mode))
        return -1;

    switch (coef){
    case COEF_APPROX:
        filter = wavelet->CAT(dec_lo_, TYPE);
        break;
    case COEF_DETAIL:
        filter = wavelet->CAT(dec_hi_, TYPE);
    }

    return CAT(TYPE, _downsampling_convolution)(input, input_len,
                                                filter, wavelet->dec_len,
                                                output, 2, mode);
}


int CAT(TYPE, _idwt_axis)(const TYPE * const restrict coefs_a, const ArrayInfo * const a_info,
                          const TYPE * const restrict coefs_d, const ArrayInfo * const d_info,
                          TYPE * const restrict output, const ArrayInfo output_info,
                          const Wavelet * const restrict wavelet,
                          const size_t axis, const MODE mode){
    size_t i;
    size_t num_loops = 1;
    TYPE * temp_coefs_a = NULL, * temp_coefs_d = NULL, * temp_output = NULL;

    // These are boolean values, but MSVC does not have <stdbool.h>
    int make_temp_coefs_a, make_temp_coefs_d, make_temp_output;
    int have_a = ((coefs_a != NULL) && (a_info != NULL));
    int have_d = ((coefs_d != NULL) && (d_info != NULL));


    if (!have_a && !have_d)
        return 3;

    if ((have_a && (a_info->ndim != output_info.ndim)) ||
        (have_d && (d_info->ndim != output_info.ndim)))
        return 1;
    if (axis >= output_info.ndim)
        return 1;

    for (i = 0; i < output_info.ndim; ++i){
        if (i == axis){
            size_t a_len = (have_a) ? a_info->shape[i] : 0;
            size_t d_len = (have_d) ? d_info->shape[i] : 0;
            size_t output_shape = idwt_buffer_length_o(a_len, d_len,
                                                       wavelet->rec_len, mode);
            if ((output_shape == 0) || (output_shape != output_info.shape[i]))
                return 1;
        } else {
            if ((have_a && (a_info->shape[i] != output_info.shape[i])) ||
                (have_d && (d_info->shape[i] != output_info.shape[i])))
                return 1;
        }
    }

    make_temp_coefs_a = have_a && a_info->strides[axis] != sizeof(TYPE);
    make_temp_coefs_d = have_d && d_info->strides[axis] != sizeof(TYPE);
    make_temp_output = output_info.strides[axis] != sizeof(TYPE);
    if (make_temp_coefs_a)
        if ((temp_coefs_a = malloc(a_info->shape[axis] * sizeof(TYPE))) == NULL)
            goto cleanup;
    if (make_temp_coefs_d)
        if ((temp_coefs_d = malloc(d_info->shape[axis] * sizeof(TYPE))) == NULL)
            goto cleanup;
    if (make_temp_output)
        if ((temp_output = malloc(output_info.shape[axis] * sizeof(TYPE))) == NULL)
            goto cleanup;

    for (i = 0; i < output_info.ndim; ++i){
        if (i != axis)
            num_loops *= output_info.shape[i];
    }

    for (i = 0; i < num_loops; ++i){
        size_t j;
        size_t a_offset = 0, d_offset = 0, output_offset = 0;
        TYPE * output_row;

        // Calculate offset into linear buffer
        {
            size_t reduced_idx = i;
            for (j = 0; j < output_info.ndim; ++j){
                size_t j_rev = output_info.ndim - 1 - j;
                if (j_rev != axis){
                    size_t axis_idx = reduced_idx % output_info.shape[j_rev];
                    reduced_idx /= output_info.shape[j_rev];

                    if (have_a)
                        a_offset += (axis_idx * a_info->strides[j_rev]);
                    if (have_d)
                        d_offset += (axis_idx * d_info->strides[j_rev]);
                    output_offset += (axis_idx * output_info.strides[j_rev]);
                }
            }
        }

        // Copy to temporary input if necessary
        if (make_temp_coefs_a)
            for (j = 0; j < a_info->shape[axis]; ++j)
                // Offsets are byte offsets, to need to cast to char and back
                temp_coefs_a[j] = *(TYPE *)((char *) coefs_a + a_offset
                                            + j * a_info->strides[axis]);
        if (make_temp_coefs_d)
            for (j = 0; j < d_info->shape[axis]; ++j)
                // Offsets are byte offsets, to need to cast to char and back
                temp_coefs_d[j] = *(TYPE *)((char *) coefs_d + d_offset
                                            + j * d_info->strides[axis]);

        // Select temporary or direct output
        output_row = make_temp_output ? temp_output
            : (TYPE *)((char *) output + output_offset);

        // upsampling_convolution adds to input, so zero
        memset(output_row, 0, output_info.shape[axis] * sizeof(TYPE));

        if (have_a){
            // Pointer arithmetic on NULL is undefined
            const TYPE * a_row = make_temp_coefs_a ? temp_coefs_a
                : (const TYPE *)((const char *) coefs_a + a_offset);
            CAT(TYPE, _upsampling_convolution_valid_sf)
                (a_row, a_info->shape[axis],
                 wavelet->CAT(rec_lo_, TYPE), wavelet->rec_len,
                 output_row, output_info.shape[axis],
                 mode);
        }
        if (have_d){
            // Pointer arithmetic on NULL is undefined
            const TYPE * d_row = make_temp_coefs_d ? temp_coefs_d
                : (const TYPE *)((const char *) coefs_d + d_offset);
            CAT(TYPE, _upsampling_convolution_valid_sf)
                (d_row, d_info->shape[axis],
                 wavelet->CAT(rec_hi_, TYPE), wavelet->rec_len,
                 output_row, output_info.shape[axis],
                 mode);
        }

        // Copy from temporary output if necessary
        if (make_temp_output)
            for (j = 0; j < output_info.shape[axis]; ++j)
                // Offsets are byte offsets, to need to cast to char and back
                *(TYPE *)((char *) output + output_offset
                          + j * output_info.strides[axis]) = output_row[j];
    }

    free(temp_coefs_a);
    free(temp_coefs_d);
    free(temp_output);
    return 0;

 cleanup:
    free(temp_coefs_a);
    free(temp_coefs_d);
    free(temp_output);
    return 2;
}


/*
 * IDWT reconstruction from approximation and detail coeffs
 *
 * If fix_size_diff is 1 then coeffs arrays can differ by one in length (this
 * is useful in multilevel decompositions and reconstructions of odd-length
 * signals).  Requires zero-filled output buffer.
 */
int CAT(TYPE, _idwt)(const TYPE * const restrict coefs_a, const size_t coefs_a_len,
                     const TYPE * const restrict coefs_d, const size_t coefs_d_len,
                     TYPE * const restrict output, const size_t output_len,
                     const Wavelet * const restrict wavelet, const MODE mode){
    size_t rec_len = idwt_buffer_length_o(coefs_a_len, coefs_d_len,
                                          wavelet->rec_len, mode);
    /* check output size */
    if((rec_len == 0) || (output_len != rec_len))
        return 0;

    memset(output, 0, output_len * sizeof(TYPE));

    /* reconstruct approximation coeffs with lowpass reconstruction filter */
    if(coefs_a){
        if(CAT(TYPE, _upsampling_convolution_valid_sf)(coefs_a, coefs_a_len,
                                                       wavelet->CAT(rec_lo_, TYPE),
                                                       wavelet->rec_len, output,
                                                       output_len, mode) < 0){
            return -1;
        }
    }
    /*
     * Add reconstruction of details coefs performed with highpass
     * reconstruction filter.
     */
    if(coefs_d){
        if(CAT(TYPE, _upsampling_convolution_valid_sf)(coefs_d, coefs_d_len,
                                                       wavelet->CAT(rec_hi_, TYPE),
                                                       wavelet->rec_len, output,
                                                       output_len, mode) < 0){
            return -1;
        }
    }

    return 0;
}


// TODO: Refactor to make more similar to `downcoef_axis`?
int CAT(TYPE, _dec_a)(const TYPE * const restrict input, const size_t input_len,
                      const Wavelet * const restrict wavelet,
                      TYPE * const restrict output, const size_t output_len,
                      const MODE mode){

    /* check output length */
    if(output_len != dwt_buffer_length(input_len, wavelet->dec_len, mode)){
        return -1;
    }

    return CAT(TYPE, _downsampling_convolution)(input, input_len,
                                                wavelet->CAT(dec_lo_, TYPE),
                                                wavelet->dec_len, output,
                                                2, mode);
}


/* Decomposition of input with highpass filter */

int CAT(TYPE, _dec_d)(const TYPE * const restrict input, const size_t input_len,
                      const Wavelet * const restrict wavelet,
                      TYPE * const restrict output, const size_t output_len,
                      const MODE mode){

    /* check output length */
    if(output_len != dwt_buffer_length(input_len, wavelet->dec_len, mode))
        return -1;

    return CAT(TYPE, _downsampling_convolution)(input, input_len,
                                                wavelet->CAT(dec_hi_, TYPE),
                                                wavelet->dec_len, output,
                                                2, mode);
}


/* Direct reconstruction with lowpass reconstruction filter */

int CAT(TYPE, _rec_a)(const TYPE * const restrict coeffs_a, const size_t coeffs_len,
                      const Wavelet * const restrict wavelet,
                      TYPE * const restrict output, const size_t output_len){

    /* check output length */
    if(output_len != reconstruction_buffer_length(coeffs_len, wavelet->rec_len))
        return -1;

    return CAT(TYPE, _upsampling_convolution_full)(coeffs_a, coeffs_len,
                                                   wavelet->CAT(rec_lo_, TYPE),
                                                   wavelet->rec_len, output,
                                                   output_len);
}


/* Direct reconstruction with highpass reconstruction filter */

int CAT(TYPE, _rec_d)(const TYPE * const restrict coeffs_d, const size_t coeffs_len,
                      const Wavelet * const restrict wavelet,
                      TYPE * const restrict output, const size_t output_len){

    /* check for output length */
    if(output_len != reconstruction_buffer_length(coeffs_len, wavelet->rec_len))
        return -1;

    return CAT(TYPE, _upsampling_convolution_full)(coeffs_d, coeffs_len,
                                                   wavelet->CAT(rec_hi_, TYPE),
                                                   wavelet->rec_len, output,
                                                   output_len);
}


/* basic SWT step (TODO: optimize) */
int CAT(TYPE, _swt_)(TYPE input[], index_t input_len,
                     const TYPE filter[], index_t filter_len,
                     TYPE output[], index_t output_len, int level){

    TYPE * e_filter;
    index_t i, e_filter_len;
    int ret;

    if(level < 1)
        return -1;

    if(level > swt_max_level(input_len))
        return -2;

    if(output_len != swt_buffer_length(input_len))
        return -1;

    /* TODO: quick hack, optimize */
    if(level > 1){
        /* allocate filter first */
        e_filter_len = filter_len << (level-1);
        e_filter = wtcalloc(e_filter_len, sizeof(TYPE));
        if(e_filter == NULL)
            return -1;

        /* compute upsampled filter values */
        for(i = 0; i < filter_len; ++i){
            e_filter[i << (level-1)] = filter[i];
        }
        ret = CAT(TYPE, _downsampling_convolution)(input, input_len, e_filter,
                                                   e_filter_len, output, 1,
                                                   MODE_PERIODIZATION);
        wtfree(e_filter);
        return ret;

    } else {
        return CAT(TYPE, _downsampling_convolution)(input, input_len, filter,
                                                    filter_len, output, 1,
                                                    MODE_PERIODIZATION);
    }
}

/*
 * Approximation at specified level
 * input - approximation coeffs from upper level or signal if level == 1
 */
int CAT(TYPE, _swt_a)(TYPE input[], index_t input_len, Wavelet* wavelet,
                      TYPE output[], index_t output_len, int level){
    return CAT(TYPE, _swt_)(input, input_len, wavelet->CAT(dec_lo_, TYPE),
                            wavelet->dec_len, output, output_len, level);
}

/* Details at specified level
 * input - approximation coeffs from upper level or signal if level == 1
 */
int CAT(TYPE, _swt_d)(TYPE input[], index_t input_len, Wavelet* wavelet,
                      TYPE output[], index_t output_len, int level){
    return CAT(TYPE, _swt_)(input, input_len, wavelet->CAT(dec_hi_, TYPE),
                            wavelet->dec_len, output, output_len, level);
}

#endif /* TYPE */
#undef restrict
