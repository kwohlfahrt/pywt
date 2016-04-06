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

BaseNode - a common interface of WaveletPacket and WaveletPacket2D
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. autoclass:: BaseNode
   :members:
   :special-members: __getitem__, __setitem__, __delitem__

WaveletPacket and WaveletPacket tree Node
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. class:: Node(BaseNode)
           WaveletPacket(Node)

  .. attribute:: node_name

     Node name describing :attr:`~BaseNode.data` coefficients type of the
     current subnode.

     For :class:`WaveletPacket` case it is just as in :func:`dwt`:
        - ``a`` - approximation coefficients
        - ``d`` - details coefficients

  .. method:: decompose()

    .. seealso::

        - :func:`dwt` for 1D Discrete Wavelet Transform output coefficients.


.. autoclass:: WaveletPacket
   :members:

WaveletPacket2D and WaveletPacket2D tree Node2D
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. class:: Node2D(BaseNode)
           WaveletPacket2D(Node2D)

  .. attribute:: node_name

     For :class:`WaveletPacket2D` case it is just as in :func:`dwt2`:
        - ``a`` - approximation coefficients (`LL`)
        - ``h`` - horizontal detail coefficients (`LH`)
        - ``v`` - vertical detail coefficients (`HL`)
        - ``d`` - diagonal detail coefficients (`HH`)

  .. method:: decompose()

     .. seealso::

        :func:`dwt2` for 2D Discrete Wavelet Transform output coefficients.

  .. method:: expand_2d_path(self, path):

.. autoclass:: WaveletPacket2D
   :members:
