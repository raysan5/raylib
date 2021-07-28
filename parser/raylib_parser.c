/**********************************************************************************************

    raylib API parser

    This parser scans raylib.h to get API information about structs, enums and functions.
    All data is divided into pieces, usually as strings. The following types are used for data:

     - struct FunctionInfo
     - struct StructInfo
     - struct EnumInfo

    CONSTRAINTS:

    This parser is specifically designed to work with raylib.h, so, it has some constraints:

     - Functions are expected as a single line with the following structure:

       <retType> <name>(<paramType[0]> <paramName[0]>, <paramType[1]> <paramName[1]>);  <desc>

       Be careful with functions broken into several lines, it breaks the process!

     - Structures are expected as several lines with the following form:

       <desc>
       typedef struct <name> {
           <fieldType[0]> <fieldName[0]>;  <fieldDesc[0]>
           <fieldType[1]> <fieldName[1]>;  <fieldDesc[1]>
           <fieldType[2]> <fieldName[2]>;  <fieldDesc[2]>
       } <name>;

     - Enums are expected as several lines with the following form:

       <desc>
       typedef enum {
           <valueName[0]> = <valueInteger[0]>, <valueDesc[0]>
           <valueName[1]>,
           <valueName[2]>, <valueDesc[2]>
           <valueName[3]>  <valueDesc[3]>
       } <name>;

       NOTE: Multiple options are supported for enums:
          - If value is not provided, (<valueInteger[i -1]> + 1) is assigned
          - Value description can be provided or not

    OTHER NOTES:

     - This parser could work with other C header files if mentioned constraints are followed.

     - This parser does not require <string.h> library, all data is parsed directly from char buffers.

    LICENSE: zlib/libpng

    raylib-parser is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
    BSD-like license that allows static linking with closed source software:

    Copyright (c) 2021 Ramon Santamaria (@raysan5)

**********************************************************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>             // Required for: malloc(), calloc(), realloc(), free(), atoi(), strtol()
#include <stdio.h>              // Required for: printf(), fopen(), fseek(), ftell(), fread(), fclose()
#include <stdbool.h>            // Required for: bool

#define MAX_FUNCS_TO_PARSE       512    // Maximum number of functions to parse
#define MAX_STRUCTS_TO_PARSE      64    // Maximum number of structures to parse
#define MAX_ENUMS_TO_PARSE        64    // Maximum number of enums to parse

#define MAX_LINE_LENGTH          512    // Maximum length of one line (including comments)
#define MAX_STRUCT_LINE_LENGTH  2048    // Maximum length of one struct (multiple lines)

#define MAX_FUNCTION_PARAMETERS   12    // Maximum number of function parameters
#define MAX_STRUCT_FIELDS         32    // Maximum number of struct fields
#define MAX_ENUM_VALUES          512    // Maximum number of enum values

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Function info data
typedef struct FunctionInfo {
    char name[64];              // Function name
    char desc[128];             // Function description (comment at the end)
    char retType[32];           // Return value type
    int paramCount;             // Number of function parameters
    char paramType[MAX_FUNCTION_PARAMETERS][32];   // Parameters type
    char paramName[MAX_FUNCTION_PARAMETERS][32];   // Parameters name
    char paramDesc[MAX_FUNCTION_PARAMETERS][8];    // Parameters description
} FunctionInfo;

// Struct info data
typedef struct StructInfo {
    char name[64];              // Struct name
    char desc[64];              // Struct type description
    int fieldCount;             // Number of fields in the struct
    char fieldType[MAX_STRUCT_FIELDS][64];     // Field type
    char fieldName[MAX_STRUCT_FIELDS][64];     // Field name
    char fieldDesc[MAX_STRUCT_FIELDS][128];    // Field description
} StructInfo;

// Enum info data
typedef struct EnumInfo {
    char name[64];              // Enum name
    char desc[64];              // Enum description
    int valueCount;             // Number of values in enumerator
    char valueName[MAX_ENUM_VALUES][64];    // Value name definition
    int valueInteger[MAX_ENUM_VALUES];      // Value integer
    char valueDesc[MAX_ENUM_VALUES][64];    // Value description
} EnumInfo;

// Output format for parsed data
typedef enum { DEFAULT = 0, JSON, XML } OutputFormat;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static int funcCount = 0;
static int structCount = 0;
static int enumCount = 0;
static FunctionInfo *funcs = NULL;
static StructInfo *structs = NULL;
static EnumInfo *enums = NULL;
static char apiDefine[32] = "RLAPI\0";

// Command line variables
static char inFileName[512] = { 0 };       // Input file name (required in case of provided through CLI)
static char outFileName[512] = { 0 };      // Output file name (required for file save/export)
static int outputFormat = DEFAULT;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void ShowCommandLineInfo(void);                      // Show command line usage info
static void ProcessCommandLine(int argc, char *argv[]);     // Process command line input

static char *LoadFileText(const char *fileName, int *length);
static char **GetTextLines(const char *buffer, int length, int *linesCount);
static void GetDataTypeAndName(const char *typeName, int typeNameLen, char *type, char *name);
static unsigned int TextLength(const char *text);           // Get text length in bytes, check for \0 character
static bool IsTextEqual(const char *text1, const char *text2, unsigned int count);
static void MemoryCopy(void *dest, const void *src, unsigned int count);
static char* CharReplace(char* text, char search, char replace);

static void ExportParsedData(const char *fileName, int format); // Export parsed data in desired format

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    if (argc > 1) ProcessCommandLine(argc, argv);

    if (inFileName[0] == '\0') MemoryCopy(inFileName, "../src/raylib.h\0", 16);

    int length = 0;
    char *buffer = LoadFileText(inFileName, &length);

    // Preprocess buffer to get separate lines
    // NOTE: GetTextLines() also removes leading spaces/tabs
    int linesCount = 0;
    char **lines = GetTextLines(buffer, length, &linesCount);

    // Function lines pointers, selected from buffer "lines"
    char **funcLines = (char **)malloc(MAX_FUNCS_TO_PARSE*sizeof(char *));

    // Structs data (multiple lines), selected from "buffer"
    char **structLines = (char **)malloc(MAX_STRUCTS_TO_PARSE*sizeof(char *));
    for (int i = 0; i < MAX_STRUCTS_TO_PARSE; i++) structLines[i] = (char *)calloc(MAX_STRUCT_LINE_LENGTH, sizeof(char));

    // Enums lines pointers, selected from buffer "lines"
    int *enumLines = (int *)malloc(MAX_ENUMS_TO_PARSE*sizeof(int));

    // Prepare required lines for parsing
    //--------------------------------------------------------------------------------------------------

    // Read function lines
    for (int i = 0; i < linesCount; i++)
    {
        // Read function line (starting with `define`, i.e. for raylib.h "RLAPI")
        if (IsTextEqual(lines[i], apiDefine, TextLength(apiDefine)))
        {
            // Keep a pointer to the function line
            funcLines[funcCount] = lines[i];
            funcCount++;
        }
    }

    // Read structs data (multiple lines, read directly from buffer)
    // TODO: Parse structs data from "lines" instead of "buffer" -> Easier to get struct definition
    for (int i = 0; i < length; i++)
    {
        // Read struct data (starting with "typedef struct", ending with '} ... ;')
        // NOTE: We read it directly from buffer
        if (IsTextEqual(buffer + i, "typedef struct", 14))
        {
            int j = 0;
            bool validStruct = false;

            // WARNING: Typedefs between types: typedef Vector4 Quaternion;

            for (int c = 0; c < 128; c++)
            {
                if (buffer[i + c] == '{')
                {
                    validStruct = true;
                    break;
                }
                else if (buffer[i + c] == ';')
                {
                    // Not valid struct:
                    // i.e typedef struct rAudioBuffer rAudioBuffer; -> Typedef and forward declaration
                    i += c;
                    break;
                }
            }

            if (validStruct)
            {
                while (buffer[i + j] != '}')
                {
                    structLines[structCount][j] = buffer[i + j];
                    j++;
                }

                while (buffer[i + j] != '\n')
                {
                    structLines[structCount][j] = buffer[i + j];
                    j++;
                }

                i += j;
                structCount++;
            }
        }
    }

    // Read enum lines
    for (int i = 0; i < linesCount; i++)
    {
        // Read enum line
        if (IsTextEqual(lines[i], "typedef enum {", 14))
        {
            // Keep the line position in the array of lines,
            // so, we can scan that position and following lines
            enumLines[enumCount] = i;
            enumCount++;
        }
    }

    // At this point we have all raylib structs, enums, functions lines data to start parsing

    free(buffer);       // Unload text buffer

    // Parsing raylib data
    //--------------------------------------------------------------------------------------------------

    // Structs info data
    structs = (StructInfo *)calloc(MAX_STRUCTS_TO_PARSE, sizeof(StructInfo));

    for (int i = 0; i < structCount; i++)
    {
        int structLineOffset = 0;

        // Get struct name: typedef struct name {
        for (int c = 15; c < 64 + 15; c++)
        {
            if (structLines[i][c] == '{')
            {
                structLineOffset = c + 2;

                MemoryCopy(structs[i].name, &structLines[i][15], c - 15 - 1);
                break;
            }
        }

        // Get struct fields and count them -> fields finish with ;
        int j = 0;
        while (structLines[i][structLineOffset + j] != '}')
        {
            // WARNING: Some structs have empty spaces and comments -> OK, processed

            int fieldStart = 0;
            if ((structLines[i][structLineOffset + j] != ' ') && (structLines[i][structLineOffset + j] != '\n')) fieldStart = structLineOffset + j;

            if (fieldStart != 0)
            {
                // Scan one field line
                int c = 0;
                int fieldEndPos = 0;
                char fieldLine[256] = { 0 };

                while (structLines[i][structLineOffset + j] != '\n')
                {
                    if (structLines[i][structLineOffset + j] == ';') fieldEndPos = c;
                    fieldLine[c] = structLines[i][structLineOffset + j];
                    c++; j++;
                }

                if (fieldLine[0] != '/')    // Field line is not a comment
                {
                    //printf("Struct field: %s_\n", fieldLine);     // OK!

                    // Get struct field type and name
                    GetDataTypeAndName(fieldLine, fieldEndPos, structs[i].fieldType[structs[i].fieldCount], structs[i].fieldName[structs[i].fieldCount]);

                    // Get the field description
                    // We start skipping spaces in front of description comment
                    int descStart = fieldEndPos;
                    while ((fieldLine[descStart] != '/') && (fieldLine[descStart] != '\0')) descStart++;

                    int k = 0;
                    while ((fieldLine[descStart + k] != '\0') && (fieldLine[descStart + k] != '\n'))
                    {
                        structs[i].fieldDesc[structs[i].fieldCount][k] = fieldLine[descStart + k];
                        k++;
                    }

                    structs[i].fieldCount++;
                }
            }

            j++;
        }

    }

    for (int i = 0; i < MAX_STRUCTS_TO_PARSE; i++) free(structLines[i]);
    free(structLines);

    // Enum info data
    enums = (EnumInfo *)calloc(MAX_ENUMS_TO_PARSE, sizeof(EnumInfo));

    for (int i = 0; i < enumCount; i++)
    {
        // TODO: Get enum description from lines[enumLines[i] - 1]

        for (int j = 1; j < MAX_ENUM_VALUES*2; j++)   // Maximum number of lines following enum first line
        {
            char *linePtr = lines[enumLines[i] + j];

            if ((linePtr[0] >= 'A') && (linePtr[0] <= 'Z'))
            {
                // Parse enum value line, possible options:
                //ENUM_VALUE_NAME,
                //ENUM_VALUE_NAME
                //ENUM_VALUE_NAME     = 99
                //ENUM_VALUE_NAME     = 99,
                //ENUM_VALUE_NAME     = 0x00000040,   // Value description

                // We start reading the value name
                int c = 0;
                while ((linePtr[c] != ',') &&
                       (linePtr[c] != ' ') &&
                       (linePtr[c] != '=') &&
                       (linePtr[c] != '\0')) { enums[i].valueName[enums[i].valueCount][c] = linePtr[c]; c++; }

                // After the name we can have:
                //  '='  -> value is provided
                //  ','  -> value is equal to previous + 1, there could be a description if not '\0'
                //  ' '  -> value is equal to previous + 1, there could be a description if not '\0'
                //  '\0' -> value is equal to previous + 1

                // Let's start checking if the line is not finished
                if ((linePtr[c] != ',') && (linePtr[c] != '\0'))
                {
                    // Two options:
                    //  '='  -> value is provided
                    //  ' '  -> value is equal to previous + 1, there could be a description if not '\0'
                    bool foundValue = false;
                    while (linePtr[c] != '\0')
                    {
                        if (linePtr[c] == '=') { foundValue = true; break; }
                        c++;
                    }

                    if (foundValue)
                    {
                        if (linePtr[c + 1] == ' ') c += 2;
                        else c++;
  
                        // Parse integer value
                        int n = 0;
                        char integer[16] = { 0 };
  
                        while ((linePtr[c] != ',') && (linePtr[c] != ' ') && (linePtr[c] != '\0'))
                        {
                            integer[n] = linePtr[c];
                            c++; n++;
                        }
  
                        if (integer[1] == 'x') enums[i].valueInteger[enums[i].valueCount] = (int)strtol(integer, NULL, 16);
                        else enums[i].valueInteger[enums[i].valueCount] = atoi(integer);
                    }
                    else enums[i].valueInteger[enums[i].valueCount] = (enums[i].valueInteger[enums[i].valueCount - 1] + 1);

                    // TODO: Parse value description if any
                }
                else enums[i].valueInteger[enums[i].valueCount] = (enums[i].valueInteger[enums[i].valueCount - 1] + 1);

                enums[i].valueCount++;
            }
            else if (linePtr[0] == '}')
            {
                // Get enum name from typedef
                int c = 0;
                while (linePtr[2 + c] != ';') { enums[i].name[c] = linePtr[2 + c]; c++; }

                break;  // Enum ended, break for() loop
            }
        }
    }

    // Functions info data
    funcs = (FunctionInfo *)calloc(MAX_FUNCS_TO_PARSE, sizeof(FunctionInfo));

    for (int i = 0; i < funcCount; i++)
    {
        int funcParamsStart = 0;
        int funcEnd = 0;

        // Get return type and function name from func line
        for (int c = 0; (c < MAX_LINE_LENGTH) && (funcLines[i][c] != '\n'); c++)
        {
            if (funcLines[i][c] == '(')     // Starts function parameters
            {
                funcParamsStart = c + 1;

                // At this point we have function return type and function name
                char funcRetTypeName[128] = { 0 };
                int dc = TextLength(apiDefine) + 1;
                int funcRetTypeNameLen = c - dc;     // Substract `define` ("RLAPI " for raylib.h)
                MemoryCopy(funcRetTypeName, &funcLines[i][dc], funcRetTypeNameLen);

                GetDataTypeAndName(funcRetTypeName, funcRetTypeNameLen, funcs[i].retType, funcs[i].name);
                break;
            }
        }

        // Get parameters from func line
        for (int c = funcParamsStart; c < MAX_LINE_LENGTH; c++)
        {
            if (funcLines[i][c] == ',')     // Starts function parameters
            {
                // Get parameter type + name, extract info
                char funcParamTypeName[128] = { 0 };
                int funcParamTypeNameLen = c - funcParamsStart;
                MemoryCopy(funcParamTypeName, &funcLines[i][funcParamsStart], funcParamTypeNameLen);

                GetDataTypeAndName(funcParamTypeName, funcParamTypeNameLen, funcs[i].paramType[funcs[i].paramCount], funcs[i].paramName[funcs[i].paramCount]);

                funcParamsStart = c + 1;
                if (funcLines[i][c + 1] == ' ') funcParamsStart += 1;
                funcs[i].paramCount++;      // Move to next parameter
            }
            else if (funcLines[i][c] == ')')
            {
                funcEnd = c + 2;

                // Check if previous word is void
                if ((funcLines[i][c - 4] == 'v') && (funcLines[i][c - 3] == 'o') && (funcLines[i][c - 2] == 'i') && (funcLines[i][c - 1] == 'd')) break;

                // Get parameter type + name, extract info
                char funcParamTypeName[128] = { 0 };
                int funcParamTypeNameLen = c - funcParamsStart;
                MemoryCopy(funcParamTypeName, &funcLines[i][funcParamsStart], funcParamTypeNameLen);

                GetDataTypeAndName(funcParamTypeName, funcParamTypeNameLen, funcs[i].paramType[funcs[i].paramCount], funcs[i].paramName[funcs[i].paramCount]);

                funcs[i].paramCount++;      // Move to next parameter
                break;
            }
        }

        // Get function description
        for (int c = funcEnd; c < MAX_LINE_LENGTH; c++)
        {
            if (funcLines[i][c] == '/')
            {
                MemoryCopy(funcs[i].desc, &funcLines[i][c], 127);   // WARNING: Size could be too long for funcLines[i][c]?
                break;
            }
        }
    }

    for (int i = 0; i < linesCount; i++) free(lines[i]);
    free(lines);
    free(funcLines);

    // At this point, all raylib data has been parsed!
    //-----------------------------------------------------------------------------------------
    // structs[] -> We have all the structs decomposed into pieces for further analysis
    // enums[]   -> We have all the enums decomposed into pieces for further analysis
    // funcs[]   -> We have all the functions decomposed into pieces for further analysis

    // Process input file to output
    if (outFileName[0] == '\0') MemoryCopy(outFileName, "raylib_api.txt\0", 15);

    printf("\nInput file:       %s", inFileName);
    printf("\nOutput file:      %s", outFileName);
    if (outputFormat == DEFAULT) printf("\nOutput format:    DEFAULT\n\n");
    else if (outputFormat == JSON) printf("\nOutput format:    JSON\n\n");
    else if (outputFormat == XML) printf("\nOutput format:    XML\n\n");

    ExportParsedData(outFileName, outputFormat);

    free(funcs);
    free(structs);
    free(enums);
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Show command line usage info
static void ShowCommandLineInfo(void)
{
    printf("\n//////////////////////////////////////////////////////////////////////////////////\n");
    printf("//                                                                              //\n");
    printf("// raylib API parser                                                            //\n");
    printf("//                                                                              //\n");
    printf("// more info and bugs-report: github.com/raysan5/raylib/parser                  //\n");
    printf("//                                                                              //\n");
    printf("// Copyright (c) 2021 Ramon Santamaria (@raysan5)                               //\n");
    printf("//                                                                              //\n");
    printf("//////////////////////////////////////////////////////////////////////////////////\n\n");

    printf("USAGE:\n\n");
    printf("    > raylib_parser [--help] [--input <filename.h>] [--output <filename.ext>] [--format <type>] [--define <DEF>]\n");

    printf("\nOPTIONS:\n\n");
    printf("    -h, --help                      : Show tool version and command line usage help\n\n");
    printf("    -i, --input <filename.h>        : Define input header file to parse.\n");
    printf("                                      NOTE: If not specified, defaults to: raylib.h\n\n");
    printf("    -o, --output <filename.ext>     : Define output file and format.\n");
    printf("                                      Supported extensions: .txt, .json, .xml, .h\n");
    printf("                                      NOTE: If not specified, defaults to: raylib_api.txt\n\n");
    printf("    -f, --format <type>             : Define output format for parser data.\n");
    printf("                                      Supported types: DEFAULT, JSON, XML\n\n");
    printf("    -d, --define <DEF>              : Define functions define (i.e. RLAPI for raylib.h, RMDEF for raymath.h, etc\n");
    printf("                                      NOTE: If not specified, defaults to: RLAPI\n\n");

    printf("\nEXAMPLES:\n\n");
    printf("    > raylib_parser --input raylib.h --output api.json\n");
    printf("        Process <raylib.h> to generate <api.json>\n\n");
    printf("    > raylib_parser --output raylib_data.info --format XML\n");
    printf("        Process <raylib.h> to generate <raylib_data.info> as XML text data\n\n");
    printf("    > raylib_parser --input raymath.h --output raymath_data.info --format XML\n");
    printf("        Process <raymath.h> to generate <raymath_data.info> as XML text data\n\n");
}

// Process command line arguments
static void ProcessCommandLine(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (IsTextEqual(argv[i], "-h", 2) || IsTextEqual(argv[i], "--help", 6))
        {
            // Show info
            ShowCommandLineInfo();
        }
        else if (IsTextEqual(argv[i], "-i", 2) || IsTextEqual(argv[i], "--input", 7))
        {
            // Check for valid argument and valid file extension
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                MemoryCopy(inFileName, argv[i + 1], TextLength(argv[i + 1])); // Read input filename
                i++;
            }
            else printf("WARNING: No input file provided\n");
        }
        else if (IsTextEqual(argv[i], "-o", 2) || IsTextEqual(argv[i], "--output", 8))
        {
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                MemoryCopy(outFileName, argv[i + 1], TextLength(argv[i + 1])); // Read output filename
                i++;
            }
            else printf("WARNING: No output file provided\n");
        }
        else if (IsTextEqual(argv[i], "-f", 2) || IsTextEqual(argv[i], "--format", 8))
        {
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                if (IsTextEqual(argv[i + 1], "DEFAULT\0", 8)) outputFormat = DEFAULT;
                else if (IsTextEqual(argv[i + 1], "JSON\0", 5)) outputFormat = JSON;
                else if (IsTextEqual(argv[i + 1], "XML\0", 4)) outputFormat = XML;
            }
            else printf("WARNING: No format parameters provided\n");
        }
        else if (IsTextEqual(argv[i], "-d", 2) || IsTextEqual(argv[i], "--define", 8))
        {
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                MemoryCopy(apiDefine, argv[i + 1], TextLength(argv[i + 1])); // Read functions define
                apiDefine[TextLength(argv[i + 1])] = '\0';
                i++;
            }
            else printf("WARNING: No define key provided\n");
        }
    }
}

// Load text data from file, returns a '\0' terminated string
// NOTE: text chars array should be freed manually
static char *LoadFileText(const char *fileName, int *length)
{
    char *text = NULL;

    if (fileName != NULL)
    {
        FILE *file = fopen(fileName, "rt");

        if (file != NULL)
        {
            // WARNING: When reading a file as 'text' file,
            // text mode causes carriage return-linefeed translation...
            // ...but using fseek() should return correct byte-offset
            fseek(file, 0, SEEK_END);
            int size = ftell(file);
            fseek(file, 0, SEEK_SET);

            if (size > 0)
            {
                text = (char *)calloc((size + 1), sizeof(char));
                unsigned int count = (unsigned int)fread(text, sizeof(char), size, file);

                // WARNING: \r\n is converted to \n on reading, so,
                // read bytes count gets reduced by the number of lines
                if (count < (unsigned int)size)
                {
                    text = realloc(text, count + 1);
                    *length = count;
                }
                else *length = size;

                // Zero-terminate the string
                text[count] = '\0';
            }

            fclose(file);
        }
    }

    return text;
}

// Get all lines from a text buffer (expecting lines ending with '\n')
static char **GetTextLines(const char *buffer, int length, int *linesCount)
{
    // Get the number of lines in the text
    int count = 0;
    for (int i = 0; i < length; i++) if (buffer[i] == '\n') count++;

    printf("Number of text lines in buffer: %i\n", count);

    // Allocate as many pointers as lines
    char **lines = (char **)malloc(count*sizeof(char **));

    char *bufferPtr = (char *)buffer;

    for (int i = 0; (i < count) || (bufferPtr[0] != '\0'); i++)
    {
        lines[i] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));

        // Remove line leading spaces
        // Find last index of space/tab character
        int index = 0;
        while ((bufferPtr[index] == ' ') || (bufferPtr[index] == '\t')) index++;

        int j = 0;
        while (bufferPtr[index + j] != '\n')
        {
            lines[i][j] = bufferPtr[index + j];
            j++;
        }

        bufferPtr += (index + j + 1);
    }

    *linesCount = count;
    return lines;
}

// Get data type and name from a string containing both
// NOTE: Useful to parse function parameters and struct fields
static void GetDataTypeAndName(const char *typeName, int typeNameLen, char *type, char *name)
{
    for (int k = typeNameLen; k > 0; k--)
    {
        if (typeName[k] == ' ' && typeName[k - 1] != ',')
        {
            // Function name starts at this point (and ret type finishes at this point)
            MemoryCopy(type, typeName, k);
            MemoryCopy(name, typeName + k + 1, typeNameLen - k - 1);
            break;
        }
        else if (typeName[k] == '*')
        {
            MemoryCopy(type, typeName, k + 1);
            MemoryCopy(name, typeName + k + 1, typeNameLen - k - 1);
            break;
        }
    }
}

// Get text length in bytes, check for \0 character
static unsigned int TextLength(const char *text)
{
    unsigned int length = 0;

    if (text != NULL) while (*text++) length++;

    return length;
}

// Custom memcpy() to avoid <string.h>
static void MemoryCopy(void *dest, const void *src, unsigned int count)
{
    char *srcPtr = (char *)src;
    char *destPtr = (char *)dest;

    for (unsigned int i = 0; i < count; i++) destPtr[i] = srcPtr[i];
}

// Compare two text strings, requires number of characters to compare
static bool IsTextEqual(const char *text1, const char *text2, unsigned int count)
{
    bool result = true;

    for (unsigned int i = 0; i < count; i++)
    {
        if (text1[i] != text2[i])
        {
            result = false;
            break;
        }
    }

    return result;
}

// Search and replace a character within a string.
static char* CharReplace(char* text, char search, char replace)
{
    for (int i = 0; text[i] != '\0'; i++)
        if (text[i] == search)
            text[i] = replace;
    return text;
}

/*
// Replace text string
// REQUIRES: strlen(), strstr(), strncpy(), strcpy() -> TODO: Replace by custom implementations!
// WARNING: Returned buffer must be freed by the user (if return != NULL)
static char *TextReplace(char *text, const char *replace, const char *by)
{
    // Sanity checks and initialization
    if (!text || !replace || !by) return NULL;

    char *result;

    char *insertPoint;      // Next insert point
    char *temp;             // Temp pointer
    int replaceLen;         // Replace string length of (the string to remove)
    int byLen;              // Replacement length (the string to replace replace by)
    int lastReplacePos;     // Distance between replace and end of last replace
    int count;              // Number of replacements

    replaceLen = strlen(replace);
    if (replaceLen == 0) return NULL;  // Empty replace causes infinite loop during count

    byLen = strlen(by);

    // Count the number of replacements needed
    insertPoint = text;
    for (count = 0; (temp = strstr(insertPoint, replace)); count++) insertPoint = temp + replaceLen;

    // Allocate returning string and point temp to it
    temp = result = (char *)malloc(strlen(text) + (byLen - replaceLen)*count + 1);

    if (!result) return NULL;   // Memory could not be allocated

    // First time through the loop, all the variable are set correctly from here on,
    //  - 'temp' points to the end of the result string
    //  - 'insertPoint' points to the next occurrence of replace in text
    //  - 'text' points to the remainder of text after "end of replace"
    while (count--)
    {
        insertPoint = strstr(text, replace);
        lastReplacePos = (int)(insertPoint - text);
        temp = strncpy(temp, text, lastReplacePos) + lastReplacePos;
        temp = strcpy(temp, by) + byLen;
        text += lastReplacePos + replaceLen; // Move to next "end of replace"
    }

    // Copy remaind text part after replacement to result (pointed by moving temp)
    strcpy(temp, text);

    return result;
}
*/

// Export parsed data in desired format
static void ExportParsedData(const char *fileName, int format)
{
    FILE *outFile = fopen(fileName, "wt");

    switch (format)
    {
        case DEFAULT:
        {
            // Print structs info
            fprintf(outFile, "\nStructures found: %i\n\n", structCount);
            for (int i = 0; i < structCount; i++)
            {
                fprintf(outFile, "Struct %02i: %s (%i fields)\n", i + 1, structs[i].name, structs[i].fieldCount);
                fprintf(outFile, "  Name: %s\n", structs[i].name);
                fprintf(outFile, "  Description: %s\n", structs[i].desc + 3);
                for (int f = 0; f < structs[i].fieldCount; f++) fprintf(outFile, "  Field[%i]: %s %s %s\n", f + 1, structs[i].fieldType[f], structs[i].fieldName[f], structs[i].fieldDesc[f]);
            }

            // Print enums info
            fprintf(outFile, "\nEnums found: %i\n\n", enumCount);
            for (int i = 0; i < enumCount; i++)
            {
                fprintf(outFile, "Enum %02i: %s (%i values)\n", i + 1, enums[i].name, enums[i].valueCount);
                fprintf(outFile, "  Name: %s\n", enums[i].name);
                fprintf(outFile, " Description: %s\n", enums[i].desc + 3);
                for (int e = 0; e < enums[i].valueCount; e++) fprintf(outFile, "  Value[%s]: %i\n", enums[i].valueName[e], enums[i].valueInteger[e]);
            }

            // Print functions info
            fprintf(outFile, "\nFunctions found: %i\n\n", funcCount);
            for (int i = 0; i < funcCount; i++)
            {
                fprintf(outFile, "Function %03i: %s() (%i input parameters)\n", i + 1, funcs[i].name, funcs[i].paramCount);
                fprintf(outFile, "  Name: %s\n", funcs[i].name);
                fprintf(outFile, "  Return type: %s\n", funcs[i].retType);
                fprintf(outFile, "  Description: %s\n", funcs[i].desc + 3);
                for (int p = 0; p < funcs[i].paramCount; p++) fprintf(outFile, "  Param[%i]: %s (type: %s)\n", p + 1, funcs[i].paramName[p], funcs[i].paramType[p]);
                if (funcs[i].paramCount == 0) fprintf(outFile, "  No input parameters\n");
            }
        } break;
        case JSON:
        {
            fprintf(outFile, "{\n");

            // Print structs info
            fprintf(outFile, "  \"structs\": [\n");
            for (int i = 0; i < structCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      \"name\": \"%s\",\n", structs[i].name);
                fprintf(outFile, "      \"description\": \"%s\",\n", structs[i].desc);
                fprintf(outFile, "      \"fields\": [\n");
                for (int f = 0; f < structs[i].fieldCount; f++)
                {
                    fprintf(outFile, "        {\n");
                    fprintf(outFile, "          \"name\": \"%s\",\n", structs[i].fieldName[f]),
                    fprintf(outFile, "          \"type\": \"%s\",\n", structs[i].fieldType[f]),
                    fprintf(outFile, "          \"description\": \"%s\"\n", structs[i].fieldDesc[f] + 3),
                    fprintf(outFile, "        }");
                    if (f < structs[i].fieldCount - 1) fprintf(outFile, ",\n");
                    else fprintf(outFile, "\n");
                }
                fprintf(outFile, "      ]\n");
                fprintf(outFile, "    }");
                if (i < structCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  ],\n");

            // Print enums info
            fprintf(outFile, "  \"enums\": [\n");
            for (int i = 0; i < enumCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      \"name\": \"%s\",\n", enums[i].name);
                fprintf(outFile, "      \"description\": \"%s\",\n", enums[i].desc + 3);
                fprintf(outFile, "      \"values\": [\n");
                for (int e = 0; e < enums[i].valueCount; e++)
                {
                    fprintf(outFile, "        {\n");
                    fprintf(outFile, "          \"name\": \"%s\",\n", enums[i].valueName[e]),
                    fprintf(outFile, "          \"value\": %i,\n", enums[i].valueInteger[e]),
                    fprintf(outFile, "          \"description\": \"%s\"\n", enums[i].valueDesc[e] + 3),
                    fprintf(outFile, "        }");
                    if (e < enums[i].valueCount - 1) fprintf(outFile, ",\n");
                    else fprintf(outFile, "\n");
                }
                fprintf(outFile, "      ]\n");
                fprintf(outFile, "    }");
                if (i < enumCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  ],\n");

            // Print functions info
            fprintf(outFile, "  \"functions\": [\n");
            for (int i = 0; i < funcCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      \"name\": \"%s\",\n", funcs[i].name);
                fprintf(outFile, "      \"description\": \"%s\",\n", CharReplace(funcs[i].desc, '\\', ' ') + 3);
                fprintf(outFile, "      \"returnType\": \"%s\"", funcs[i].retType);

                if (funcs[i].paramCount == 0) fprintf(outFile, "\n");
                else
                {
                    fprintf(outFile, ",\n      \"params\": {\n");
                    for (int p = 0; p < funcs[i].paramCount; p++)
                    {
                        fprintf(outFile, "        \"%s\": \"%s\"", funcs[i].paramName[p], funcs[i].paramType[p]);
                        if (p < funcs[i].paramCount - 1) fprintf(outFile, ",\n");
                        else fprintf(outFile, "\n");
                    }
                    fprintf(outFile, "      }\n");
                }
                fprintf(outFile, "    }");

                if (i < funcCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  ]\n");
            fprintf(outFile, "}\n");
        } break;
        case XML:
        {
            // XML format to export data:
            /*
            <?xml version="1.0" encoding="Windows-1252" ?>
            <raylibAPI>
                <Structs count="">
                    <Struct name="" fieldCount="" desc="">
                        <Field type="" name="" desc="">
                        <Field type="" name="" desc="">
                    </Struct>
                <Structs>
                <Enums count="">
                    <Enum name="" valueCount="" desc="">
                        <Value name="" integer="" desc="">
                        <Value name="" integer="" desc="">
                    </Enum>
                </Enums>
                <Functions count="">
                    <Function name="" retType="" paramCount="" desc="">
                        <Param type="" name="" desc="" />
                        <Param type="" name="" desc="" />
                    </Function>
                </Functions>
            </raylibAPI>
            */

            fprintf(outFile, "<?xml version=\"1.0\" encoding=\"Windows-1252\" ?>\n");
            fprintf(outFile, "<raylibAPI>\n");

            // Print structs info
            fprintf(outFile, "    <Structs count=\"%i\">\n", structCount);
            for (int i = 0; i < structCount; i++)
            {
                fprintf(outFile, "        <Struct name=\"%s\" fieldCount=\"%i\" desc=\"%s\">\n", structs[i].name, structs[i].fieldCount, structs[i].desc + 3);
                for (int f = 0; f < structs[i].fieldCount; f++)
                {
                    fprintf(outFile, "            <Field type=\"%s\" name=\"%s\" desc=\"%s\" />\n", structs[i].fieldType[f], structs[i].fieldName[f], structs[i].fieldDesc[f] + 3);
                }
                fprintf(outFile, "        </Struct>\n");
            }
            fprintf(outFile, "    </Structs>\n");

            // Print enums info
            fprintf(outFile, "    <Enums count=\"%i\">\n", enumCount);
            for (int i = 0; i < enumCount; i++)
            {
                fprintf(outFile, "        <Enum name=\"%s\" valueCount=\"%i\" desc=\"%s\">\n", enums[i].name, enums[i].valueCount, enums[i].desc + 3);
                for (int v = 0; v < enums[i].valueCount; v++)
                {
                    fprintf(outFile, "            <Value name=\"%s\" integer=\"%i\" desc=\"%s\" />\n", enums[i].valueName[v], enums[i].valueInteger[v], enums[i].valueDesc[v] + 3);
                }
                fprintf(outFile, "        </Enum>\n");
            }
            fprintf(outFile, "    </Enums>\n");

            // Print functions info
            fprintf(outFile, "    <Functions count=\"%i\">\n", funcCount);
            for (int i = 0; i < funcCount; i++)
            {
                fprintf(outFile, "        <Function name=\"%s\" retType=\"%s\" paramCount=\"%i\" desc=\"%s\">\n", funcs[i].name, funcs[i].retType, funcs[i].paramCount, funcs[i].desc + 3);
                for (int p = 0; p < funcs[i].paramCount; p++)
                {
                    fprintf(outFile, "            <Param type=\"%s\" name=\"%s\" desc=\"%s\" />\n", funcs[i].paramType[p], funcs[i].paramName[p], funcs[i].paramDesc[p] + 3);
                }
                fprintf(outFile, "        </Function>\n");
            }
            fprintf(outFile, "    </Functions>\n");

            fprintf(outFile, "</raylibAPI>\n");

        } break;
        default: break;
    }

    fclose(outFile);
}
