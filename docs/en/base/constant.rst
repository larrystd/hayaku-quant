.. py:currentmodule:: hayaku
.. highlight:: python

Global Variables and Constants
==============================

Null Values and Security Types
------------------------------

.. py:data:: constant

    The global constants, an instance of :py:class:`Constant`, used to judge the related null values and the stock types, e.g.:
    
    ::
        
        a = Datetime(201601010000)
        if (a == constant.null_datetime ):
            print(True)

.. py:class:: Constant

    .. py:attribute:: null_datetime An invalid Datetime
    
    .. py:attribute:: inf

    .. py:attribute:: nan

    .. py:attribute:: null_price The same as nan

    .. py:attribute:: null_int An invalid int
    
    .. py:attribute:: null_size An invalid size
     
    .. py:attribute:: null_int64 An invalid int64
    
    .. py:attribute:: pickle_support Whether pickle is supported
    
    .. py:attribute:: STOCKTYPE_BLOCK Stock type - Block
    
    .. py:attribute:: STOCKTYPE_A Stock type - A-share
    
    .. py:attribute:: STOCKTYPE_INDEX Stock type - Index
    
    .. py:attribute:: STOCKTYPE_B Stock type - B-share
    
    .. py:attribute:: STOCKTYPE_FUND Stock type - Fund
    
    .. py:attribute:: STOCKTYPE_ETF Stock type - ETF
    
    .. py:attribute:: STOCKTYPE_ND Stock type - Treasury bond
    
    .. py:attribute:: STOCKTYPE_BOND Stock type - Other bonds
    
    .. py:attribute:: STOCKTYPE_GEM Stock type - ChiNext (GEM)

    .. py:attribute:: STOCKTYPE_START Stock type - ChiNext (GEM)

    .. py:attribute:: STOCKTYPE_A_BJ Stock type - A-share of the Beijing Stock Exchange
    
    .. py:attribute:: STOCKTYPE_TMP Stock type - Temporary CSV
