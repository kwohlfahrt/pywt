.. _reg-wavelet:

.. currentmodule:: pywt

The Wavelet object
==================

:class:`Wavelet` objects are carriers of a :attr:`filter bank
<Wavelet.filter_bank>`, along with associated metadata.

.. _built-in wavelets:

Built-in Wavelets
-----------------

The most convenient way of getting a wavelet is to use one of the builtin named
Wavelets.

    >>> w = pywt.Wavelet('coif4')

These wavelets are organized into groups called wavelet families. The built-in
families are:

    >>> pywt.families()
    ['haar', 'db', 'sym', 'coif', 'bior', 'rbio', 'dmey']

The :func:`wavelist` function is used to obtain the list of wavelet names in
each family.

    >>> for family in pywt.families():
    ...     print("%s family: " % family + ', '.join(pywt.wavelist(family)))
    haar family: haar
    db family: db1, db2, db3, db4, db5, db6, db7, db8, db9, db10, db11, db12, db13, db14, db15, db16, db17, db18, db19, db20
    sym family: sym2, sym3, sym4, sym5, sym6, sym7, sym8, sym9, sym10, sym11, sym12, sym13, sym14, sym15, sym16, sym17, sym18, sym19, sym20
    coif family: coif1, coif2, coif3, coif4, coif5
    bior family: bior1.1, bior1.3, bior1.5, bior2.2, bior2.4, bior2.6, bior2.8, bior3.1, bior3.3, bior3.5, bior3.7, bior3.9, bior4.4, bior5.5, bior6.8
    rbio family: rbio1.1, rbio1.3, rbio1.5, rbio2.2, rbio2.4, rbio2.6, rbio2.8, rbio3.1, rbio3.3, rbio3.5, rbio3.7, rbio3.9, rbio4.4, rbio5.5, rbio6.8
    dmey family: dmey

To get the full list of builtin wavelets' names just use the :func:`wavelist`
with no argument. As you can see currently there are 76 builtin wavelets.

    >>> len(pywt.wavelist())
    76


Wavelet properties
------------------

But what can we do with :class:`Wavelet` objects? Well, they carry some
interesting information.

Printing a :class:`Wavelet` object shows brief information about its name, its
family name and some properties. Details of the properties can be found in the
API reference for the :class:`Wavelet` class.

    >>> print(w)
    Wavelet db3
      Family name:    Daubechies
      Short name:     db
      Filters length: 6
      Orthogonal:     True
      Biorthogonal:   True
      Symmetry:       asymmetric

But the most important information are the wavelet filters coefficients, which
are used in the various wavelet transforms. These coefficients can be obtained
via the :attr:`Wavelet.dec_lo`, :attr:`Wavelet.dec_hi`, :attr:`Wavelet.rec_lo`
and :attr:`Wavelet.rec_hi` attributes.

    >>> print(w.dec_lo)
    [0.03522629188210, -0.08544127388224, -0.13501102001039, 0.45987750211933, 0.80689150931334, 0.33267055295096]
    >>> print(w.dec_hi)
    [-0.33267055295096, 0.80689150931334, -0.45987750211933, -0.13501102001039, 0.08544127388224, 0.03522629188210]
    >>> print(w.rec_lo)
    [0.33267055295096, 0.80689150931334, 0.45987750211933, -0.13501102001039, -0.08544127388224, 0.03522629188210]
    >>> print(w.rec_hi)
    [0.03522629188210, 0.08544127388224, -0.13501102001039, -0.45987750211933, 0.80689150931334, -0.33267055295096]

Another way to get the filters data is to use the :attr:`Wavelet.filter_bank`
attribute, which returns all four filters in a tuple.

Custom Wavelets
---------------

If the wavelet you want to use is not part of the built-in set, you can specify
a custom wavelet from a filter bank. This can be done in two ways:

    1) Passing the filters coefficients directly as the ``filter_bank``
       parameter. They must be specified in the following order:

       1) :attr:`Wavelet.dec_lo`
       2) :attr:`Wavelet.dec_hi`
       3) :attr:`Wavelet.rec_lo`
       4) :attr:`Wavelet.rec_hi`

       >>> from math import sqrt
       >>> my_filter_bank = ([sqrt(2)/2, sqrt(2)/2], [-sqrt(2)/2, sqrt(2)/2],
       ...                   [sqrt(2)/2, sqrt(2)/2], [sqrt(2)/2, -sqrt(2)/2])
       >>> my_wavelet = pywt.Wavelet('My Haar Wavelet', filter_bank=my_filter_bank)

    2) Passing the filter bank object that implements the ``filter_bank``
       attribute. The attribute must return four filter coefficient arrays.

       >>> class MyHaarFilterBank(object):
       ...     @property
       ...     def filter_bank(self):
       ...         from math import sqrt
       ...         return ([sqrt(2)/2, sqrt(2)/2], [-sqrt(2)/2, sqrt(2)/2],
       ...                 [sqrt(2)/2, sqrt(2)/2], [sqrt(2)/2, -sqrt(2)/2])


       >>> my_wavelet = pywt.Wavelet('My Haar Wavelet', filter_bank=MyHaarFilterBank())

Note that such custom wavelets **will not** have all the properties set
to correct values:

    >>> print(my_wavelet)
    Wavelet My Haar Wavelet
      Family name:
      Short name:
      Filters length: 2
      Orthogonal:     False
      Biorthogonal:   False
      Symmetry:       unknown

    You can however set a few of them on your own:

    >>> my_wavelet.orthogonal = True
    >>> my_wavelet.biorthogonal = True

    >>> print(my_wavelet)
    Wavelet My Haar Wavelet
      Family name:
      Short name:
      Filters length: 2
      Orthogonal:     True
      Biorthogonal:   True
      Symmetry:       unknown


Wavelet Functions
-----------------

So far, the wavelet has been discussed as a filter bank, containing four
filters, becuase this is how ``pywt`` works internally. However, it can also
be described as the convolution with a wavelet and scaling function.

Many common wavelets do no have a closed form solution for the corresponding
wavelet and scaling functions, but can only be represented as a filter bank.
However, the wavelet and scaling functions can be approximated using the
`cascade algorithm <https://en.wikipedia.org/wiki/Cascade_algorithm>`_ - this is
the purpose of the :meth:`Wavelet.wavefun` method.

The number of returned values varies depending on the wavelet's
orthogonality property. For orthogonal wavelets the result is tuple
with scaling function (``phi``), wavelet function (``psi``) and xgrid
coordinates.

    >>> w = pywt.Wavelet('sym3')
    >>> w.orthogonal
    True
    >>> (phi, psi, x) = w.wavefun(level=5)

For biorthogonal (non-orthogonal) wavelets different scaling and wavelet
functions are used for decomposition and reconstruction, and thus five
elements are returned: decomposition scaling and wavelet functions
approximations, reconstruction scaling and wavelet functions approximations,
and the xgrid.

    >>> w = pywt.Wavelet('bior1.3')
    >>> w.orthogonal
    False
    >>> (phi_d, psi_d, phi_r, psi_r, x) = w.wavefun(level=5)

.. seealso:: You can find live examples of :meth:`Wavelet.wavefun` usage and
             images of all the built-in wavelets on the `Wavelet Properties
             Browser <http://wavelets.pybytes.com>`_ page.
