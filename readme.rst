Quick Reference for Common Conversions
=======================================

Include Header
--------------

.. code-block:: cpp

   #include <wutils/wutils.hpp>

Common Conversions
------------------

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Function
     - Conversion
   * - ``wutils::ws(str).value``
     - string → wstring
   * - ``wutils::s(wstr).value``
     - wstring → string
   * - ``wutils::u8s(str).value``
     - string → u8string
   * - ``wutils::u16s(str).value``
     - string → u16string
   * - ``wutils::u32s(str).value``
     - string → u32string
