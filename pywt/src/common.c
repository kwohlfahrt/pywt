/* Copyright (c) 2006-2012 Filip Wasilewski <http://en.ig.ma/> */
/* See COPYING for license details. */

#include "common.h"

#ifdef PY_EXTENSION
void *wtcalloc(size_t len, size_t size){
        void *p = wtmalloc(len*size);
        if(p)
            memset(p, 0, len*size);
        return p;
}
#endif

/* Returns the floor of the base-2 log of it's input
 *
 * x = 0
 *    MSVC: returns 0
 *    Undefined on GCC/clang
 */
unsigned char uint_log2(unsigned long x){
#if defined(_MSC_VER)
    unsigned long i;
    _BitScanReverse(&i, x);
    return i;
#else
    // GCC and clang
    // Safe cast: 0 <= clzl < arch_bits (64) where result is defined
    unsigned char leading_zeros = (unsigned char) __builtin_clzl(x);
    return sizeof(unsigned long) * 8 - leading_zeros - 1;
#endif
}

/* buffers and max levels params */

size_t dwt_buffer_length(size_t input_len, size_t filter_len, MODE mode){
    if(input_len < 1 || filter_len < 1)
        return 0;

    switch(mode){
            case MODE_PERIODIZATION:
                return input_len / 2 + ((input_len%2) ? 1 : 0);
            default:
                return (input_len + filter_len - 1) / 2;
    }
}

size_t dwt_buffer_length_o(size_t input_len, size_t filter_len,
                           Coefficient coef, MODE mode){
    size_t len, adj;
    if(input_len < 1 || filter_len < 1)
        return 0;

    adj = ((input_len % 2) && coef == COEF_APPROX) ? 1 : 0;

    switch(mode){
    case MODE_PERIODIZATION:
        return input_len / 2 + adj;
    default:
        return (input_len + filter_len - 1) / 2 - 1 + adj;
    }
}

size_t reconstruction_buffer_length(size_t coeffs_len, size_t filter_len){
    if(coeffs_len < 1 || filter_len < 1)
        return 0;

    return 2*coeffs_len+filter_len-2;
}

size_t idwt_buffer_length(size_t coeffs_len, size_t filter_len, MODE mode){
    switch(mode){
            case MODE_PERIODIZATION:
                return 2*coeffs_len;
            default:
                return 2*coeffs_len-filter_len+2;
    }
}

size_t idwt_buffer_length_o(size_t coefs_a_len, size_t coefs_d_len,
                            size_t filter_len, MODE mode){
    size_t coefs_len, adj;

    if (coefs_a_len == 0 && coefs_d_len == 0){
        return 0;
    } else if (coefs_a_len == 0) {
        adj = 0;
        coefs_len = coefs_d_len;
    } else if (coefs_d_len == 0) {
        adj = 0;
        coefs_len = coefs_a_len;
    } else {
        adj = coefs_a_len - coefs_d_len;
        coefs_len = coefs_d_len;
        if (adj > 1)
            return 0;
    }

    switch (mode){
    case MODE_PERIODIZATION:
        return 2 * coefs_len + adj;
    default:
        return (coefs_len + adj) * 2 - filter_len + 1;
        //return 2 * coefs_len - filter_len + adj;
    }
}

size_t swt_buffer_length(size_t input_len){
    return input_len;
}

unsigned char dwt_max_level(size_t input_len, size_t filter_len){
    if(input_len < 1 || filter_len < 2)
        return 0;

    return uint_log2(input_len/(filter_len-1));
}

unsigned char swt_max_level(size_t input_len){
    /* check how many times input_len is divisible by 2 */
    unsigned char j = 0;
    while (input_len > 0){
        if (input_len % 2)
            return j;
        input_len /= 2;
        j++;
    }
    return j;
}
