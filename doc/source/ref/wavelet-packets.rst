.. _ref-wp:

.. currentmodule:: pywt
.. include:: ../substitutions.rst

===============
Wavelet Packets
===============

Wavelet packets are a tree structure that provides wavelet coefficients for a
data set at arbitrary levels. They are available for 1D and 2D data.

The node classes are used as data wrappers and can be organized in trees (binary
trees for 1D transform case and quad-trees for the 2D one).

The diagram below illustrates the inheritance tree:

- :class:`~pywt.BaseNode` - common interface for 1D and 2D nodes:

  - :class:`~pywt.Node` - data carrier node in a 1D decomposition tree

    - :class:`~pywt.WaveletPacket` - 1D decomposition tree root node

  - :class:`~pywt.Node2D` - data carrier node in a 2D decomposition tree

    - :class:`~pywt.WaveletPacket2D` - 2D decomposition tree root node

Common Interface
~~~~~~~~~~~~~~~~

.. autoclass:: BaseNode
   :members:
   :special-members: __getitem__, __setitem__, __delitem__

1D Packets
~~~~~~~~~~

.. autoclass:: Node
   :members:

.. autoclass:: WaveletPacket
   :members:

2D Packets
~~~~~~~~~~

.. autoclass:: Node2D
   :members:

.. autoclass:: WaveletPacket2D
   :members:
