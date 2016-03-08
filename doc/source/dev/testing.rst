.. _dev-testing:

Testing
=======

Continous integration
---------------------

The project is using `Travis-CI <https://travis-ci.org/PyWavelets/pywt>`_ and
`AppVeyor <https://ci.appveyor.com>`_ for continous integration and testing on
Linux and Windows respectively.

Current build status is:

.. image::
   https://travis-ci.org/PyWavelets/pywt.png?branch=master
   :alt: Travis-CI Build Status
   :target: https://travis-ci.org/PyWavelets/pywt

.. image::
   https://ci.appveyor.com/api/projects/status/wwlm4dxx9aum6qvd/branch/master?svg=true
   :alt: AppVeyor Build Status
   :target: https://ci.appveyor.com/project/PyWavelets/pywt/history

If you are submitting a patch or pull request please make sure it
does not break the build. The status will be shown on any pull requests
submitted via `Github <https://github.com/PyWavelets/pywt/pulls>`_.


Running tests locally
---------------------

Tests are implemented with `nose`_, so use one of:

    $ nosetests pywt

    >>> pywt.test()  # doctest: +SKIP

Note doctests require `Matplotlib`_ in addition to the usual dependencies.


Running tests with Tox
----------------------

There's also a config file for running tests with `Tox`_ (``pip install tox``).
To for example run tests for Python 2.7 and Python 3.4 use::

  tox -e py27,py34

For more information see the `Tox`_ documentation.


.. _nose: http://nose.readthedocs.org/en/latest/
.. _Tox: http://tox.testrun.org/
.. _Matplotlib: http://matplotlib.org
