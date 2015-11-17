#!/usr/bin/env python3

import numpy
from itertools import count, repeat, chain
from math import floor, ceil

# Has to take upsampling as a parameter because desired length may not be exact multiple
def edge_extend(a, n, upsampling, mode='zero-pad'):
    r = numpy.zeros(len(a) * upsampling + sum(n), dtype=a.dtype)
    r[n[0]:-n[1]] = upsample(a, upsampling)

    start_extend = range(upsampling - 1, n[0], upsampling)
    end_extend = range(-n[1], 0, upsampling)
    if mode == 'zero-pad':
        pass
    elif mode == 'periodic':
        reverse_enumerate = reversed(range(len(a)))
        reverse_enumerate = chain.from_iterable(repeat(list(reverse_enumerate)))
        for i, j in zip(reverse_enumerate, reversed(start_extend)):
            r[j] = a[i]
        forward_enumerate = chain.from_iterable(repeat(range(len(a))))
        for i, j in zip(forward_enumerate, end_extend):
            r[j] = a[i]
    elif mode == 'constant':
        r[upsampling-1:n[0]:upsampling] = a[0]
        r[-n[1]::upsampling] = a[-1]
    elif mode == 'smooth':
        for i, j in zip(count(1), reversed(start_extend)):
            r[j] = a[0] + i * (a[0] - a[1])
        for i, j in zip(count(1), end_extend):
            r[j] = a[-1] + i * (a[-1] - a[-2])
    elif mode == 'symmetric':
        reverse_enumerate = chain(range(len(a)), reversed(range(len(a))))
        reverse_enumerate = chain.from_iterable(repeat(list(reverse_enumerate)))
        for i, j in zip(reverse_enumerate, reversed(start_extend)):
            r[j] = a[i]
        forward_enumerate = chain(reversed(range(len(a))), range(len(a)))
        forward_enumerate = chain.from_iterable(repeat(list(forward_enumerate)))
        for i, j in zip(forward_enumerate, end_extend):
            r[j] = a[i]
    else:
        raise ValueError("Invalid edge extension mode.")

    return r

def downsample(a, step):
    return a[step-1::step]

def upsample(a, step):
    r = numpy.zeros(len(a) * step)
    r[::step] = a
    return r

def unified_convolution(input, filter,
                        input_upsampling=1, output_downsampling=1, filter_upsampling=1,
                        mode='zero-pad'):
    filter = upsample(filter, filter_upsampling)
    padding = (len(filter) - 1, len(filter) - input_upsampling)

    input = edge_extend(input, padding, input_upsampling, mode)
    convolved = numpy.convolve(input, filter, mode='valid')

    return downsample(convolved, output_downsampling)

def printArray(a):
    print("".join(map("{: 6.2f} ".format, a)))

i = numpy.arange(12) + 1
f = numpy.arange(6) + 1
printArray(unified_convolution(i, f, output_downsampling=2, mode='zero-pad'))
printArray(unified_convolution(i, f, input_upsampling=2, mode='zero-pad'))
printArray(unified_convolution(i, f, filter_upsampling=2, mode='periodic'))
