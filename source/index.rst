.. py-yajl documentation master file, created by
   sphinx-quickstart on Tue Dec 22 14:28:34 2009.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Meet py-yajl
===================================

The py-yajl project aims to provide Python bindings to "Yajl" 
(Yet Another JSON Library) originally developed by `Lloyd Hilaiel
<http://github.com/lloyd>`_. "Yet another Python JSON module?" you might
find yourself thinking, the advantages that Yajl brings to the table 
are:
  * Speed
  * Flexibility

Contents:

.. toctree::
   :maxdepth: 2

Example
------------
Using ``py-yajl`` is straight-forward and similar to using the :mod:`json` 
module introduced in `Python 2.6 <http://docs.python.org/whatsnew/2.6.html>`_.

**Encoding**:: 

    import yajl
    buffer = yajl.dumps({'foo' : 'bar'})

**Decoding**::

    import yajl
    obj = yajl.loads('{"foo":"bar"}')


Get the code
-------------
You can find releases of ``py-yajl`` in the `Python Package Index
<http://pypi.python.org/pypi/yajl>`_ or you can get the source from
`GitHub <http://www.github.com>`_ via:: 

    git clone git://github.com/rtyler/py-yajl.git

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

