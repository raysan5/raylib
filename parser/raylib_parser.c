/**********************************************************************************************

    raylib API parser

    This parser scans raylib.h to get API information about defines, structs, aliases, enums, callbacks and functions.
    All data is divided into pieces, usually as strings. The following types are used for data:

     - struct DefineInfo
     - struct StructInfo
     - struct AliasInfo
     - struct EnumInfo
     - struct FunctionInfo

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
#include <ctype.h>              // Required for: isdigit()

#define MAX_DEFINES_TO_PARSE    2048    // Maximum number of defines to parse
#define MAX_STRUCTS_TO_PARSE      64    // Maximum number of structures to parse
#define MAX_ALIASES_TO_PARSE      64    // Maximum number of aliases to parse
#define MAX_ENUMS_TO_PARSE        64    // Maximum number of enums to parse
#define MAX_CALLBACKS_TO_PARSE    64    // Maximum number of callbacks to parse
#define MAX_FUNCS_TO_PARSE       512    // Maximum number of functions to parse

#define MAX_LINE_LENGTH          512    // Maximum length of one line (including comments)

#define MAX_STRUCT_FIELDS         64    // Maximum number of struct fields
#define MAX_ENUM_VALUES          512    // Maximum number of enum values
#define MAX_FUNCTION_PARAMETERS   12    // Maximum number of function parameters

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Type of parsed define
typedef enum { UNKNOWN = 0, MACRO, GUARD, INT, LONG, FLOAT, DOUBLE, CHAR, STRING, COLOR } DefineType;

// Define info data
typedef struct DefineInfo {
    char name[64];    // Define name
    DefineType type;  // Define type
    char value[256];  // Define value
    char desc[128];   // Define description
    bool isHex;       // Define is hex number (for types INT, LONG)
} DefineInfo;

// Struct info data
typedef struct StructInfo {
    char name[64];              // Struct name
    char desc[128];             // Struct type description
    int fieldCount;             // Number of fields in the struct
    char fieldType[MAX_STRUCT_FIELDS][64];     // Field type
    char fieldName[MAX_STRUCT_FIELDS][64];     // Field name
    char fieldDesc[MAX_STRUCT_FIELDS][128];    // Field description
} StructInfo;

// Alias info data
typedef struct AliasInfo {
    char type[64];              // Alias type
    char name[64];              // Alias name
    char desc[128];             // Alias description
} AliasInfo;

// Enum info data
typedef struct EnumInfo {
    char name[64];              // Enum name
    char desc[128];             // Enum description
    int valueCount;             // Number of values in enumerator
    char valueName[MAX_ENUM_VALUES][64];    // Value name definition
    int valueInteger[MAX_ENUM_VALUES];      // Value integer
    char valueDesc[MAX_ENUM_VALUES][128];   // Value description
} EnumInfo;

// Function info data
typedef struct FunctionInfo {
    char name[64];              // Function name
    char desc[128];             // Function description (comment at the end)
    char retType[32];           // Return value type
    int paramCount;             // Number of function parameters
    char paramType[MAX_FUNCTION_PARAMETERS][32];   // Parameters type
    char paramName[MAX_FUNCTION_PARAMETERS][32];   // Parameters name
    char paramDesc[MAX_FUNCTION_PARAMETERS][128];  // Parameters description
} FunctionInfo;

// Output format for parsed data
typedef enum { DEFAULT = 0, JSON, XML, LUA } OutputFormat;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static int defineCount = 0;
static int structCount = 0;
static int aliasCount = 0;
static int enumCount = 0;
static int callbackCount = 0;
static int funcCount = 0;
static DefineInfo *defines = NULL;
static StructInfo *structs = NULL;
static AliasInfo *aliases = NULL;
static EnumInfo *enums = NULL;
static FunctionInfo *callbacks = NULL;
static FunctionInfo *funcs = NULL;

// Command line variables
static char apiDefine[32] = { 0 };         // Functions define (i.e. RLAPI for raylib.h, RMDEF for raymath.h, etc.)
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
static void GetDescription(const char *source, char *description);
static void MoveArraySize(char *name, char *type);          // Move array size from name to type
static unsigned int TextLength(const char *text);           // Get text length in bytes, check for \0 character
static bool IsTextEqual(const char *text1, const char *text2, unsigned int count);
static void MemoryCopy(void *dest, const void *src, unsigned int count);
static char *EscapeBackslashes(char *text);                 // Replace '\' by "\\" when exporting to JSON and XML
static const char *StrDefineType(DefineType type);          // Get string of define type

static void ExportParsedData(const char *fileName, int format); // Export parsed data in desired format

//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    if (argc > 1) ProcessCommandLine(argc, argv);

    if (inFileName[0] == '\0') MemoryCopy(inFileName, "../src/raylib.h\0", 16);
    if (outFileName[0] == '\0') MemoryCopy(outFileName, "raylib_api.txt\0", 15);
    if (apiDefine[0] == '\0') MemoryCopy(apiDefine, "RLAPI\0", 6);

    int length = 0;
    char *buffer = LoadFileText(inFileName, &length);

    // Preprocess buffer to get separate lines
    // NOTE: GetTextLines() also removes leading spaces/tabs
    int linesCount = 0;
    char **lines = GetTextLines(buffer, length, &linesCount);

    // Defines line indices
    int *defineLines = (int *)malloc(MAX_DEFINES_TO_PARSE*sizeof(int));

    // Structs line indices
    int *structLines = (int *)malloc(MAX_STRUCTS_TO_PARSE*sizeof(int));

    // Aliases line indices
    int *aliasLines = (int *)malloc(MAX_ALIASES_TO_PARSE*sizeof(int));

    // Enums line indices
    int *enumLines = (int *)malloc(MAX_ENUMS_TO_PARSE*sizeof(int));

    // Callbacks line indices
    int *callbackLines = (int *)malloc(MAX_CALLBACKS_TO_PARSE*sizeof(int));

    // Function line indices
    int *funcLines = (int *)malloc(MAX_FUNCS_TO_PARSE*sizeof(int));

    // Prepare required lines for parsing
    //----------------------------------------------------------------------------------

    // Read define lines
    for (int i = 0; i < linesCount; i++)
    {
        int j = 0;
        while ((lines[i][j] == ' ') || (lines[i][j] == '\t')) j++; // skip spaces and tabs in the begining
        // Read define line
        if (IsTextEqual(lines[i]+j, "#define ", 8))
        {
            // Keep the line position in the array of lines,
            // so, we can scan that position and following lines
            defineLines[defineCount] = i;
            defineCount++;
        }
    }

    // Read struct lines
    for (int i = 0; i < linesCount; i++)
    {
        // Find structs
        // starting with "typedef struct ... {" or "typedef struct ... ; \n struct ... {"
        // ending with "} ... ;"
        // i.e. excluding "typedef struct rAudioBuffer rAudioBuffer;" -> Typedef and forward declaration only
        if (IsTextEqual(lines[i], "typedef struct", 14))
        {
            bool validStruct = IsTextEqual(lines[i + 1], "struct", 6);
            if (!validStruct)
            {
                for (int c = 0; c < MAX_LINE_LENGTH; c++)
                {
                    char v = lines[i][c];
                    if (v == '{') validStruct = true;
                    if ((v == '{') || (v == ';') || (v == '\0')) break;
                }
            }
            if (!validStruct) continue;
            structLines[structCount] = i;
            while (lines[i][0] != '}') i++;
            while (lines[i][0] != '\0') i++;
            structCount++;
        }
    }

    // Read alias lines
    for (int i = 0; i < linesCount; i++)
    {
        // Find aliases (lines with "typedef ... ...;")
        if (IsTextEqual(lines[i], "typedef", 7))
        {
            int spaceCount = 0;
            bool validAlias = false;

            for (int c = 0; c < MAX_LINE_LENGTH; c++)
            {
                char v = lines[i][c];
                if (v == ' ') spaceCount++;
                if ((v == ';') && (spaceCount == 2)) validAlias = true;
                if ((v == ';') || (v == '(') || (v == '\0')) break;
            }
            if (!validAlias) continue;
            aliasLines[aliasCount] = i;
            aliasCount++;
        }
    }

    // Read enum lines
    for (int i = 0; i < linesCount; i++)
    {
        // Read enum line
        if (IsTextEqual(lines[i], "typedef enum {", 14) && (lines[i][TextLength(lines[i])-1] != ';')) // ignore inline enums
        {
            // Keep the line position in the array of lines,
            // so, we can scan that position and following lines
            enumLines[enumCount] = i;
            enumCount++;
        }
    }

    // Read callback lines
    for (int i = 0; i < linesCount; i++)
    {
        // Find callbacks (lines with "typedef ... (* ... )( ... );")
        if (IsTextEqual(lines[i], "typedef", 7))
        {
            bool hasBeginning = false;
            bool hasMiddle = false;
            bool hasEnd = false;

            for (int c = 0; c < MAX_LINE_LENGTH; c++)
            {
                if ((lines[i][c] == '(') && (lines[i][c + 1] == '*')) hasBeginning = true;
                if ((lines[i][c] == ')') && (lines[i][c + 1] == '(')) hasMiddle = true;
                if ((lines[i][c] == ')') && (lines[i][c + 1] == ';')) hasEnd = true;
                if (hasEnd) break;
            }

            if (hasBeginning && hasMiddle && hasEnd)
            {
                callbackLines[callbackCount] = i;
                callbackCount++;
            }
        }
    }

    // Read function lines
    for (int i = 0; i < linesCount; i++)
    {
        // Read function line (starting with `define`, i.e. for raylib.h "RLAPI")
        if (IsTextEqual(lines[i], apiDefine, TextLength(apiDefine)))
        {
            funcLines[funcCount] = i;
            funcCount++;
        }
    }

    // At this point we have all raylib defines, structs, aliases, enums, callbacks, functions lines data to start parsing

    free(buffer);       // Unload text buffer

    // Parsing raylib data
    //----------------------------------------------------------------------------------

    // Define info data
    defines = (DefineInfo *)calloc(MAX_DEFINES_TO_PARSE, sizeof(DefineInfo));
    int defineIndex = 0;

    for (int i = 0; i < defineCount; i++)
    {
        char *linePtr = lines[defineLines[i]];
        int j = 0;

        while ((linePtr[j] == ' ') || (linePtr[j] == '\t')) j++; // Skip spaces and tabs in the begining
        j += 8;                                                  // Skip "#define "
        while ((linePtr[j] == ' ') || (linePtr[j] == '\t')) j++; // Skip spaces and tabs after "#define "

        // Extract name
        int defineNameStart = j;
        int openBraces = 0;
        while (linePtr[j] != '\0')
        {
            if (((linePtr[j] == ' ') || (linePtr[j] == '\t')) && (openBraces == 0)) break;
            if (linePtr[j] == '(') openBraces++;
            if (linePtr[j] == ')') openBraces--;
            j++;
        }
        int defineNameEnd = j-1;

        // Skip duplicates
        int nameLen = defineNameEnd - defineNameStart + 1;
        bool isDuplicate = false;
        for (int k = 0; k < defineIndex; k++)
        {
            if ((nameLen == TextLength(defines[k].name)) && IsTextEqual(defines[k].name, &linePtr[defineNameStart], nameLen))
            {
                isDuplicate = true;
                break;
            }
        }
        if (isDuplicate) continue;

        MemoryCopy(defines[defineIndex].name, &linePtr[defineNameStart], nameLen);

        // Determine type
        if (linePtr[defineNameEnd] == ')') defines[defineIndex].type = MACRO;

        while ((linePtr[j] == ' ') || (linePtr[j] == '\t')) j++; // Skip spaces and tabs after name

        int defineValueStart = j;
        if ((linePtr[j] == '\0') || (linePtr == "/")) defines[defineIndex].type = GUARD;
        if (linePtr[j] == '"') defines[defineIndex].type = STRING;
        else if (linePtr[j] == '\'') defines[defineIndex].type = CHAR;
        else if (IsTextEqual(linePtr+j, "CLITERAL(Color)", 15)) defines[defineIndex].type = COLOR;
        else if (isdigit(linePtr[j])) // Parsing numbers
        {
            bool isFloat = false, isNumber = true, isHex = false;
            while ((linePtr[j] != ' ') && (linePtr[j] != '\t') && (linePtr[j] != '\0'))
            {
                char ch = linePtr[j];
                if (ch == '.') isFloat = true;
                if (ch == 'x') isHex = true;
                if (!(isdigit(ch) ||
                      ((ch >= 'a') && (ch <= 'f')) ||
                      ((ch >= 'A') && (ch <= 'F')) ||
                      (ch == 'x') ||
                      (ch == 'L') ||
                      (ch == '.') ||
                      (ch == '+') ||
                      (ch == '-'))) isNumber = false;
                j++;
            }
            if (isNumber)
            {
                if (isFloat)
                {
                    defines[defineIndex].type = linePtr[j-1] == 'f' ? FLOAT : DOUBLE;
                }
                else
                {
                    defines[defineIndex].type = linePtr[j-1] == 'L' ? LONG : INT;
                    defines[defineIndex].isHex = isHex;
                }
            }
        }

        // Extracting value
        while ((linePtr[j] != '\\') && (linePtr[j] != '\0') && !((linePtr[j] == '/') && (linePtr[j+1] == '/'))) j++;
        int defineValueEnd = j-1;
        while ((linePtr[defineValueEnd] == ' ') || (linePtr[defineValueEnd] == '\t')) defineValueEnd--; // Remove trailing spaces and tabs
        if ((defines[defineIndex].type == LONG) || (defines[defineIndex].type == FLOAT)) defineValueEnd--; // Remove number postfix
        int valueLen = defineValueEnd - defineValueStart + 1;
        if (valueLen > 255) valueLen = 255;

        if (valueLen > 0) MemoryCopy(defines[defineIndex].value, &linePtr[defineValueStart], valueLen);

        // Extracting description
        if ((linePtr[j] == '/') && linePtr[j + 1] == '/')
        {
            j += 2;
            while (linePtr[j] == ' ') j++;
            int commentStart = j;
            while ((linePtr[j] != '\\') && (linePtr[j] != '\0')) j++;
            int commentEnd = j-1;
            int commentLen = commentEnd - commentStart + 1;
            if (commentLen > 127) commentLen = 127;

            MemoryCopy(defines[defineIndex].desc, &linePtr[commentStart], commentLen);
        }

        defineIndex++;
    }
    defineCount = defineIndex;
    free(defineLines);

    // Structs info data
    structs = (StructInfo *)calloc(MAX_STRUCTS_TO_PARSE, sizeof(StructInfo));

    for (int i = 0; i < structCount; i++)
    {
        char **linesPtr = &lines[structLines[i]];

        // Parse struct description
        GetDescription(linesPtr[-1], structs[i].desc);

        // Get struct name: typedef struct name {
        const int TDS_LEN = 15; // length of "typedef struct "
        for (int c = TDS_LEN; c < 64 + TDS_LEN; c++)
        {
            if ((linesPtr[0][c] == '{') || (linesPtr[0][c] == ' '))
            {
                int nameLen = c - TDS_LEN;
                while (linesPtr[0][TDS_LEN + nameLen - 1] == ' ') nameLen--;
                MemoryCopy(structs[i].name, &linesPtr[0][TDS_LEN], nameLen);
                break;
            }
        }

        // Get struct fields and count them -> fields finish with ;
        int l = 1;
        while (linesPtr[l][0] != '}')
        {
            // WARNING: Some structs have empty spaces and comments -> OK, processed
            if ((linesPtr[l][0] != ' ') && (linesPtr[l][0] != '\0'))
            {
                // Scan one field line
                char *fieldLine = linesPtr[l];
                int fieldEndPos = 0;
                while (fieldLine[fieldEndPos] != ';') fieldEndPos++;

                if ((fieldLine[0] != '/') && !IsTextEqual(fieldLine, "struct", 6)) // Field line is not a comment and not a struct declaration
                {
                    //printf("Struct field: %s_\n", fieldLine);     // OK!

                    // Get struct field type and name
                    GetDataTypeAndName(fieldLine, fieldEndPos, structs[i].fieldType[structs[i].fieldCount], structs[i].fieldName[structs[i].fieldCount]);

                    // Get the field description
                    GetDescription(&fieldLine[fieldEndPos], structs[i].fieldDesc[structs[i].fieldCount]);

                    structs[i].fieldCount++;

                    // Split field names containing multiple fields (like Matrix)
                    int additionalFields = 0;
                    int originalIndex = structs[i].fieldCount - 1;
                    for (int c = 0; c < TextLength(structs[i].fieldName[originalIndex]); c++)
                    {
                        if (structs[i].fieldName[originalIndex][c] == ',') additionalFields++;
                    }
                    if (additionalFields > 0)
                    {
                        int originalLength = -1;
                        int lastStart;
                        for (int c = 0; c < TextLength(structs[i].fieldName[originalIndex]) + 1; c++)
                        {
                            char v = structs[i].fieldName[originalIndex][c];
                            bool isEndOfString = (v == '\0');
                            if ((v == ',') || isEndOfString)
                            {
                                if (originalLength == -1)
                                {
                                    // Save length of original field name
                                    // Don't truncate yet, still needed for copying
                                    originalLength = c;
                                }
                                else
                                {
                                    // Copy field data from original field
                                    int nameLength = c - lastStart;
                                    MemoryCopy(structs[i].fieldName[structs[i].fieldCount], &structs[i].fieldName[originalIndex][lastStart], nameLength);
                                    MemoryCopy(structs[i].fieldType[structs[i].fieldCount], &structs[i].fieldType[originalIndex][0], TextLength(structs[i].fieldType[originalIndex]));
                                    MemoryCopy(structs[i].fieldDesc[structs[i].fieldCount], &structs[i].fieldDesc[originalIndex][0], TextLength(structs[i].fieldDesc[originalIndex]));
                                    structs[i].fieldCount++;
                                }
                                if (!isEndOfString)
                                {
                                    // Skip comma and spaces
                                    c++;
                                    while (structs[i].fieldName[originalIndex][c] == ' ') c++;

                                    // Save position for next field
                                    lastStart = c;
                                }
                            }
                        }
                        // Set length of original field to truncate the first field name
                        structs[i].fieldName[originalIndex][originalLength] = '\0';
                    }

                    // Split field types containing multiple fields (like MemNode)
                    additionalFields = 0;
                    originalIndex = structs[i].fieldCount - 1;
                    for (int c = 0; c < TextLength(structs[i].fieldType[originalIndex]); c++)
                    {
                        if (structs[i].fieldType[originalIndex][c] == ',') additionalFields++;
                    }
                    if (additionalFields > 0) {
                        // Copy original name to last additional field
                        structs[i].fieldCount += additionalFields;
                        MemoryCopy(structs[i].fieldName[originalIndex + additionalFields], &structs[i].fieldName[originalIndex][0], TextLength(structs[i].fieldName[originalIndex]));

                        // Copy names from type to additional fields
                        int fieldsRemaining = additionalFields;
                        int nameStart = -1;
                        int nameEnd = -1;
                        for (int k = TextLength(structs[i].fieldType[originalIndex]); k > 0; k--)
                        {
                            char v = structs[i].fieldType[originalIndex][k];
                            if ((v == '*') || (v == ' ') || (v == ','))
                            {
                                if (nameEnd != -1) {
                                    // Don't copy to last additional field
                                    if (fieldsRemaining != additionalFields)
                                    {
                                        nameStart = k + 1;
                                        MemoryCopy(structs[i].fieldName[originalIndex + fieldsRemaining], &structs[i].fieldType[originalIndex][nameStart], nameEnd - nameStart + 1);
                                    }
                                    nameEnd = -1;
                                    fieldsRemaining--;
                                }
                            }
                            else if (nameEnd == -1) nameEnd = k;
                        }

                        // Truncate original field type
                        int fieldTypeLength = nameStart;
                        structs[i].fieldType[originalIndex][fieldTypeLength] = '\0';

                        // Set field type and description of additional fields
                        for (int j = 1; j <= additionalFields; j++)
                        {
                            MemoryCopy(structs[i].fieldType[originalIndex + j], &structs[i].fieldType[originalIndex][0], fieldTypeLength);
                            MemoryCopy(structs[i].fieldDesc[originalIndex + j], &structs[i].fieldDesc[originalIndex][0], TextLength(structs[i].fieldDesc[originalIndex]));
                        }
                    }
                }
            }

            l++;
        }

        // Move array sizes from name to type
        for (int j = 0; j < structs[i].fieldCount; j++)
        {
            MoveArraySize(structs[i].fieldName[j], structs[i].fieldType[j]);
        }
    }
    free(structLines);

    // Alias info data
    aliases = (AliasInfo *)calloc(MAX_ALIASES_TO_PARSE, sizeof(AliasInfo));

    for (int i = 0; i < aliasCount; i++)
    {
        // Description from previous line
        GetDescription(lines[aliasLines[i] - 1], aliases[i].desc);

        char *linePtr = lines[aliasLines[i]];

        // Skip "typedef "
        int c = 8;

        // Type
        int typeStart = c;
        while(linePtr[c] != ' ') c++;
        int typeLen = c - typeStart;
        MemoryCopy(aliases[i].type, &linePtr[typeStart], typeLen);

        // Skip space
        c++;

        // Name
        int nameStart = c;
        while(linePtr[c] != ';') c++;
        int nameLen = c - nameStart;
        MemoryCopy(aliases[i].name, &linePtr[nameStart], nameLen);

        // Description
        GetDescription(&linePtr[c], aliases[i].desc);
    }
    free(aliasLines);

    // Enum info data
    enums = (EnumInfo *)calloc(MAX_ENUMS_TO_PARSE, sizeof(EnumInfo));

    for (int i = 0; i < enumCount; i++)
    {

        // Parse enum description
        // NOTE: This is not necessarily from the line immediately before,
        // some of the enums have extra lines between the "description"
        // and the typedef enum
        for (int j = enumLines[i] - 1; j > 0; j--)
        {
            char *linePtr = lines[j];
            if ((linePtr[0] != '/') || (linePtr[2] != ' '))
            {
                GetDescription(&lines[j + 1][0], enums[i].desc);
                break;
            }
        }

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
                       (linePtr[c] != '\0'))
                {
                    enums[i].valueName[enums[i].valueCount][c] = linePtr[c];
                    c++;
                }

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
                    while ((linePtr[c] != '\0') && (linePtr[c] != '/'))
                    {
                        if (linePtr[c] == '=')
                        {
                            foundValue = true;
                            break;
                        }
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
                }
                else enums[i].valueInteger[enums[i].valueCount] = (enums[i].valueInteger[enums[i].valueCount - 1] + 1);

                // Parse value description
                GetDescription(&linePtr[c], enums[i].valueDesc[enums[i].valueCount]);

                enums[i].valueCount++;
            }
            else if (linePtr[0] == '}')
            {
                // Get enum name from typedef
                int c = 0;
                while (linePtr[2 + c] != ';')
                {
                    enums[i].name[c] = linePtr[2 + c];
                    c++;
                }

                break;  // Enum ended, break for() loop
            }
        }
    }
    free(enumLines);

    // Callback info data
    callbacks = (FunctionInfo *)calloc(MAX_CALLBACKS_TO_PARSE, sizeof(FunctionInfo));

    for (int i = 0; i < callbackCount; i++)
    {
        char *linePtr = lines[callbackLines[i]];

        // Skip "typedef "
        int c = 8;

        // Return type
        int retTypeStart = c;
        while(linePtr[c] != '(') c++;
        int retTypeLen = c - retTypeStart;
        while(linePtr[retTypeStart + retTypeLen - 1] == ' ') retTypeLen--;
        MemoryCopy(callbacks[i].retType, &linePtr[retTypeStart], retTypeLen);

        // Skip "(*"
        c += 2;

        // Name
        int nameStart = c;
        while(linePtr[c] != ')') c++;
        int nameLen = c - nameStart;
        MemoryCopy(callbacks[i].name, &linePtr[nameStart], nameLen);

        // Skip ")("
        c += 2;

        // Params
        int paramStart = c;
        for (c; c < MAX_LINE_LENGTH; c++)
        {
            if ((linePtr[c] == ',') || (linePtr[c] == ')'))
            {
                // Get parameter type + name, extract info
                int paramLen = c - paramStart;
                GetDataTypeAndName(&linePtr[paramStart], paramLen, callbacks[i].paramType[callbacks[i].paramCount], callbacks[i].paramName[callbacks[i].paramCount]);
                callbacks[i].paramCount++;
                paramStart = c + 1;
                while(linePtr[paramStart] == ' ') paramStart++;
            }
            if (linePtr[c] == ')') break;
        }

        // Description
        GetDescription(&linePtr[c], callbacks[i].desc);

        // Move array sizes from name to type
        for (int j = 0; j < callbacks[i].paramCount; j++)
        {
            MoveArraySize(callbacks[i].paramName[j], callbacks[i].paramType[j]);
        }
    }
    free(callbackLines);

    // Functions info data
    funcs = (FunctionInfo *)calloc(MAX_FUNCS_TO_PARSE, sizeof(FunctionInfo));

    for (int i = 0; i < funcCount; i++)
    {
        char *linePtr = lines[funcLines[i]];

        int funcParamsStart = 0;
        int funcEnd = 0;

        // Get return type and function name from func line
        for (int c = 0; (c < MAX_LINE_LENGTH) && (linePtr[c] != '\n'); c++)
        {
            if (linePtr[c] == '(')     // Starts function parameters
            {
                funcParamsStart = c + 1;

                // At this point we have function return type and function name
                char funcRetTypeName[128] = { 0 };
                int dc = TextLength(apiDefine) + 1;
                int funcRetTypeNameLen = c - dc;     // Substract `define` ("RLAPI " for raylib.h)
                MemoryCopy(funcRetTypeName, &linePtr[dc], funcRetTypeNameLen);

                GetDataTypeAndName(funcRetTypeName, funcRetTypeNameLen, funcs[i].retType, funcs[i].name);
                break;
            }
        }

        // Get parameters from func line
        for (int c = funcParamsStart; c < MAX_LINE_LENGTH; c++)
        {
            if (linePtr[c] == ',')     // Starts function parameters
            {
                // Get parameter type + name, extract info
                char funcParamTypeName[128] = { 0 };
                int funcParamTypeNameLen = c - funcParamsStart;
                MemoryCopy(funcParamTypeName, &linePtr[funcParamsStart], funcParamTypeNameLen);

                GetDataTypeAndName(funcParamTypeName, funcParamTypeNameLen, funcs[i].paramType[funcs[i].paramCount], funcs[i].paramName[funcs[i].paramCount]);

                funcParamsStart = c + 1;
                if (linePtr[c + 1] == ' ') funcParamsStart += 1;
                funcs[i].paramCount++;      // Move to next parameter
            }
            else if (linePtr[c] == ')')
            {
                funcEnd = c + 2;

                // Check if previous word is void
                if ((linePtr[c - 4] == 'v') && (linePtr[c - 3] == 'o') && (linePtr[c - 2] == 'i') && (linePtr[c - 1] == 'd')) break;

                // Get parameter type + name, extract info
                char funcParamTypeName[128] = { 0 };
                int funcParamTypeNameLen = c - funcParamsStart;
                MemoryCopy(funcParamTypeName, &linePtr[funcParamsStart], funcParamTypeNameLen);

                GetDataTypeAndName(funcParamTypeName, funcParamTypeNameLen, funcs[i].paramType[funcs[i].paramCount], funcs[i].paramName[funcs[i].paramCount]);

                funcs[i].paramCount++;      // Move to next parameter
                break;
            }
        }

        // Get function description
        GetDescription(&linePtr[funcEnd], funcs[i].desc);

        // Move array sizes from name to type
        for (int j = 0; j < funcs[i].paramCount; j++)
        {
            MoveArraySize(funcs[i].paramName[j], funcs[i].paramType[j]);
        }
    }
    free(funcLines);

    for (int i = 0; i < linesCount; i++) free(lines[i]);
    free(lines);

    // At this point, all raylib data has been parsed!
    //----------------------------------------------------------------------------------
    // defines[]   -> We have all the defines decomposed into pieces for further analysis
    // structs[]   -> We have all the structs decomposed into pieces for further analysis
    // aliases[]   -> We have all the aliases decomposed into pieces for further analysis
    // enums[]     -> We have all the enums decomposed into pieces for further analysis
    // callbacks[] -> We have all the callbacks decomposed into pieces for further analysis
    // funcs[]     -> We have all the functions decomposed into pieces for further analysis

    printf("\nInput file:       %s", inFileName);
    printf("\nOutput file:      %s", outFileName);
    if (outputFormat == DEFAULT) printf("\nOutput format:    DEFAULT\n\n");
    else if (outputFormat == JSON) printf("\nOutput format:    JSON\n\n");
    else if (outputFormat == XML) printf("\nOutput format:    XML\n\n");
    else if (outputFormat == LUA) printf("\nOutput format:    LUA\n\n");

    ExportParsedData(outFileName, outputFormat);

    free(defines);
    free(structs);
    free(aliases);
    free(enums);
    free(callbacks);
    free(funcs);
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
    printf("                                      Supported types: DEFAULT, JSON, XML, LUA\n\n");
    printf("    -d, --define <DEF>              : Define functions define (i.e. RLAPI for raylib.h, RMDEF for raymath.h, etc.)\n");
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
            exit(0);
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
                else if (IsTextEqual(argv[i + 1], "LUA\0", 4)) outputFormat = LUA;
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
        if ((typeName[k] == ' ') && (typeName[k - 1] != ','))
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
        else if ((typeName[k] == '.') && (typeNameLen == 3)) // Handle varargs ...);
        {
            MemoryCopy(type, "...", 3);
            MemoryCopy(name, "args", 4);
            break;
        }
    }
}

// Get comment from a line, do nothing if no comment in line
static void GetDescription(const char *line, char *description)
{
    int c = 0;
    int descStart = -1;
    int lastSlash = -2;
    bool isValid = false;
    while (line[c] != '\0')
    {
        if (isValid && (descStart == -1) && (line[c] != ' ')) descStart = c;
        else if (line[c] == '/')
        {
            if (lastSlash == c - 1) isValid = true;
            lastSlash = c;
        }
        c++;
    }
    if (descStart != -1) MemoryCopy(description, &line[descStart], c - descStart);
}

// Move array size from name to type
static void MoveArraySize(char *name, char *type)
{
    int nameLength = TextLength(name);
    if (name[nameLength - 1] == ']')
    {
        for (int k = nameLength; k > 0; k--)
        {
            if (name[k] == '[')
            {
                int sizeLength = nameLength - k;
                MemoryCopy(&type[TextLength(type)], &name[k], sizeLength);
                name[k] = '\0';
            }
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

// Custom memcpy() to avoid <string.h>
static void MemoryCopy(void *dest, const void *src, unsigned int count)
{
    char *srcPtr = (char *)src;
    char *destPtr = (char *)dest;

    for (unsigned int i = 0; i < count; i++) destPtr[i] = srcPtr[i];
}

// Escape backslashes in a string, writing the escaped string into a static buffer
static char *EscapeBackslashes(char *text)
{
    static char buffer[256] = { 0 };

    int count = 0;

    for (int i = 0; (text[i] != '\0') && (i < 255); i++, count++)
    {
        buffer[count] = text[i];

        if (text[i] == '\\')
        {
            buffer[count + 1] = '\\';
            count++;
        }
    }

    buffer[count] = '\0';

    return buffer;
}

// Get string of define type
static const char *StrDefineType(DefineType type)
{
    switch (type)
    {
        case UNKNOWN: return "UNKNOWN";
        case GUARD:   return "GUARD";
        case MACRO:   return "MACRO";
        case INT:     return "INT";
        case LONG:    return "LONG";
        case FLOAT:   return "FLOAT";
        case DOUBLE:  return "DOUBLE";
        case CHAR:    return "CHAR";
        case STRING:  return "STRING";
        case COLOR:   return "COLOR";
    }
    return "";
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
            // Print defines info
            fprintf(outFile, "\nDefines found: %i\n\n", defineCount);
            for (int i = 0; i < defineCount; i++)
            {
                fprintf(outFile, "Define %03i: %s\n", i + 1, defines[i].name);
                fprintf(outFile, "  Name: %s\n", defines[i].name);
                fprintf(outFile, "  Type: %s\n", StrDefineType(defines[i].type));
                fprintf(outFile, "  Value: %s\n", defines[i].value);
                fprintf(outFile, "  Description: %s\n", defines[i].desc);
            }

            // Print structs info
            fprintf(outFile, "\nStructures found: %i\n\n", structCount);
            for (int i = 0; i < structCount; i++)
            {
                fprintf(outFile, "Struct %02i: %s (%i fields)\n", i + 1, structs[i].name, structs[i].fieldCount);
                fprintf(outFile, "  Name: %s\n", structs[i].name);
                fprintf(outFile, "  Description: %s\n", structs[i].desc);
                for (int f = 0; f < structs[i].fieldCount; f++)
                {
                    fprintf(outFile, "  Field[%i]: %s %s ", f + 1, structs[i].fieldType[f], structs[i].fieldName[f]);
                    if (structs[i].fieldDesc[f][0]) fprintf(outFile, "// %s\n", structs[i].fieldDesc[f]);
                    else fprintf(outFile, "\n");
                }
            }

            // Print aliases info
            fprintf(outFile, "\nAliases found: %i\n\n", aliasCount);
            for (int i = 0; i < aliasCount; i++)
            {
                fprintf(outFile, "Alias %03i: %s\n", i + 1, aliases[i].name);
                fprintf(outFile, "  Type: %s\n", aliases[i].type);
                fprintf(outFile, "  Name: %s\n", aliases[i].name);
                fprintf(outFile, "  Description: %s\n", aliases[i].desc);
            }

            // Print enums info
            fprintf(outFile, "\nEnums found: %i\n\n", enumCount);
            for (int i = 0; i < enumCount; i++)
            {
                fprintf(outFile, "Enum %02i: %s (%i values)\n", i + 1, enums[i].name, enums[i].valueCount);
                fprintf(outFile, "  Name: %s\n", enums[i].name);
                fprintf(outFile, "  Description: %s\n", enums[i].desc);
                for (int e = 0; e < enums[i].valueCount; e++) fprintf(outFile, "  Value[%s]: %i\n", enums[i].valueName[e], enums[i].valueInteger[e]);
            }

            // Print callbacks info
            fprintf(outFile, "\nCallbacks found: %i\n\n", callbackCount);
            for (int i = 0; i < callbackCount; i++)
            {
                fprintf(outFile, "Callback %03i: %s() (%i input parameters)\n", i + 1, callbacks[i].name, callbacks[i].paramCount);
                fprintf(outFile, "  Name: %s\n", callbacks[i].name);
                fprintf(outFile, "  Return type: %s\n", callbacks[i].retType);
                fprintf(outFile, "  Description: %s\n", callbacks[i].desc);
                for (int p = 0; p < callbacks[i].paramCount; p++) fprintf(outFile, "  Param[%i]: %s (type: %s)\n", p + 1, callbacks[i].paramName[p], callbacks[i].paramType[p]);
                if (callbacks[i].paramCount == 0) fprintf(outFile, "  No input parameters\n");
            }

            // Print functions info
            fprintf(outFile, "\nFunctions found: %i\n\n", funcCount);
            for (int i = 0; i < funcCount; i++)
            {
                fprintf(outFile, "Function %03i: %s() (%i input parameters)\n", i + 1, funcs[i].name, funcs[i].paramCount);
                fprintf(outFile, "  Name: %s\n", funcs[i].name);
                fprintf(outFile, "  Return type: %s\n", funcs[i].retType);
                fprintf(outFile, "  Description: %s\n", funcs[i].desc);
                for (int p = 0; p < funcs[i].paramCount; p++) fprintf(outFile, "  Param[%i]: %s (type: %s)\n", p + 1, funcs[i].paramName[p], funcs[i].paramType[p]);
                if (funcs[i].paramCount == 0) fprintf(outFile, "  No input parameters\n");
            }
        } break;
        case JSON:
        {
            fprintf(outFile, "{\n");

            // Print defines info
            fprintf(outFile, "  \"defines\": [\n");
            for (int i = 0; i < defineCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      \"name\": \"%s\",\n", defines[i].name);
                fprintf(outFile, "      \"type\": \"%s\",\n", StrDefineType(defines[i].type));
                if (defines[i].isHex) // INT or LONG
                {
                    fprintf(outFile, "      \"value\": %ld,\n", strtol(defines[i].value, NULL, 16));
                }
                else if ((defines[i].type == INT) ||
                         (defines[i].type == LONG) ||
                         (defines[i].type == FLOAT) ||
                         (defines[i].type == DOUBLE) ||
                         (defines[i].type == STRING))
                {
                    fprintf(outFile, "      \"value\": %s,\n", defines[i].value);
                }
                else
                {
                    fprintf(outFile, "      \"value\": \"%s\",\n", defines[i].value);
                }
                fprintf(outFile, "      \"description\": \"%s\"\n", defines[i].desc);
                fprintf(outFile, "    }");

                if (i < defineCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  ],\n");

            // Print structs info
            fprintf(outFile, "  \"structs\": [\n");
            for (int i = 0; i < structCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      \"name\": \"%s\",\n", structs[i].name);
                fprintf(outFile, "      \"description\": \"%s\",\n", EscapeBackslashes(structs[i].desc));
                fprintf(outFile, "      \"fields\": [\n");
                for (int f = 0; f < structs[i].fieldCount; f++)
                {
                    fprintf(outFile, "        {\n");
                    fprintf(outFile, "          \"type\": \"%s\",\n", structs[i].fieldType[f]);
                    fprintf(outFile, "          \"name\": \"%s\",\n", structs[i].fieldName[f]);
                    fprintf(outFile, "          \"description\": \"%s\"\n", EscapeBackslashes(structs[i].fieldDesc[f]));
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

            // Print aliases info
            fprintf(outFile, "  \"aliases\": [\n");
            for (int i = 0; i < aliasCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      \"type\": \"%s\",\n", aliases[i].type);
                fprintf(outFile, "      \"name\": \"%s\",\n", aliases[i].name);
                fprintf(outFile, "      \"description\": \"%s\"\n", aliases[i].desc);
                fprintf(outFile, "    }");

                if (i < aliasCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  ],\n");

            // Print enums info
            fprintf(outFile, "  \"enums\": [\n");
            for (int i = 0; i < enumCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      \"name\": \"%s\",\n", enums[i].name);
                fprintf(outFile, "      \"description\": \"%s\",\n", EscapeBackslashes(enums[i].desc));
                fprintf(outFile, "      \"values\": [\n");
                for (int e = 0; e < enums[i].valueCount; e++)
                {
                    fprintf(outFile, "        {\n");
                    fprintf(outFile, "          \"name\": \"%s\",\n", enums[i].valueName[e]);
                    fprintf(outFile, "          \"value\": %i,\n", enums[i].valueInteger[e]);
                    fprintf(outFile, "          \"description\": \"%s\"\n", EscapeBackslashes(enums[i].valueDesc[e]));
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

            // Print callbacks info
            fprintf(outFile, "  \"callbacks\": [\n");
            for (int i = 0; i < callbackCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      \"name\": \"%s\",\n", callbacks[i].name);
                fprintf(outFile, "      \"description\": \"%s\",\n", EscapeBackslashes(callbacks[i].desc));
                fprintf(outFile, "      \"returnType\": \"%s\"", callbacks[i].retType);

                if (callbacks[i].paramCount == 0) fprintf(outFile, "\n");
                else
                {
                    fprintf(outFile, ",\n      \"params\": [\n");
                    for (int p = 0; p < callbacks[i].paramCount; p++)
                    {
                        fprintf(outFile, "        {\n");
                        fprintf(outFile, "          \"type\": \"%s\",\n", callbacks[i].paramType[p]);
                        fprintf(outFile, "          \"name\": \"%s\"\n", callbacks[i].paramName[p]);
                        fprintf(outFile, "        }");
                        if (p < callbacks[i].paramCount - 1) fprintf(outFile, ",\n");
                        else fprintf(outFile, "\n");
                    }
                    fprintf(outFile, "      ]\n");
                }
                fprintf(outFile, "    }");

                if (i < callbackCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  ],\n");

            // Print functions info
            fprintf(outFile, "  \"functions\": [\n");
            for (int i = 0; i < funcCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      \"name\": \"%s\",\n", funcs[i].name);
                fprintf(outFile, "      \"description\": \"%s\",\n", EscapeBackslashes(funcs[i].desc));
                fprintf(outFile, "      \"returnType\": \"%s\"", funcs[i].retType);

                if (funcs[i].paramCount == 0) fprintf(outFile, "\n");
                else
                {
                    fprintf(outFile, ",\n      \"params\": [\n");
                    for (int p = 0; p < funcs[i].paramCount; p++)
                    {
                        fprintf(outFile, "        {\n");
                        fprintf(outFile, "          \"type\": \"%s\",\n", funcs[i].paramType[p]);
                        fprintf(outFile, "          \"name\": \"%s\"\n", funcs[i].paramName[p]);
                        fprintf(outFile, "        }");
                        if (p < funcs[i].paramCount - 1) fprintf(outFile, ",\n");
                        else fprintf(outFile, "\n");
                    }
                    fprintf(outFile, "      ]\n");
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
                <Defines count="">
                    <Define name="" type="" value="" desc="" />
                </Defines>
                <Structs count="">
                    <Struct name="" fieldCount="" desc="">
                        <Field type="" name="" desc="" />
                        <Field type="" name="" desc="" />
                    </Struct>
                <Structs>
                <Aliases count="">
                    <Alias type="" name="" desc="" />
                </Aliases>
                <Enums count="">
                    <Enum name="" valueCount="" desc="">
                        <Value name="" integer="" desc="" />
                        <Value name="" integer="" desc="" />
                    </Enum>
                </Enums>
                <Callbacks count="">
                    <Callback name="" retType="" paramCount="" desc="">
                        <Param type="" name="" desc="" />
                        <Param type="" name="" desc="" />
                    </Callback>
                </Callbacks>
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

            // Print defines info
            fprintf(outFile, "    <Defines count=\"%i\">\n", defineCount);
            for (int i = 0; i < defineCount; i++)
            {
                fprintf(outFile, "        <Define name=\"%s\" type=\"%s\" ", defines[i].name, StrDefineType(defines[i].type));
                if (defines[i].type == STRING)
                {
                    fprintf(outFile, "value=%s", defines[i].value);
                }
                else
                {
                    fprintf(outFile, "value=\"%s\"", defines[i].value);
                }
                fprintf(outFile, " desc=\"%s\" />\n", defines[i].desc);
            }
            fprintf(outFile, "    </Defines>\n");

            // Print structs info
            fprintf(outFile, "    <Structs count=\"%i\">\n", structCount);
            for (int i = 0; i < structCount; i++)
            {
                fprintf(outFile, "        <Struct name=\"%s\" fieldCount=\"%i\" desc=\"%s\">\n", structs[i].name, structs[i].fieldCount, structs[i].desc);
                for (int f = 0; f < structs[i].fieldCount; f++)
                {
                    fprintf(outFile, "            <Field type=\"%s\" name=\"%s\" desc=\"%s\" />\n", structs[i].fieldType[f], structs[i].fieldName[f], structs[i].fieldDesc[f]);
                }
                fprintf(outFile, "        </Struct>\n");
            }
            fprintf(outFile, "    </Structs>\n");

            // Print aliases info
            fprintf(outFile, "    <Aliases count=\"%i\">\n", aliasCount);
            for (int i = 0; i < aliasCount; i++)
            {
                fprintf(outFile, "        <Alias type=\"%s\" name=\"%s\" desc=\"%s\" />\n", aliases[i].name, aliases[i].type, aliases[i].desc);
            }
            fprintf(outFile, "    </Aliases>\n");

            // Print enums info
            fprintf(outFile, "    <Enums count=\"%i\">\n", enumCount);
            for (int i = 0; i < enumCount; i++)
            {
                fprintf(outFile, "        <Enum name=\"%s\" valueCount=\"%i\" desc=\"%s\">\n", enums[i].name, enums[i].valueCount, enums[i].desc);
                for (int v = 0; v < enums[i].valueCount; v++)
                {
                    fprintf(outFile, "            <Value name=\"%s\" integer=\"%i\" desc=\"%s\" />\n", enums[i].valueName[v], enums[i].valueInteger[v], enums[i].valueDesc[v]);
                }
                fprintf(outFile, "        </Enum>\n");
            }
            fprintf(outFile, "    </Enums>\n");

            // Print callbacks info
            fprintf(outFile, "    <Callbacks count=\"%i\">\n", callbackCount);
            for (int i = 0; i < callbackCount; i++)
            {
                fprintf(outFile, "        <Callback name=\"%s\" retType=\"%s\" paramCount=\"%i\" desc=\"%s\">\n", callbacks[i].name, callbacks[i].retType, callbacks[i].paramCount, callbacks[i].desc);
                for (int p = 0; p < callbacks[i].paramCount; p++)
                {
                    fprintf(outFile, "            <Param type=\"%s\" name=\"%s\" desc=\"%s\" />\n", callbacks[i].paramType[p], callbacks[i].paramName[p], callbacks[i].paramDesc[p]);
                }
                fprintf(outFile, "        </Callback>\n");
            }
            fprintf(outFile, "    </Callbacks>\n");

            // Print functions info
            fprintf(outFile, "    <Functions count=\"%i\">\n", funcCount);
            for (int i = 0; i < funcCount; i++)
            {
                fprintf(outFile, "        <Function name=\"%s\" retType=\"%s\" paramCount=\"%i\" desc=\"%s\">\n", funcs[i].name, funcs[i].retType, funcs[i].paramCount, funcs[i].desc);
                for (int p = 0; p < funcs[i].paramCount; p++)
                {
                    fprintf(outFile, "            <Param type=\"%s\" name=\"%s\" desc=\"%s\" />\n", funcs[i].paramType[p], funcs[i].paramName[p], funcs[i].paramDesc[p]);
                }
                fprintf(outFile, "        </Function>\n");
            }
            fprintf(outFile, "    </Functions>\n");

            fprintf(outFile, "</raylibAPI>\n");

        } break;
        case LUA:
        {
            fprintf(outFile, "return {\n");

            // Print defines info
            fprintf(outFile, "  defines = {\n");
            for (int i = 0; i < defineCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      name = \"%s\",\n", defines[i].name);
                fprintf(outFile, "      type = \"%s\",\n", StrDefineType(defines[i].type));
                if ((defines[i].type == INT) ||
                    (defines[i].type == LONG) ||
                    (defines[i].type == FLOAT) ||
                    (defines[i].type == DOUBLE) ||
                    (defines[i].type == STRING))
                {
                    fprintf(outFile, "      value = %s,\n", defines[i].value);
                }
                else
                {
                    fprintf(outFile, "      value = \"%s\",\n", defines[i].value);
                }
                fprintf(outFile, "      description = \"%s\"\n", defines[i].desc);
                fprintf(outFile, "    }");

                if (i < defineCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  },\n");

            // Print structs info
            fprintf(outFile, "  structs = {\n");
            for (int i = 0; i < structCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      name = \"%s\",\n", structs[i].name);
                fprintf(outFile, "      description = \"%s\",\n", EscapeBackslashes(structs[i].desc));
                fprintf(outFile, "      fields = {\n");
                for (int f = 0; f < structs[i].fieldCount; f++)
                {
                    fprintf(outFile, "        {\n");
                    fprintf(outFile, "          type = \"%s\",\n", structs[i].fieldType[f]);
                    fprintf(outFile, "          name = \"%s\",\n", structs[i].fieldName[f]);
                    fprintf(outFile, "          description = \"%s\"\n", EscapeBackslashes(structs[i].fieldDesc[f]));
                    fprintf(outFile, "        }");
                    if (f < structs[i].fieldCount - 1) fprintf(outFile, ",\n");
                    else fprintf(outFile, "\n");
                }
                fprintf(outFile, "      }\n");
                fprintf(outFile, "    }");
                if (i < structCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  },\n");

            // Print aliases info
            fprintf(outFile, "  aliases = {\n");
            for (int i = 0; i < aliasCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      type = \"%s\",\n", aliases[i].type);
                fprintf(outFile, "      name = \"%s\",\n", aliases[i].name);
                fprintf(outFile, "      description = \"%s\"\n", aliases[i].desc);
                fprintf(outFile, "    }");

                if (i < aliasCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  },\n");

            // Print enums info
            fprintf(outFile, "  enums = {\n");
            for (int i = 0; i < enumCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      name = \"%s\",\n", enums[i].name);
                fprintf(outFile, "      description = \"%s\",\n", EscapeBackslashes(enums[i].desc));
                fprintf(outFile, "      values = {\n");
                for (int e = 0; e < enums[i].valueCount; e++)
                {
                    fprintf(outFile, "        {\n");
                    fprintf(outFile, "          name = \"%s\",\n", enums[i].valueName[e]);
                    fprintf(outFile, "          value = %i,\n", enums[i].valueInteger[e]);
                    fprintf(outFile, "          description = \"%s\"\n", EscapeBackslashes(enums[i].valueDesc[e]));
                    fprintf(outFile, "        }");
                    if (e < enums[i].valueCount - 1) fprintf(outFile, ",\n");
                    else fprintf(outFile, "\n");
                }
                fprintf(outFile, "      }\n");
                fprintf(outFile, "    }");
                if (i < enumCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  },\n");

            // Print callbacks info
            fprintf(outFile, "  callbacks = {\n");
            for (int i = 0; i < callbackCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      name = \"%s\",\n", callbacks[i].name);
                fprintf(outFile, "      description = \"%s\",\n", EscapeBackslashes(callbacks[i].desc));
                fprintf(outFile, "      returnType = \"%s\"", callbacks[i].retType);

                if (callbacks[i].paramCount == 0) fprintf(outFile, "\n");
                else
                {
                    fprintf(outFile, ",\n      params = {\n");
                    for (int p = 0; p < callbacks[i].paramCount; p++)
                    {
                        fprintf(outFile, "        {type = \"%s\", name = \"%s\"}", callbacks[i].paramType[p], callbacks[i].paramName[p]);
                        if (p < callbacks[i].paramCount - 1) fprintf(outFile, ",\n");
                        else fprintf(outFile, "\n");
                    }
                    fprintf(outFile, "      }\n");
                }
                fprintf(outFile, "    }");

                if (i < callbackCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  },\n");

            // Print functions info
            fprintf(outFile, "  functions = {\n");
            for (int i = 0; i < funcCount; i++)
            {
                fprintf(outFile, "    {\n");
                fprintf(outFile, "      name = \"%s\",\n", funcs[i].name);
                fprintf(outFile, "      description = \"%s\",\n", EscapeBackslashes(funcs[i].desc));
                fprintf(outFile, "      returnType = \"%s\"", funcs[i].retType);

                if (funcs[i].paramCount == 0) fprintf(outFile, "\n");
                else
                {
                    fprintf(outFile, ",\n      params = {\n");
                    for (int p = 0; p < funcs[i].paramCount; p++)
                    {
                        fprintf(outFile, "        {type = \"%s\", name = \"%s\"}", funcs[i].paramType[p], funcs[i].paramName[p]);
                        if (p < funcs[i].paramCount - 1) fprintf(outFile, ",\n");
                        else fprintf(outFile, "\n");
                    }
                    fprintf(outFile, "      }\n");
                }
                fprintf(outFile, "    }");

                if (i < funcCount - 1) fprintf(outFile, ",\n");
                else fprintf(outFile, "\n");
            }
            fprintf(outFile, "  }\n");
            fprintf(outFile, "}\n");
        } break;
        default: break;
    }

    fclose(outFile);
}
