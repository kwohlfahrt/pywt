.. _reg-swt-iswt:

.. currentmodule:: pywt

The Stationary Wavelet Transform
================================

Stationary Wavelet Transform
----------------------------

The discrete wavelet transform is shift variant due to it's downsampling step
(i.e. the position of the first sample can have a large effect on the resulting
coefficients). One way to overcome this is to skip the downsampling, and instead
upsample the wavelet filter for each level of the transform (the *algorithme à
trous*). This results in a redunant set of coefficients (the coefficients are
twice as long as the input signal).

    >>> x = [3, 7, 1, 1, -2, 5, 4, 6]
    >>> w = pywt.Wavelet('db2')
    >>> [(cA3, cD3), (cA2, cD2), (cA1, cD1)] = pywt.swt(x, w)

    >>> print(cA3)
    [ 8.83883476  8.83883476  8.83883476  8.83883476  8.83883476  8.83883476
      8.83883476  8.83883476]
    >>> print(cD2)
    [ 1.81129763 -3.95184209 -5.6749183  -1.46081668  1.83581668  1.26882939
      2.02780398  4.14382939]

Differences to DWT
~~~~~~~~~~~~~~~~~~

Unlike the DWT, this method produces several levels of the transform (the
starting and ending levels can be specified). See :ref:`multilevel <reg-multilevel>`
for details.

Inverse Stationary Wavelet Transform
------------------------------------

The inverse transform works similarly to the DWT:

    >>> pywt.iswt(pwyt.swt(coeffs, w), w)
    [ 3.  7.  1.  1. -2.  5.  4.  6.]

Example
-------

In the following example, we will plot the multiple approximation and detail
SWT coefficients for the ECG sample datasets.

    >>> data = pywt.data.ecg()
    >>> coeffs = pywt.swt(data, 'sym5', 5)
    >>> fig = plt.figure()
    >>> ax_main = fig.add_subplot(len(coeffs) + 1, 1, 1)
    >>> ax_main.plot(data)
    >>> for i, (cA, cD) in enumerate(reversed(coeffs), start=1):
    ...     ax_a = fig.add_subplot(len(coeffs) + 1, 2, i * 2 + 1)
    ...     ax_a.plot(cA, 'r')
    ...     ax_a.set_ylabel("A%d" % (i + 1))
    ...     ax_d = fig.add_subplot(len(coeffs) + 1, 2, i * 2 + 2)
    ...     ax_d.plot(cD, 'g')
    ...     ax_a.set_ylabel("D%d" % (i + 1))
    >>> plt.show()
