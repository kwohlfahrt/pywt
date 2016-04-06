.. _reg-multilevel:

.. currentmodule:: pywt

Multilevel DWT and SWT
======================

Multilevel functions for both the SWT and DWT are provided.

Multilevel DWT
--------------

The multilevel DWT functions are provided as :func:`waverec` and :func:`wavedec`
(with appropriate suffixes for multidimensional versions).

>>> x = [3, 7, 1, 1, -2, 5, 4, 6]
>>> cA, cD3, cD2, cD1 = pywt.wavedec(x, 'db1')
>>> cAn, cD3n, cD2n, cD1n = pywt.wavedecn(x, 'db1')

Decomposition
~~~~~~~~~~~~~

The output of the decomposition function is a list of coefficients. The first
member is always an array, containing the approximation coefficients at the
highest level.

>>> type(cA) is type(cAn) is np.ndarray
True
>>> print(cA)
[ 8.83883476]

The following coefficients are the detail coefficients. For the 1D transform,
these will be more 1D arrays, while for the 2D variant they will be 3-tuples and
for the n-D variant they will be dictionaries. This matches the output format of
the single level transform (minus the approximation coefficients).

>>> all(type(c) is np.ndarray for c in cD3, cD2, cD1)
True
>>> all(type(c) is dict for c in cD3n, cD2n, cD1n)
True
>>> list(cD3n.keys())
['d']

The number of levels can be specified, by default it is the maximum possible
(remember the highest-level approximation coefficients are included in the
output).

>>> cA, cD2, cD1 = pywt.wavedec(x, 'db1', mode='constant', level=2)
>>> pywt.dwt_max_level(len(x), 'db1')
3
>>> pywt.dwt_max_level(len(x), 'db1') == len(pywt.wavdec(x, 'db1')) - 1
True

Reconstruction
~~~~~~~~~~~~~~

Reconstruction is straightforward, using just the output coefficients of
:func:`wavedec`.

>>> coeffs = cA, cD3, cD2, cD1
>>> print(pywt.waverec(coeffs, 'db1'))
[ 3.  7.  1.  1. -2.  5.  4.  6.]

As with the single level transforms, any parameter can be replaced with `None`
to simulate all zero coefficients.

>>> print(pywt.waverec([cA, None, cD2, None], 'db1'))

Multilevel SWT
--------------

The SWT functions are natively multilevel (since the signal size does not
change).

Decomposition
~~~~~~~~~~~~~

The SWT works similarly to the multilevel DWT, except the approximation
coefficients are preserved at each level.

>>> x = [3, 7, 1, 3, -2, 6, 4, 6]
>>> (cA2, cD2), (cA1, cD1) = pywt.swt(x, 'db1', level=2)

Similarly, functions to calculate the maximum possible level of the
decomposition exist.

>>> len(pywt.swt(x, 'db1'))
3
>>> pywt.swt_max_level(len(x))
3

A start level can be specified as well, and the passed data will be treated as
approximation coefficients at that level.

>>> (cA2_p, cD2_p), = pywt.swt(cA1, 'db1', level=1, start_level=1)
>>> (cA2 == cA2_p).all()
True

Reconstruction
~~~~~~~~~~~~~~

There are no particular notes about reconstruction - just plug in the
coefficients output by the decomposition (inserting ``None`` if desired).

>>> pywt.iswt([(cA2, cD2), (cA1, cD1)], 'db1')
[3, 7, 1, 3, -2, 6, 4, 6]
