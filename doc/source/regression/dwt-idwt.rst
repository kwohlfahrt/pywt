.. _reg-dwt-idwt:

.. currentmodule:: pywt

The Discrete Wavelet Transform
==============================

Discrete Wavelet Transform
--------------------------

The discrete wavelet transform (DWT) applies a high- and low- pass filter to the
signal, and downsamples the resulting coefficients by two. This produces detail
and approximation coefficients with at total length approximately the same as
the input signal length.

Let's do a :func:`Discrete Wavelet Transform <dwt>` of a sample data ``x`` using
the ``db2`` wavelet.

    >>> import pywt
    >>> x = [3, 7, 1, 1, -2, 5, 4, 6]
    >>> w = pywt.Wavelet('db2')
    >>> cA, cD = pywt.dwt(x, w)

And the approximation and details coefficients are in ``cA`` and ``cD``
respectively:

    >>> print(cA)
    [ 5.65685425  7.39923721  0.22414387  3.33677403  7.77817459]
    >>> print(cD)
    [-2.44948974 -1.60368225 -4.44140056 -0.41361256  1.22474487]

A number of border extension :data:`modes <Modes>` are also supported, to
minimize edge artifacts.

    >>> cA, cD = pywt.dwt(x, w, mode='constant')
    >>> print(cA)
    [ 3.7250026   7.39923721  0.22414387  3.33677403  7.51935555]
    >>> print(cD)
    [-1.93185165 -1.60368225 -4.44140056 -0.41361256  0.25881905]

Note that the center coefficients are the same, but those at the edge are
different. The appropriate edge extension mode will depend on the data being
transformed.

Coefficient Length
~~~~~~~~~~~~~~~~~~

Note that the output coefficients arrays length depends not only on the input
data length but also on the :class:Wavelet type (particularly on its
:attr:`filters lenght <~Wavelet.dec_len>` that are used in the transformation).

To find out what will be the output data size use the :func:`dwt_coeff_len`
function:

    >>> pywt.dwt_coeff_len(len(x), w.dec_len, 'symmetric')
    5
    >>> len(cA)
    5

The third argument of the :func:`dwt_coeff_len` is the already mentioned signal
extension mode.

    >>> for mode in pywt.Modes.modes:
    ...     print(mode, pywt.dwt_coeff_len(len(x), w.dec_len, mode))
    zero 5
    constant 5
    symmetric 5
    periodic 5
    smooth 5
    periodization 4

As you see in the above example, the periodizaion :ref:`Mode <Modes>` mode is
slightly different from the others. It's aim when doing the :func:`DWT <dwt>`
transform is to output coefficients arrays that are half of the length of the
input data.

Knowing that, you should never mix the periodization mode with other modes when
doing :func:`DWT <dwt>` and :func:`IDWT <idwt>`. Otherwise, it will produce
**invalid results**:

    >>> x
    [3, 7, 1, 1, -2, 5, 4, 6]
    >>> cA, cD = pywt.dwt(x, wavelet=w, mode='periodization')
    >>> print(pywt.idwt(cA, cD, 'sym3', 'symmetric')) # invalid mode
    [ 1.  1. -2.  5.]
    >>> print(pywt.idwt(cA, cD, 'sym3', 'periodization'))
    [ 3.  7.  1.  1. -2.  5.  4.  6.]

Inverse Discrete Wavelet Transform
----------------------------------

Now let's do an opposite operation
- :func:`Inverse Discrete Wavelet Transform <idwt>`:

    >>> print(pywt.idwt(cA, cD, w))
    [ 3.  7.  1.  1. -2.  5.  4.  6.]

The input data is perfectly reconstructed from the coefficients.

Alternative Input
-----------------

Wavelets by Name
~~~~~~~~~~~~~~~~

Instead of a wavelet object, the name of any built-in wavelet can be passed to
the wavelet transform.

    >>> pywt.dwt(x, 'db2') == (cA, cD)

Passing ``None`` instead of coefficients data to :func:`idwt`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now some tips & tricks. Passing ``None`` as one of the coefficient arrays
parameters is the same as passing a *zero-filled* array.

    >>> print(pywt.idwt([1,2,0,1], None, 'db2', 'symmetric'))
    [ 1.19006969  1.54362308  0.44828774 -0.25881905  0.48296291  0.8365163 ]

    >>> print(pywt.idwt([1, 2, 0, 1], [0, 0, 0, 0], 'db2', 'symmetric'))
    [ 1.19006969  1.54362308  0.44828774 -0.25881905  0.48296291  0.8365163 ]

Remember that only one argument at a time can be ``None``:

    >>> print(pywt.idwt(None, None, 'db2', 'symmetric'))
    Traceback (most recent call last):
    ...
    ValueError: At least one coefficient parameter must be specified.
