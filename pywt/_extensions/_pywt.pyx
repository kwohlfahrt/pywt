# Copyright (c) 2006-2012 Filip Wasilewski <http://en.ig.ma/>
# See COPYING for license details.

__doc__ = """Pyrex wrapper for low-level C wavelet transform implementation."""
__all__ = ['MODES', 'Modes', 'Wavelet', 'wavelist', 'families']

###############################################################################
# imports
import warnings

cimport c_wt
cimport common
from ._dwt cimport upcoef

from libc.math cimport pow, sqrt

import numpy as np


###############################################################################
# Modes
_old_modes = ['zpd',
              'cpd',
              'sym',
              'ppd',
              'sp1',
              'per',
              ]

_attr_deprecation_msg = ('{old} has been renamed to {new} and will '
                         'be unavailable in a future version '
                         'of pywt.')

class _Modes(object):
    """
    Because the most common and practical way of representing digital signals
    in computer science is with finite arrays of values, some extrapolation of
    the input data has to be performed in order to extend the signal before
    computing the :ref:`Discrete Wavelet Transform <ref-dwt>` using the
    cascading filter banks algorithm.

    Depending on the extrapolation method, significant artifacts at the
    signal's borders can be introduced during that process, which in turn may
    lead to inaccurate computations of the DWT at the signal's ends.

    PyWavelets provides several methods of signal extrapolation that can be
    used to minimize this negative effect:

    - Zero-padding (``Modes.zero``)::

        ... 0  0 | x1 x2 ... xn | 0  0 ...

    - Constant-padding (``Modes.constant``)::

        ... x1 x1 | x1 x2 ... xn | xn xn ...

    - Symmetric-padding (``Modes.symmetric``)::

        ... x2 x1 | x1 x2 ... xn | xn xn-1 ...

    - Reflect-padding (``Modes.reflect``)::

        ... x2 x1 | x1 x2 ... xn | xn-1 xn-2 ...

    - Periodic-padding (``Modes.periodic``)::

        ... xn-1 xn | x1 x2 ... xn | x1 x2 ...

    - 1st-derivative extrapolation (``Modes.smooth``)

    DWT performed for these extension modes is slightly redundant, but ensure a
    perfect reconstruction for IDWT. To receive the smallest possible number of
    coefficients, computations can be performed with the periodization mode:

    - Periodization (``Modes.periodization``):

      Like periodic-padding but gives the smallest possible number of
      decomposition coefficients. IDWT must be performed with the same mode.

    Examples
    --------
    >>> import pywt
    >>> pywt.Modes.modes
        ['zero', 'constant', 'symmetric', 'reflect', 'periodic', 'smooth', 'periodization']
    >>> # The different ways of passing wavelet and mode parameters
    >>> (a, d) = pywt.dwt([1,2,3,4,5,6], 'db2', 'smooth')
    >>> (a, d) = pywt.dwt([1,2,3,4,5,6], pywt.Wavelet('db2'), pywt.Modes.smooth)

    Notes
    -----
    Extending data in context of PyWavelets does not mean reallocation of the
    data in computer's physical memory and copying values, but rather computing
    the extra values only when they are needed.  This feature saves extra
    memory and CPU resources and helps to avoid page swapping when handling
    relatively big data arrays on computers with low physical memory.
    """

    # I can't make attribute docstrings work with Cython
    zero = common.MODE_ZEROPAD
    constant = common.MODE_CONSTANT_EDGE
    symmetric = common.MODE_SYMMETRIC
    reflect = common.MODE_REFLECT
    periodic = common.MODE_PERIODIC
    smooth = common.MODE_SMOOTH
    periodization = common.MODE_PERIODIZATION

    modes = ["zero", "constant", "symmetric", "reflect", "periodic",
             "smooth", "periodization"]

    def from_object(self, mode):
        if isinstance(mode, int):
            if mode <= common.MODE_INVALID or mode >= common.MODE_MAX:
                raise ValueError("Invalid mode.")
            m = mode
        else:
            try:
                m = getattr(Modes, mode)
            except AttributeError:
                raise ValueError("Unknown mode name '%s'." % mode)

        return m

    def __getattr__(self, mode):
        # catch deprecated mode names
        if mode in _old_modes:
            new_mode = Modes.modes[_old_modes.index(mode)]
            warnings.warn(_attr_deprecation_msg.format(old=mode, new=new_mode),
                          DeprecationWarning)
            mode = new_mode
        return Modes.__getattribute__(mode)


Modes = _Modes()

class _DeprecatedMODES(_Modes):
    msg = ("MODES has been renamed to Modes and will be "
           "removed in a future version of pywt.")

    def __getattribute__(self, attr):
        """Override so that deprecation warning is shown
        every time MODES is used.

        N.B. have to use __getattribute__ as well as __getattr__
        to ensure warning on e.g. `MODES.symmetric`.
        """
        if not attr.startswith('_'):
            warnings.warn(_DeprecatedMODES.msg, DeprecationWarning)
        return _Modes.__getattribute__(self, attr)

    def __getattr__(self, attr):
        """Override so that deprecation warning is shown
        every time MODES is used.
        """
        warnings.warn(_DeprecatedMODES.msg, DeprecationWarning)
        return _Modes.__getattr__(self, attr)


MODES = _DeprecatedMODES()

###############################################################################
# Wavelet

include "wavelets_list.pxi"  # __wname_to_code

cdef object wname_to_code(name):
    cdef object code_number
    try:
        code_number = __wname_to_code[name]
        assert len(code_number) == 2
        assert isinstance(code_number[0], int)
        assert isinstance(code_number[1], int)
        return code_number
    except KeyError:
        raise ValueError("Unknown wavelet name '%s', check wavelist() for the "
                         "list of available builtin wavelets." % name)


def wavelist(family=None):
    """
    wavelist(family=None)

    Returns list of built-in wavelet names for the given family name.

    Parameters
    ----------
    family : {'haar', 'db', 'sym', 'coif', 'bior', 'rbio', 'dmey'}
        Short family name.  If the family name is None (default) then names
        of all the built-in wavelets are returned. Otherwise the function
        returns names of wavelets that belong to the given family.

    Returns
    -------
    wavelist : list
        List of available wavelet names

    Examples
    --------
    >>> import pywt
    >>> pywt.wavelist('coif')
    ['coif1', 'coif2', 'coif3', 'coif4', 'coif5']
    """
    cdef object wavelets, sorting_list
    sorting_list = []  # for natural sorting order
    wavelets = []
    cdef object name
    if family is None:
        for name in __wname_to_code:
            sorting_list.append((name[:2], len(name), name))
    elif family in __wfamily_list_short:
        for name in __wname_to_code:
            if name.startswith(family):
                sorting_list.append((name[:2], len(name), name))
    else:
        raise ValueError("Invalid short family name '%s'." % family)

    sorting_list.sort()
    for x, x, name in sorting_list:
        wavelets.append(name)
    return wavelets


def families(int short=True):
    """
    families(short=True)

    Returns a list of available built-in wavelet families.

    Currently the built-in families are:

    * Haar (``haar``)
    * Daubechies (``db``)
    * Symlets (``sym``)
    * Coiflets (``coif``)
    * Biorthogonal (``bior``)
    * Reverse biorthogonal (``rbio``)
    * `"Discrete"` FIR approximation of Meyer wavelet (``dmey``)

    Parameters
    ----------
    short : bool, optional
        Use short names (default: True).

    Returns
    -------
    families : list
        List of available wavelet families.

    Examples
    --------
    >>> import pywt
    >>> pywt.families()
    ['haar', 'db', 'sym', 'coif', 'bior', 'rbio', 'dmey']
    >>> pywt.families(short=False)
    ['Haar', 'Daubechies', 'Symlets', 'Coiflets', 'Biorthogonal', 'Reverse biorthogonal', 'Discrete Meyer (FIR Approximation)']

    """
    if short:
        return __wfamily_list_short[:]
    return __wfamily_list_long[:]


cdef public class Wavelet [type WaveletType, object WaveletObject]:
    """
    Wavelet(name, filter_bank=None)

    Describes the properties of a wavelet identified by name.

    Parameters
    ----------
    name : str, optional
        The name for the wavelet. If it is one of the built-in names, the
        ``filter_bank`` parameter is optional

    filter_bank : 4-tuple of array_like or object with ``filter_bank`` attribute
        This parameter must be provided in the same format as the corresponding
        attribute. An object defining the ``filter_bank`` attribute (e.g. an
        instance of this class) may also be used.

    Examples
    --------

    >>> import pywt
    >>> w = pywt.Wavelet('db1')
    >>> print(w)
    Wavelet db1
      Family name:    Daubechies
      Short name:     db
      Filters length: 2
      Orthogonal:     True
      Biorthogonal:   True
      Symmetry:       asymmetric
    >>> print(wavelet.dec_lo, wavelet.dec_hi)
    [0.70710678118655, 0.70710678118655] [-0.70710678118655, 0.70710678118655]
    >>> print(wavelet.rec_lo, wavelet.rec_hi)
    [0.70710678118655, 0.70710678118655] [0.70710678118655, -0.70710678118655]
    """

    #cdef readonly properties
    def __cinit__(self, name=u"", object filter_bank=None):
        cdef object family_code, family_number
        cdef object filters
        cdef pywt_index_t filter_length
        cdef object dec_lo, dec_hi, rec_lo, rec_hi

        if not name and filter_bank is None:
            raise TypeError("Wavelet name or filter bank must be specified.")

        if filter_bank is None:
            # builtin wavelet
            self.name = name.lower()
            family_code, family_number = wname_to_code(self.name)
            self.w = <wavelet.Wavelet*> wavelet.wavelet(family_code, family_number)

            if self.w is NULL:
                raise ValueError("Invalid wavelet name.")
            self.number = family_number
        else:
            if hasattr(filter_bank, "filter_bank"):
                filters = filter_bank.filter_bank
                if len(filters) != 4:
                    raise ValueError("Expected filter bank with 4 filters, "
                    "got filter bank with %d filters." % len(filters))
            elif hasattr(filter_bank, "get_filters_coeffs"):
                msg = ("Creating custom Wavelets using objects that define "
                       "`get_filters_coeffs` method is deprecated. "
                       "The `filter_bank` parameter should define a "
                       "`filter_bank` attribute instead of "
                       "`get_filters_coeffs` method.")
                warnings.warn(msg, DeprecationWarning)
                filters = filter_bank.get_filters_coeffs()
                if len(filters) != 4:
                    msg = ("Expected filter bank with 4 filters, got filter "
                           "bank with %d filters." % len(filters))
                    raise ValueError(msg)
            else:
                filters = filter_bank
                if len(filters) != 4:
                    msg = ("Expected list of 4 filters coefficients, "
                           "got %d filters." % len(filters))
                    raise ValueError(msg)
            try:
                dec_lo = np.asarray(filters[0], dtype=np.float64)
                dec_hi = np.asarray(filters[1], dtype=np.float64)
                rec_lo = np.asarray(filters[2], dtype=np.float64)
                rec_hi = np.asarray(filters[3], dtype=np.float64)
            except TypeError:
                raise ValueError("Filter bank with numeric values required.")

            if not (1 == dec_lo.ndim == dec_hi.ndim ==
                         rec_lo.ndim == rec_hi.ndim):
                raise ValueError("All filters in filter bank must be 1D.")

            filter_length = len(dec_lo)
            if not (0 < filter_length == len(dec_hi) == len(rec_lo) ==
                                         len(rec_hi)) > 0:
                raise ValueError("All filters in filter bank must have "
                                 "length greater than 0.")

            self.w = <wavelet.Wavelet*> wavelet.blank_wavelet(filter_length)
            if self.w is NULL:
                raise MemoryError("Could not allocate memory for given "
                                  "filter bank.")

            # copy values to struct
            copy_object_to_float32_array(dec_lo, self.w.dec_lo_float)
            copy_object_to_float32_array(dec_hi, self.w.dec_hi_float)
            copy_object_to_float32_array(rec_lo, self.w.rec_lo_float)
            copy_object_to_float32_array(rec_hi, self.w.rec_hi_float)

            copy_object_to_float64_array(dec_lo, self.w.dec_lo_double)
            copy_object_to_float64_array(dec_hi, self.w.dec_hi_double)
            copy_object_to_float64_array(rec_lo, self.w.rec_lo_double)
            copy_object_to_float64_array(rec_hi, self.w.rec_hi_double)

            self.name = name

    def __dealloc__(self):
        if self.w is not NULL:
            wavelet.free_wavelet(self.w)
            self.w = NULL

    def __len__(self):
        return self.w.dec_len

    property dec_lo:
        "Lowpass decomposition filter"
        def __get__(self):
            return float64_array_to_list(self.w.dec_lo_double, self.w.dec_len)

    property dec_hi:
        "Highpass decomposition filter"
        def __get__(self):
            return float64_array_to_list(self.w.dec_hi_double, self.w.dec_len)

    property rec_lo:
        "Lowpass reconstruction filter"
        def __get__(self):
            return float64_array_to_list(self.w.rec_lo_double, self.w.rec_len)

    property rec_hi:
        "Highpass reconstruction filter"
        def __get__(self):
            return float64_array_to_list(self.w.rec_hi_double, self.w.rec_len)

    property rec_len:
        "Reconstruction filters length"
        def __get__(self):
            return self.w.rec_len

    property dec_len:
        "Decomposition filters length"
        def __get__(self):
            return self.w.dec_len

    property family_name:
        "Wavelet family name"
        def __get__(self):
            return self.w.family_name.decode('latin-1')

    property short_family_name:
        "Short wavelet family name"
        def __get__(self):
            return self.w.short_name.decode('latin-1')

    property orthogonal:
        "Is orthogonal"
        def __get__(self):
            return bool(self.w.orthogonal)
        def __set__(self, int value):
            self.w.orthogonal = (value != 0)

    property biorthogonal:
        "Is biorthogonal"
        def __get__(self):
            return bool(self.w.biorthogonal)
        def __set__(self, int value):
            self.w.biorthogonal = (value != 0)

    property symmetry:
        """Wavelet symmetry.

        May be ``asymmetric``, ``near symmetric`` or ``symmetric``
        """
        def __get__(self):
            if self.w.symmetry == wavelet.ASYMMETRIC:
                return "asymmetric"
            elif self.w.symmetry == wavelet.NEAR_SYMMETRIC:
                return "near symmetric"
            elif self.w.symmetry == wavelet.SYMMETRIC:
                return "symmetric"
            else:
                return "unknown"

    property vanishing_moments_psi:
        "Number of vanishing moments for wavelet function"
        def __get__(self):
            if self.w.vanishing_moments_psi >= 0:
                return self.w.vanishing_moments_psi

    property vanishing_moments_phi:
        "Number of vanishing moments for scaling function"
        def __get__(self):
            if self.w.vanishing_moments_phi >= 0:
                return self.w.vanishing_moments_phi

    property filter_bank:
        """Returns tuple of wavelet filters coefficients, in the following
        order::

        >>> w = Wavelet('bior1.3')
        >>> w.filter_bank == (w.dec_lo, w.dec_hi, w.rec_lo, w.rec_hi)
        True
        """
        def __get__(self):
            return (self.dec_lo, self.dec_hi, self.rec_lo, self.rec_hi)

    def get_filters_coeffs(self):
        warnings.warn("The `get_filters_coeffs` method is deprecated. "
                      "Use `filter_bank` attribute instead.", DeprecationWarning)
        return self.filter_bank

    property inverse_filter_bank:
        """Tuple of inverse wavelet filters coefficients::

        >>> w = Wavelet('bior1.3')
        >>> w.inverse_filter_bank == (w.rec_lo[::-1], w.rec_hi[::-1],
                                      w.dec_lo[::-1], w.dec_hi[::-1])
        True
        """
        def __get__(self):
            return (self.rec_lo[::-1], self.rec_hi[::-1], self.dec_lo[::-1],
                    self.dec_hi[::-1])

    def get_reverse_filters_coeffs(self):
        warnings.warn("The `get_reverse_filters_coeffs` method is deprecated. "
                      "Use `inverse_filter_bank` attribute instead.",
                      DeprecationWarning)
        return self.inverse_filter_bank

    def wavefun(self, int level=8):
        """
        wavefun(self, level=8)

        Calculates approximations of scaling function (`phi`) and wavelet
        function (`psi`) on xgrid (`x`) at a given level of refinement.

        Parameters
        ----------
        level : int, optional
            Level of refinement (default: 8).

        Returns
        -------
        [phi, psi, x] : array_like
            For orthogonal wavelets returns scaling function, wavelet function
            and xgrid

        [phi_d, psi_d, phi_r, psi_r, x] : array_like
            For biorthogonal wavelets returns scaling and wavelet function both
            for decomposition and reconstruction and xgrid

        Examples
        --------
        >>> import pywt
        >>> # Orthogonal
        >>> wavelet = pywt.Wavelet('db2')
        >>> phi, psi, x = wavelet.wavefun(level=5)
        >>> # Biorthogonal
        >>> wavelet = pywt.Wavelet('bior3.5')
        >>> phi_d, psi_d, phi_r, psi_r, x = wavelet.wavefun(level=5)
        """
        cdef pywt_index_t filter_length "filter_length"
        cdef pywt_index_t right_extent_length "right_extent_length"
        cdef pywt_index_t output_length "output_length"
        cdef pywt_index_t keep_length "keep_length"
        cdef np.float64_t n, n_mul
        cdef np.float64_t[::1] n_arr = <np.float64_t[:1]> &n,
        cdef np.float64_t[::1] n_mul_arr = <np.float64_t[:1]> &n_mul
        cdef double p "p"
        cdef double mul "mul"
        cdef Wavelet other "other"
        cdef phi_d, psi_d, phi_r, psi_r

        n = pow(sqrt(2.), <double>level)
        p = (pow(2., <double>level))

        if self.w.orthogonal:
            filter_length = self.w.dec_len
            output_length = <pywt_index_t> ((filter_length-1) * p + 1)
            keep_length = get_keep_length(output_length, level, filter_length)
            output_length = fix_output_length(output_length, keep_length)

            right_extent_length = get_right_extent_length(output_length,
                                                          keep_length)

            # phi, psi, x
            return [np.concatenate(([0.],
                                    keep(upcoef(True, n_arr, self, level, 0), keep_length),
                                    np.zeros(right_extent_length))),
                    np.concatenate(([0.],
                                    keep(upcoef(False, n_arr, self, level, 0), keep_length),
                                    np.zeros(right_extent_length))),
                    np.linspace(0.0, (output_length-1)/p, output_length)]
        else:
            if self.w.biorthogonal and (self.w.vanishing_moments_psi % 4) != 1:
                # FIXME: I don't think this branch is well tested
                n_mul = -n
            else:
                n_mul = n

            other = Wavelet(filter_bank=self.inverse_filter_bank)

            filter_length  = other.w.dec_len
            output_length = <pywt_index_t> ((filter_length-1) * p)
            keep_length = get_keep_length(output_length, level, filter_length)
            output_length = fix_output_length(output_length, keep_length)
            right_extent_length = get_right_extent_length(output_length, keep_length)

            phi_d  = np.concatenate(([0.],
                                     keep(upcoef(True, n_arr, other, level, 0), keep_length),
                                     np.zeros(right_extent_length)))
            psi_d  = np.concatenate(([0.],
                                     keep(upcoef(False, n_mul_arr, other, level, 0),
                                          keep_length),
                                     np.zeros(right_extent_length)))

            filter_length = self.w.dec_len
            output_length = <pywt_index_t> ((filter_length-1) * p)
            keep_length = get_keep_length(output_length, level, filter_length)
            output_length = fix_output_length(output_length, keep_length)
            right_extent_length = get_right_extent_length(output_length, keep_length)

            phi_r  = np.concatenate(([0.],
                                     keep(upcoef(True, n_arr, self, level, 0), keep_length),
                                     np.zeros(right_extent_length)))
            psi_r  = np.concatenate(([0.],
                                     keep(upcoef(False, n_mul_arr, self, level, 0),
                                          keep_length),
                                     np.zeros(right_extent_length)))

            return [phi_d, psi_d, phi_r, psi_r,
                    np.linspace(0.0, (output_length - 1) / p, output_length)]

    def __str__(self):
        s = []
        for x in [
            u"Wavelet %s" % self.name,
            u"  Family name:    %s" % self.family_name,
            u"  Short name:     %s" % self.short_family_name,
            u"  Filters length: %d" % self.dec_len,
            u"  Orthogonal:     %s" % self.orthogonal,
            u"  Biorthogonal:   %s" % self.biorthogonal,
            u"  Symmetry:       %s" % self.symmetry
            ]:
            s.append(x.rstrip())
        return u'\n'.join(s)

    def __repr__(self):
        repr = "{module}.{classname}(name='{name}', filter_bank={filter_bank})"
        return repr.format(module=type(self).__module__,
                           classname=type(self).__name__,
                           name=self.name,
                           filter_bank=self.filter_bank)


cdef pywt_index_t get_keep_length(pywt_index_t output_length,
                             int level, pywt_index_t filter_length):
    cdef pywt_index_t lplus "lplus"
    cdef pywt_index_t keep_length "keep_length"
    cdef int i "i"
    lplus = filter_length - 2
    keep_length = 1
    for i in range(level):
        keep_length = 2*keep_length+lplus
    return keep_length

cdef pywt_index_t fix_output_length(pywt_index_t output_length, pywt_index_t keep_length):
    if output_length-keep_length-2 < 0:
        output_length = keep_length+2
    return output_length

cdef pywt_index_t get_right_extent_length(pywt_index_t output_length, pywt_index_t keep_length):
    return output_length - keep_length - 1


def wavelet_from_object(wavelet):
    return c_wavelet_from_object(wavelet)


cdef c_wavelet_from_object(wavelet):
    if isinstance(wavelet, Wavelet):
        return wavelet
    else:
        return Wavelet(wavelet)


cpdef np.dtype _check_dtype(data):
    """Check for cA/cD input what (if any) the dtype is."""
    cdef np.dtype dt
    try:
        dt = data.dtype
        if dt not in (np.float64, np.float32):
            # integer input was always accepted; convert to float64
            dt = np.dtype('float64')
    except AttributeError:
        dt = np.dtype('float64')
    return dt


# TODO: Can this be replaced by the take parameter of upcoef? Or vice-versa?
def keep(arr, keep_length):
    length = len(arr)
    if keep_length < length:
        left_bound = (length - keep_length) / 2
        return arr[left_bound:left_bound + keep_length]
    return arr


# Some utility functions

cdef object float64_array_to_list(double* data, pywt_index_t n):
    cdef pywt_index_t i
    cdef object app
    cdef object ret
    ret = []
    app = ret.append
    for i in range(n):
        app(data[i])
    return ret


cdef void copy_object_to_float64_array(source, double* dest) except *:
    cdef pywt_index_t i
    cdef double x
    i = 0
    for x in source:
        dest[i] = x
        i = i + 1


cdef void copy_object_to_float32_array(source, float* dest) except *:
    cdef pywt_index_t i
    cdef float x
    i = 0
    for x in source:
        dest[i] = x
        i = i + 1
