# raylib parser

This parser scans [`raylib.h`](../src/raylib.h) to get information about `defines`, `structs`, `enums` and `functions`.
All data is separated into parts, usually as strings. The following types are used for data:

 - `struct DefineInfo`
 - `struct FunctionInfo`
 - `struct StructInfo`
 - `struct EnumInfo`
 
Check `raylib_parser.c` for details about those structs.

## Command Line

```
//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
// raylib API parser                                                            //
//                                                                              //
// more info and bugs-report: github.com/raysan5/raylib/parser                  //
//                                                                              //
// Copyright (c) 2021-2022 Ramon Santamaria (@raysan5)                          //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////

USAGE:

    > raylib_parser [--help] [--input <filename.h>] [--output <filename.ext>] [--format <type>]

OPTIONS:

    -h, --help                      : Show tool version and command line usage help

    -i, --input <filename.h>        : Define input header file to parse.
                                      NOTE: If not specified, defaults to: raylib.h

    -o, --output <filename.ext>     : Define output file and format.
                                      Supported extensions: .txt, .json, .xml, .h
                                      NOTE: If not specified, defaults to: raylib_api.txt

    -f, --format <type>             : Define output format for parser data.
                                      Supported types: DEFAULT, JSON, XML, LUA

    -d, --define <DEF>              : Define functions specifiers (i.e. RLAPI for raylib.h, RMDEF for raymath.h, etc.)
                                      NOTE: If no specifier defined, defaults to: RLAPI

    -t, --truncate <after>          : Define string to truncate input after (i.e. "RLGL IMPLEMENTATION" for rlgl.h)
                                      NOTE: If not specified, the full input file is parsed.


EXAMPLES:

    > raylib_parser --input raylib.h --output api.json
        Process <raylib.h> to generate <api.json>

    > raylib_parser --output raylib_data.info --format XML
        Process <raylib.h> to generate <raylib_data.info> as XML text data

    > raylib_parser --input raymath.h --output raymath_data.info --format XML
        Process <raymath.h> to generate <raymath_data.info> as XML text data
```

## Constraints

This parser is specifically designed to work with raylib.h, so, it has some constraints: 

 - Functions are expected as a single line with the following structure:
```
   <retType> <name>(<paramType[0]> <paramName[0]>, <paramType[1]> <paramName[1]>);  <desc>
```
   Be careful with functions broken into several lines, it breaks the process!
   
 - Structures are expected as several lines with the following form:
```
   <desc>
   typedef struct <name> {
       <fieldType[0]> <fieldName[0]>;  <fieldDesc[0]>
       <fieldType[1]> <fieldName[1]>;  <fieldDesc[1]>
       <fieldType[2]> <fieldName[2]>;  <fieldDesc[2]>
   } <name>;
```
 - Enums are expected as several lines with the following form:
```
   <desc>
   typedef enum {
       <valueName[0]> = <valueInteger[0]>, <valueDesc[0]>
       <valueName[1]>,
       <valueName[2]>, <valueDesc[2]>
       <valueName[3]>  <valueDesc[3]>
   } <name>;
```   
   
_NOTE: For enums, multiple options are supported:_
      
 - If value is not provided, (<valueInteger[i -1]> + 1) is assigned
 - Value description can be provided or not

## Additional notes

This parser _could_ work with other C header files if mentioned constraints are followed.

This parser **does not require `<string.h>` library**, all data is parsed directly from char buffers.

### LICENSE: zlib/libpng

raylib-parser is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.

