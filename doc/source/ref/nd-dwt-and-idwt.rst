.. _ref-dwtn:
.. include:: ../substitutions.rst

=================================================
nD Forward and Inverse Discrete Wavelet Transform
=================================================

.. currentmodule:: pywt

N-dimensional Transform
=======================

The N-dimensional functions work very similarly to the 1D functions, with the
coefficients in a different format. Instead of a tuple, they return a dictionary
with the keys being a sequence of N 'a' and 'd' characters, indicating that the
transform has been '**a**\ pproximate' or '**d**\ etail' in the respective dimension.

For example, the key ``'aad'`` is a coefficient where the approximate transform
was used for the first two dimensions, and the detail transform for the third
dimension.

>>> x = [1, 2, 3, 4]
>>> cA, cD = pywt.dwt(x, 'db2')
>>> c = pywt.dwtn(x, 'db2')
>>> (cA == c['a']).all() and (cD == c['d']).all()
True

Single level
------------
.. autofunction:: dwtn

.. autofunction:: idwtn

Multilevel
----------
.. autofunction:: wavedecn

.. autofunction:: waverecn

2D Transform
============

The 2D transforms are again similar, with the exception of the format of the
coefficient.

>>> x = pywt.aero()
>>> cN = pywt.dwtn(x, 'db2')
>>> c2 = pywt.dwt2(x, 'db2')
>>> c2 == (c['aa'], (c['ad'], c['da'], c['dd']))

.. note:: Internally, these transforms use the ND transform where avilable, and
          then re-arrange the input and output as required.

The reader may be more familliar with the common layout where all coefficients
are stored in a large array. The output of the 2D decomposition function
corresponds do these as follows::
  
           pywt                        Array
                                -------------------
                                |        |        |
                                | cA(AA) | cH(AD) |
                                |        |        |
    (cA, (cH, cV, cD))  <--->   -------------------
                                |        |        |
                                | cV(DA) | cD(DD) |
                                |        |        |
                                -------------------

.. note:: A function to convert between the two formats is a work in progress.

Single level
------------

.. autofunction:: dwt2

.. autofunction:: idwt2

Multilevel
----------

.. autofunction:: wavedec2

.. autofunction:: waverec2
