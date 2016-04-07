.. _reg-wp:

.. currentmodule:: pywt

Wavelet Packets
===============

Wavelet packets structure the wavelet decomposition into a tree, with
approximation and detail decompositions available for each node in the tree.

Creating the Tree
-----------------

The root node (:class:`WaveletPacket`) is created from the original signal:

    >>> x = [1, 2, 3, 4, 5, 6, 7, 8]
    >>> wp = pywt.WaveletPacket(data=x, wavelet='db1', mode='symmetric')

Each node stores it's coefficients in the :attr:`WaveletPacket.data` attribute.
For the root node, this is the original signale:

    >>> print(wp.data)
    [1, 2, 3, 4, 5, 6, 7, 8]

:class:`Nodes <Node>` are identified by :attr:`paths <~Node.path>`. For the root
node the path is ``''`` and the decomposition level is ``0``.

    >>> print(repr(wp.path))
    ''
    >>> print(wp.level)
    0

The maximum decomposition level, if not given as param in the constructor, is
automatically computed. It is the same for all nodes in the tree:

    >>> print(wp.maxlevel)
    3

Traversing the Tree
-------------------

Accessing subnodes
~~~~~~~~~~~~~~~~~~

Sub-nodes can be accessed by their key, which depends on the dimensionality of
the transform. See the `reference docs <ref-wp>`_ for details. For the 1D
transform, the keys are ``'a'`` for the approximation coefficients and ``'d'``
for detail coefficients.

    * 1st level:

        >>> print(wp['a'].data)
        [  2.12132034   4.94974747   7.77817459  10.60660172]
        >>> print(wp['a'].path)
        a

    * 2nd level:

        >>> print(wp['aa'].data)
        [  5.  13.]
        >>> print(wp['aa'].path)
        aa


    * 3rd level:

        >>> print(wp['aaa'].data)
        [ 12.72792206]
        >>> print(wp['aaa'].path)
        aaa

Exceeding the maximum depth results in an :exc:`IndexError`:

    >>> print(wp['aaaa'].data)
    Traceback (most recent call last):
    ...
    IndexError: Path length is out of range.

While an invalid path is a :exc:`ValueError`:

    >>> print(wp['ac'])
    Traceback (most recent call last):
    ...
    ValueError: Subnode name must be in ['a', 'd'], not 'c'.

A nodes attributes are documented on the `reference pages <ref-wp>`_.

Collecting nodes
~~~~~~~~~~~~~~~~

We can get all nodes on the particular level either in ``natural`` order:

    >>> print([node.path for node in wp.get_level(3, 'natural')])
    ['aaa', 'aad', 'ada', 'add', 'daa', 'dad', 'dda', 'ddd']

or sorted based on the band frequency (``freq``):

    >>> print([node.path for node in wp.get_level(3, 'freq')])
    ['aaa', 'aad', 'add', 'ada', 'dda', 'ddd', 'dad', 'daa']

This method can be used to display the coefficients at each level:

    >>> import matplotlib.pyplot as plt, numpy as np
    >>> ecg_wp = pywt.WaveletPacket(pywt.data.ecg(), 'haar', 'symmetric')
    >>> nodes = ecg_wp.get_level(3)
    >>> labels = [n.path for n in nodes]
    >>> values = -abs(np.array([n.data for n in nodes]))
    >>> fig = plt.figure()
    >>> ax = fig.add_subplot(1, 1, 1)
    >>> ax.imshow(values, interpolation='nearest', aspect='auto', cmap='bone')
    ...
    >>> ax.set_yticks(range(len(labels)))
    ...
    >>> ax.set_yticklabels(labels)
    ...
    >>> plt.show()

This will display the coefficients at level ``3``, with the approximation
coefficients near the top of the image and the detail coefficients towards the
bottom.

Reconstructing data from Wavelet Packets:
-----------------------------------------

Now create a new :class:`Wavelet Packet <WaveletPacket>` and set its nodes with
some data.

    >>> new_wp = pywt.WaveletPacket(data=None, wavelet='db1', mode='symmetric')

    >>> new_wp['aa'] = wp['aa'].data
    >>> new_wp['ad'] = [-2., -2.]

For convenience, :attr:`Node.data` gets automatically extracted from the
:class:`Node` object:

    >>> new_wp['d'] = wp['d']

And reconstruct the data from the ``aa``, ``ad`` and ``d`` packets.

    >>> print(new_wp.reconstruct(update=False))
    [ 1.  2.  3.  4.  5.  6.  7.  8.]

If the *update* param in the reconstruct method is set to ``False``, the node's
:attr:`~Node.data` will not be updated.

    >>> print(new_wp.data)
    None

Otherwise, the :attr:`~Node.data` attribute will be set to the reconstructed
value.

    >>> print(new_wp.reconstruct(update=True))
    [ 1.  2.  3.  4.  5.  6.  7.  8.]
    >>> print(new_wp.data)
    [ 1.  2.  3.  4.  5.  6.  7.  8.]

The leaf nodes can be accessed using :meth:`Wavelet_acket.get_leaf_nodes`.

    >>> print([n.path for n in new_wp.get_leaf_nodes()])
    ['aa', 'ad', 'd']

Removing Nodes
--------------

Nodes in the tree can be accessed just like dictionaries. We've already seen
insertion and retrieval of nodes. Nodes can also be deleted with ``del``

    >>> node = new_wp['ad']
    >>> del new_wp['ad']

The leaf nodes that left in the tree are:

    >>> print([n.path for n in new_wp.get_leaf_nodes()])
    ['aa', 'd']

And the reconstruction is:

    >>> print(wp.reconstruct())
    [ 2.  3.  2.  3.  6.  7.  6.  7.]

Now restore the deleted node value.

    >>> wp['ad'].data = node.data

Printing leaf nodes and tree reconstruction confirms the original state of the
tree:

    >>> print(wp.reconstruct())
    [ 1.  2.  3.  4.  5.  6.  7.  8.]
