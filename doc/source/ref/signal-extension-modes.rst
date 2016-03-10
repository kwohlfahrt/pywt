.. _ref-modes:

.. currentmodule:: pywt

======================
Signal extension modes
======================

.. _Modes:

.. autodata:: pywt.Modes
   :annotation:

Naming Conventions
------------------
The correspondence between PyWavelets edge modes and the extension modes
available in Matlab's dwtmode and numpy's pad are tabulated here for reference.

================== ============= ===================
**PyWavelets**     **Matlab**    **numpy.pad**
================== ============= ===================
symmetric          sym, symh     symmetric
reflect            symw          reflect
smooth             spd, sp1      N/A
constant           sp0           edge
zero               zpd           constant, cval=0
periodic           ppd           wrap
periodization      per           N/A
N/A                asym, asymh   N/A
N/A                asymw         N/A
================== ============= ===================
