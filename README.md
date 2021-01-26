# ExtSort
Sort files by given keys

See extsort /? for command line help.

The DOS/Windows command SORT is fast but it offers minimal features.
This command line utility extends the DOS command by adding a first step to build an indexes file. This file is passed to the command SORT. Then the records are written to the output file based on the sorted indexes.
So it offers the possiblity to sort file(s) depending on several keys that can be present at any place in the input file(s).

It supports:
  - string, numeric and date key types,
  - linux, windows or mac text files,
  - delimited and not delimited structures of fields.
  
Two precision status simple and double are supported for numeric key type. Numeric values are stored in indexes using a scientific-like format. Values are sorted based on sign,  exponent sign, exponent value and significant value. Simple values are defined with a 2 digits exponent and 8 digits significant value. Double values are defined with a 3 digits exponent limited to +/- 328 and a 17 digits significant value. An error occurs if the value exceeds the 64 bits capacity of IEEE-754 standard. An option can avoid some conversion errors, at the expense of precision.

For delimited fields, the maximum length that will be used to build indexes must be provided for string values.

Note that date values are not checked, the digits for year, month and day are considered as defined by the given format.

Binaries are provided for 32 and 64 bits Windows platforms. An Unix/Linux port will not be provided since the native command SORT already supports many features.


Version history:
  1.0 First release.
