/**********************************************************************************************

    raylib parser - raylib header parser

    This parser scans raylib.h to get information about structs, enums and functions.
    All data is separated into parts, usually as strings. The following types are used for data:
    
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
       
       NOTE: Multiple options are supported: 
          - If value is not provided, (<valueInteger[i -1]> + 1) is assigned
          - Value description can be provided or not
          
    This parser could work with other C header files if mentioned constraints are followed.
    
    This parser does not require <string.h> library, all data is parsed directly from char buffers.

    LICENSE: zlib/libpng

    raylib-parser is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
    BSD-like license that allows static linking with closed source software:

    Copyright (c) 2021 Ramon Santamaria (@raysan5)

**********************************************************************************************/

#include <stdlib.h>             // Required for: malloc(), calloc(), realloc(), free(), atoi(), strtol()
#include <stdio.h>              // Required for: printf(), fopen(), fseek(), ftell(), fread(), fclose()
#include <stdbool.h>            // Required for: bool

#define MAX_FUNCS_TO_PARSE       512    // Maximum number of functions to parse
#define MAX_STRUCTS_TO_PARSE      64    // Maximum number of structures to parse
#define MAX_ENUMS_TO_PARSE        64    // Maximum number of enums to parse

#define MAX_LINE_LENGTH          512    // Maximum length of one line (including comments)
#define MAX_STRUCT_LINE_LENGTH  2048    // Maximum length of one struct (multiple lines)

enum OutputFormat { PlainText, JSON };       // Which format the header information should be in

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Function info data
typedef struct FunctionInfo {
    char name[64];              // Function name
    char desc[128];             // Function description (comment at the end)
    char retType[32];           // Return value type
    int paramCount;             // Number of function parameters
    char paramType[12][32];     // Parameters type (max: 12 parameters)
    char paramName[12][32];     // Parameters name (max: 12 parameters)
} FunctionInfo;

// Struct info data
typedef struct StructInfo {
    char name[64];              // Struct name
    char desc[64];              // Struct type description
    int fieldCount;             // Number of fields in the struct
    char fieldType[16][32];     // Field type (max: 16 fields)
    char fieldName[16][32];     // Field name (max: 16 fields)
    char fieldDesc[16][128];    // Field description (max: 16 fields)
} StructInfo;

// Enum info data
typedef struct EnumInfo {
    char name[64];              // Enum name
    char desc[64];              // Enum description
    int valueCount;             // Number of values in enumerator
    char valueName[128][64];    // Value name definition (max: 128 values)
    int valueInteger[128];      // Value integer (max: 128 values)
    char valueDesc[128][64];    // Value description (max: 128 values)
} EnumInfo;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
char *LoadFileText(const char *fileName, int *length);
char **GetTextLines(const char *buffer, int length, int *linesCount);
void GetDataTypeAndName(const char *typeName, int typeNameLen, char *type, char *name);
bool IsTextEqual(const char *text1, const char *text2, unsigned int count);
void MemoryCopy(void *dest, const void *src, unsigned int count);
char* CharReplace(char* text, char search, char replace);

// Main entry point
int main(int argc, char* argv[])
{
    // Help
    if (IsTextEqual(argv[1], "--help", 6)) {
        printf("Usage:\n");
        printf("  raylib_parser [--json]\n");
        return 0;
    }

    // Allow changing the output format.
    int outputFormat = 0;
    if (IsTextEqual(argv[1], "--json", 6)) {
        outputFormat = JSON;
    }

    int length = 0;
    char *buffer = LoadFileText("../src/raylib.h", &length);
    
    // Preprocess buffer to get separate lines
    // NOTE: GetTextLines() also removes leading spaces/tabs
    int linesCount = 0;
    char **lines = GetTextLines(buffer, length, &linesCount);
    
    // Print buffer lines
    //for (int i = 0; i < linesCount; i++) printf("_%s_\n", lines[i]);
    
    // Function lines pointers, selected from buffer "lines"
    int funcCount = 0;
    char **funcLines = (char **)malloc(MAX_FUNCS_TO_PARSE*sizeof(char *));

    // Structs data (multiple lines), selected from "buffer"
    int structCount = 0;
    char **structLines = (char **)malloc(MAX_STRUCTS_TO_PARSE*sizeof(char *));
    for (int i = 0; i < MAX_STRUCTS_TO_PARSE; i++) structLines[i] = (char *)calloc(MAX_STRUCT_LINE_LENGTH, sizeof(char));
    
    // Enums lines pointers, selected from buffer "lines"
    int enumCount = 0;
    int *enumLines = (int *)malloc(MAX_ENUMS_TO_PARSE*sizeof(int));
    
    // Prepare required lines for parsing
    //--------------------------------------------------------------------------------------------------
    
    // Read function lines
    for (int i = 0; i < linesCount; i++)
    {
        // Read function line (starting with "RLAPI")
        if (IsTextEqual(lines[i], "RLAPI", 5))
        {
            // Keep a pointer to the function line
            funcLines[funcCount] = lines[i];
            funcCount++;
        }
    }
    
    // Print function lines
    //for (int i = 0; i < funcCount; i++) printf("%s\n", funcLines[i]);

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
        // Read function line (starting with "RLAPI")
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
    StructInfo *structs = (StructInfo *)calloc(MAX_STRUCTS_TO_PARSE, sizeof(StructInfo));
    
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
    EnumInfo *enums = (EnumInfo *)calloc(MAX_ENUMS_TO_PARSE, sizeof(EnumInfo));
    
    for (int i = 0; i < enumCount; i++)
    {
        // TODO: Get enum description from lines[enumLines[i] - 1]
        
        for (int j = 1; j < 256; j++)   // Maximum number of lines following enum first line
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
    FunctionInfo *funcs = (FunctionInfo *)calloc(MAX_FUNCS_TO_PARSE, sizeof(FunctionInfo));
    
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
                int funcRetTypeNameLen = c - 6;     // Substract "RLAPI "
                MemoryCopy(funcRetTypeName, &funcLines[i][6], funcRetTypeNameLen);
                
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
    
    // Print structs info
    switch (outputFormat)
    {
        case PlainText: {
            printf("\nStructures found: %i\n\n", structCount);
            for (int i = 0; i < structCount; i++)
            {
                printf("Struct %02i: %s (%i fields)\n", i + 1, structs[i].name, structs[i].fieldCount);
                //printf("Description: %s\n", structs[i].desc);
                for (int f = 0; f < structs[i].fieldCount; f++) printf("  Fields %i: %s %s %s\n", f + 1, structs[i].fieldType[f], structs[i].fieldName[f], structs[i].fieldDesc[f]);
            }

            // Print enums info
            printf("\nEnums found: %i\n\n", enumCount);
            for (int i = 0; i < enumCount; i++)
            {
                printf("Enum %02i: %s (%i values)\n", i + 1, enums[i].name, enums[i].valueCount);
                //printf("Description: %s\n", enums[i].desc);
                for (int e = 0; e < enums[i].valueCount; e++) printf("  Value %s: %i\n", enums[i].valueName[e], enums[i].valueInteger[e]);
            }

            // Print function info
            printf("\nFunctions found: %i\n\n", funcCount);
            for (int i = 0; i < funcCount; i++)
            {
                printf("Function %03i: %s() (%i input parameters)\n", i + 1, funcs[i].name, funcs[i].paramCount);
                printf("  Description: %s\n", funcs[i].desc);
                printf("  Return type: %s\n", funcs[i].retType);
                for (int p = 0; p < funcs[i].paramCount; p++) printf("  Param %i: %s (type: %s)\n", p + 1, funcs[i].paramName[p], funcs[i].paramType[p]);
                if (funcs[i].paramCount == 0) printf("  No input parameters\n");
            }
        } break;
        case JSON: {
            printf("{\n");
            printf("  \"structs\": [\n");
            for (int i = 0; i < structCount; i++)
            {
                printf("    {\n");
                printf("      \"name\": \"%s\",\n", structs[i].name);
                printf("      \"description\": \"%s\",\n", structs[i].desc);
                printf("      \"fields\": [\n");
                for (int f = 0; f < structs[i].fieldCount; f++)
                {
                    printf("        {\n");
                    printf("          \"name\": \"%s\",\n", structs[i].fieldName[f]),
                    printf("          \"type\": \"%s\",\n", structs[i].fieldType[f]),
                    printf("          \"description\": \"%s\"\n", structs[i].fieldDesc[f] + 3),
                    printf("        }");
                    if (f < structs[i].fieldCount - 1)
                        printf(",\n");
                    else
                        printf("\n");
                }
                printf("      ]\n");
                printf("    }");
                if (i < structCount - 1)
                    printf(",\n");
                else
                    printf("\n");
            }
            printf("  ],\n");

            // Print enums info
            printf("  \"enums\": [\n");
            for (int i = 0; i < enumCount; i++)
            {
                printf("    {\n");
                printf("      \"name\": \"%s\",\n", enums[i].name);
                printf("      \"description\": \"%s\",\n", enums[i].desc + 3);
                printf("      \"values\": [\n");
                for (int e = 0; e < enums[i].valueCount; e++)
                {
                    printf("        {\n");
                    printf("          \"name\": \"%s\",\n", enums[i].valueName[e]),
                    printf("          \"value\": %i,\n", enums[i].valueInteger[e]),
                    printf("          \"description\": \"%s\"\n", enums[i].valueDesc[e] + 3),
                    printf("        }");
                    if (e < enums[i].valueCount - 1)
                        printf(",\n");
                    else
                        printf("\n");
                }
                printf("      ]\n");
                printf("    }");
                if (i < enumCount - 1)
                    printf(",\n");
                else
                    printf("\n");
            }
            printf("  ],\n");

            // Print function info
            printf("  \"functions\": [\n");
            for (int i = 0; i < funcCount; i++)
            {
                printf("    {\n");
                printf("      \"name\": \"%s\",\n", funcs[i].name);
                printf("      \"description\": \"%s\",\n", CharReplace(funcs[i].desc, '\\', ' ') + 3);
                printf("      \"returnType\": \"%s\"", funcs[i].retType);

                if (funcs[i].paramCount == 0)
                    printf("\n");
                else
                {
                    printf(",\n      \"params\": {\n");
                    for (int p = 0; p < funcs[i].paramCount; p++)
                    {
                        printf("        \"%s\": \"%s\"", funcs[i].paramName[p], funcs[i].paramType[p]);
                        if (p < funcs[i].paramCount - 1)
                            printf(",\n");
                        else
                            printf("\n");
                    }
                    printf("      }\n");
                }
                printf("    }");

                if (i < funcCount - 1)
                    printf(",\n");
                else
                    printf("\n");
            }
            printf("  ]\n");
            printf("}\n");
        } break;
    }
    
    free(funcs);
    free(structs);
    free(enums);
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Load text data from file, returns a '\0' terminated string
// NOTE: text chars array should be freed manually
char *LoadFileText(const char *fileName, int *length)
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
                *length = size;
                
                text = (char *)calloc((size + 1), sizeof(char));
                unsigned int count = (unsigned int)fread(text, sizeof(char), size, file);

                // WARNING: \r\n is converted to \n on reading, so,
                // read bytes count gets reduced by the number of lines
                if (count < (unsigned int)size) text = realloc(text, count + 1);

                // Zero-terminate the string
                text[count] = '\0';
            }

            fclose(file);
        }
    }

    return text;
}

// Get all lines from a text buffer (expecting lines ending with '\n')
char **GetTextLines(const char *buffer, int length, int *linesCount)
{
    //#define MAX_LINE_LENGTH     512
    
    // Get the number of lines in the text
    int count = 0;
    for (int i = 0; i < length; i++) if (buffer[i] == '\n') count++;

    //printf("Number of text lines in buffer: %i\n", count);
    
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
void GetDataTypeAndName(const char *typeName, int typeNameLen, char *type, char *name)
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

// Custom memcpy() to avoid <string.h>
void MemoryCopy(void *dest, const void *src, unsigned int count)
{
    char *srcPtr = (char *)src;
    char *destPtr = (char *)dest;

    for (unsigned int i = 0; i < count; i++) destPtr[i] = srcPtr[i];
}

// Compare two text strings, requires number of characters to compare
bool IsTextEqual(const char *text1, const char *text2, unsigned int count)
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
char* CharReplace(char* text, char search, char replace)
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
char *TextReplace(char *text, const char *replace, const char *by)
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