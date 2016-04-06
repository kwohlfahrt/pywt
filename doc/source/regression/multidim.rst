.. _reg-multidim:

.. currentmodule:: pywt

Multidimensional DWT and SWT
============================

Most of the 1D decomposition functions will have 2D and n-D equivalents. They
will have the same name, with the suffix ``2`` or ``n`` respectively - e.g.
:func:`dwt2` for 2D DWT, and :func:`waverecn` for n-D multilevel decomposition.
They also take the same parameters, and differ only in their output format as
described on the reference pages.

Multidimensional DWT
--------------------

Decomposition
~~~~~~~~~~~~~

The multidimensional DWT works similarly to the 1D transform. The number of
coefficients returned depends on the dimensionality of the data:

>>> all(2 ** ndim == len(dwtn(np.zeros((4,) * ndim), 'haar')) for ndim in range(3))
True

Taking the transform is simple:

>>> import matplotlib.pyplot as plt
>>> x = pywt.data.aero()
>>> coeffs = pywt.dwtn(x, 'haar')
>>> sorted(coeffs.keys())
['aa', 'ad', 'da', 'dd']
>>> rec = pywt.idwtn(coeffs, 'haar')

And the result can be easily plotted:

>>> fig = plt.figure()
>>> titles = {'aa': 'Approximation', 'ad': 'Horizontal detail',
...           'da': 'Vertical detail', 'dd': 'Diagonal detail'}
>>> for i, key in enumerate(sorted(d.keys()), start=1):
...   ax = fig.add_subplot(2, 2, i)
...   ax.imshow(coeffs[key], cmap=plt.cm.gray)
...   ax.set_title(titles[key])
>>> fig = plt.figure()
>>> plt.imshow(rec, cmap=plt.cm.gray)
>>> plt.imshow()

For ``dwt2`` a similar method can be used, except the coefficients are in a
different format:

>>> coeffs = pywt.dwt2(x, 'haar')
>>> LL, (LH, HL, HH) = coeffs

Reconstruction
~~~~~~~~~~~~~~

The reconstruction functions take the coefficients in the same format.

>>> pywt.idwt2(coeffs, 'haar')

As with the 1D transform, any of the coefficients can be replaced by ``None`` to
simulate an array of zeros.

>>> pywt.idwt2((None, (None, HL, HH)), 'haar')
