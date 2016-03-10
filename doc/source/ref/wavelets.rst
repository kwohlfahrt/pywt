.. _ref-wavelets:

.. currentmodule:: pywt

========
Wavelets
========

Wavelet ``families()``
----------------------

.. autofunction:: families


Built-in wavelets - ``wavelist()``
----------------------------------

.. autofunction:: wavelist


``Wavelet`` object
------------------

.. autoclass:: Wavelet
   :members:

.. _using-custom-wavelets:

Using custom wavelets
---------------------

PyWavelets comes with a :func:`long list <pywt.wavelist>` of the most popular
wavelets built-in and ready to use. If you need to use a specific wavelet which
is not included in the list it is very easy to do so. Just pass a list of four
filters or an object with a :attr:`~Wavelet.filter_bank` attribute as a
*filter_bank* argument to the :class:`Wavelet` constructor.

.. compound::

    The filters list, either in a form of a simple Python list or returned via
    the :attr:`~Wavelet.filter_bank` attribute, must be in the following order:

      * lowpass decomposition filter
      * highpass decomposition filter
      * lowpass reconstruction filter
      * highpass reconstruction filter

    just as for the :attr:`~Wavelet.filter_bank` attribute of the
    :class:`Wavelet` class.

The Wavelet object created in this way is a standard :class:`Wavelet` instance.

The following example illustrates the way of creating custom Wavelet objects
from plain Python lists of filter coefficients and a *filter bank-like* objects.

  **Example:**

  .. sourcecode:: python

    >>> import pywt, math
    >>> c = math.sqrt(2)/2
    >>> dec_lo, dec_hi, rec_lo, rec_hi = [c, c], [-c, c], [c, c], [c, -c]
    >>> filter_bank = [dec_lo, dec_hi, rec_lo, rec_hi]
    >>> myWavelet = pywt.Wavelet(name="myHaarWavelet", filter_bank=filter_bank)
    >>>
    >>> class HaarFilterBank(object):
    ...     @property
    ...     def filter_bank(self):
    ...         c = math.sqrt(2)/2
    ...         dec_lo, dec_hi, rec_lo, rec_hi = [c, c], [-c, c], [c, c], [c, -c]
    ...         return [dec_lo, dec_hi, rec_lo, rec_hi]
    >>> filter_bank = HaarFilterBank()
    >>> myOtherWavelet = pywt.Wavelet(name="myHaarWavelet", filter_bank=filter_bank)
