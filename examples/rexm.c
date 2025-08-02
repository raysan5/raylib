/*******************************************************************************************
*
*   rexm [raylib examples manager] - A simple command-line tool to manage raylib examples
*
*   Supported processes:
*    - create <new_example_name>
*    - add <example_name>
*    - rename <old_examples_name> <new_example_name>
*    - remove <example_name>
*    - validate
*
*   Files involved in the processes:
*    - raylib/examples/<category>/<category>_example_name.c
*    - raylib/examples/<category>/<category>_example_name.png
*    - raylib/examples/<category>/resources/..
*    - raylib/examples/Makefile
*    - raylib/examples/Makefile.Web
*    - raylib/examples/README.md
*    - raylib/projects/VS2022/examples/<category>_example_name.vcxproj
*    - raylib/projects/VS2022/raylib.sln
*    - raylib.com/common/examples.js
*    - raylib.com/examples/<category>/<category>_example_name.html
*    - raylib.com/examples/<category>/<category>_example_name.data
*    - raylib.com/examples/<category>/<category>_example_name.wasm
*    - raylib.com/examples/<category>/<category>_example_name.js
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2025 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>
#include <stdio.h>      // Required for: rename(), remove()
#include <string.h>     // Required for: strcmp(), strcpy()

#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO) && defined(_DEBUG)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// raylib example info struct
typedef struct {
    char category[16];
    char name[128];
    char stars;
    float verCreated;
    float verUpdated;
    char author[64];
    char authorGitHub[64];
} rlExampleInfo;

// Example management operations
typedef enum {
    OP_NONE     = 0,    // No process to do
    OP_CREATE   = 1,    // Create new example, using default template
    OP_ADD      = 2,    // Add existing examples (hopefully following template)
    OP_RENAME   = 3,    // Rename existing example
    OP_REMOVE   = 4,    // Remove existing example
    OP_VALIDATE = 5,    // Validate examples, using [examples_list.txt] as main source by default
} rlExampleOperation;

#define MAX_EXAMPLE_CATEGORIES 8

static const char *exCategories[MAX_EXAMPLE_CATEGORIES] = { "core", "shapes", "textures", "text", "models", "shaders", "audio", "others" };

//----------------------------------------------------------------------------------
// Module specific functions declaration
//----------------------------------------------------------------------------------
static int FileTextReplace(const char *fileName, const char *textLookUp, const char *textReplace);
static int FileCopy(const char *srcPath, const char *dstPath);
static int FileRename(const char *fileName, const char *fileRename);
static int FileRemove(const char *fileName);

// Load examples collection information
// NOTE 1: Load by category: "ALL", "core", "shapes", "textures", "text", "models", "shaders", others"
// NOTE 2: Sort examples list on request flag
static rlExampleInfo *LoadExamplesData(const char *fileName, const char *category, bool sort, int *exCount);
static void UnloadExamplesData(rlExampleInfo *exInfo);

// Get text lines (by line-breaks '\n')
// WARNING: It does not copy text data, just returns line pointers 
static const char **GetTextLines(const char *text, int *count);

// raylib example line info parser
// Parses following line format: core/core_basic_window;⭐️☆☆☆;1.0;1.0;"Ray"/@raysan5
static int ParseExampleInfoLine(const char *line, rlExampleInfo *entry);

// Sort array of strings by name
// WARNING: items[] pointers are reorganized
static void SortExampleByName(rlExampleInfo *items, int count);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Paths required for examples management
    // TODO: Avoid hardcoding path values...
    char *exBasePath = "C:/GitHub/raylib/examples";
    char *exWebPath = "C:/GitHub/raylib.com/examples";
    char *exTemplateFilePath = "C:/GitHub/raylib/examples/examples_template.c";
    char *exTemplateScreenshot = "C:/GitHub/raylib/examples/examples_template.png";
    char *exCollectionListPath = "C:/GitHub/raylib/examples/examples_list.txt";
    
    char inFileName[1024] = { 0 };  // Example input filename (to be added)

    char exName[64] = { 0 };        // Example name, without extension: core_basic_window
    char exCategory[32] = { 0 };    // Example category: core
    char exRename[64] = { 0 };      // Example re-name, without extension

    int opCode = OP_NONE;           // Operation code: 0-None(Help), 1-Create, 2-Add, 3-Rename, 4-Remove

    // Command-line usage mode
    //--------------------------------------------------------------------------------------
    if (argc > 1)
    {
        // Supported commands:
        //    help                          : Provides command-line usage information (default)
        //    create <new_example_name>     : Creates an empty example, from internal template
        //    add <example_name>            : Add existing example, category extracted from name
        //    rename <old_examples_name> <new_example_name> : Rename an existing example
        //    remove <example_name>         : Remove an existing example
        //    validate                      : Validate examples collection
        if (strcmp(argv[1], "create") == 0)
        {
            // Check for valid upcoming argument
            if (argc == 2) LOG("WARNING: No filename provided to create\n");
            else if (argc > 3) LOG("WARNING: Too many arguments provided\n");
            else
            {
                // TODO: Additional security checks for file name?

                strcpy(exName, argv[2]); // Register filename for new example creation
                strncpy(exCategory, exName, TextFindIndex(exName, "_"));
                opCode = 1;
            }
        }
        else if (strcmp(argv[1], "add") == 0)
        {
            // Check for valid upcoming argument
            if (argc == 2) LOG("WARNING: No filename provided to create\n");
            else if (argc > 3) LOG("WARNING: Too many arguments provided\n");
            else
            {
                if (IsFileExtension(argv[2], ".c")) // Check for valid file extension: input
                {
                    if (FileExists(inFileName))
                    {
                        strcpy(inFileName, argv[2]); // Register filename for addition
                        strcpy(exName, GetFileNameWithoutExt(argv[2])); // Register example name
                        strncpy(exCategory, exName, TextFindIndex(exName, "_"));
                        opCode = 2;
                    }
                    else LOG("WARNING: Input file not found, include path\n");
                }
                else LOG("WARNING: Input file extension not recognized (.c)\n");
            }
        }
        else if (strcmp(argv[1], "rename") == 0)
        {
            if (argc == 2) LOG("WARNING: No filename provided to be renamed\n");
            else if (argc > 4) LOG("WARNING: Too many arguments provided\n");
            else
            {
                strcpy(exName, argv[2]); // Register example name
                strncpy(exCategory, exName, TextFindIndex(exName, "_"));
                strcpy(exRename, argv[3]);
                // TODO: Consider rename with change of category
                opCode = 3;
            }
        }
        else if (strcmp(argv[1], "remove") == 0)
        {
            // Check for valid upcoming argument
            if (argc == 2) LOG("WARNING: No filename provided to create\n");
            else if (argc > 3) LOG("WARNING: Too many arguments provided\n");
            else
            {
                strcpy(exName, argv[2]); // Register filename for removal
                opCode = 4;
            }
        }
        else if (strcmp(argv[1], "validate") == 0)
        {
            opCode = 5;
        }
    }

    // Load examples collection information
    //exInfo = LoadExamplesData(exCollectionListPath, "core", true, &exInfoCount);    
    //for (int i = 0; i < exInfoCount; i++) printf("%i - %s [%i]\n", i + 1, exInfo[i].name, exInfo[i].stars);

    switch (opCode)
    {
        case 1:     // Create: New example from template
        {
            // Create: raylib/examples/<category>/<category>_example_name.c
            char *exText = LoadFileText(exTemplateFilePath);
            char *exTextUpdated[6] = { 0 };
            int exIndex = TextFindIndex(exText, "/****************");
            
            exTextUpdated[0] = TextReplace(exText + exIndex, "<module>", exCategory);
            exTextUpdated[1] = TextReplace(exTextUpdated[0], "<name>", exName + strlen(exCategory) + 1);
            //TextReplace(newExample, "<user_name>", "Ray");
            //TextReplace(newExample, "@<user_github>", "@raysan5");
            //TextReplace(newExample, "<year_created>", 2025);
            //TextReplace(newExample, "<year_updated>", 2025);
            
            SaveFileText(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName), exTextUpdated[1]);
            for (int i = 0; i < 6; i++) { MemFree(exTextUpdated[i]); exTextUpdated[i] = NULL; }
            UnloadFileText(exText);
        }
        case 2:     // Add: Example from command-line input filename
        {
            // Create: raylib/examples/<category>/<category>_example_name.c
            if (opCode != 1) FileCopy(inFileName, TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));

            // Create: raylib/examples/<category>/<category>_example_name.png
            FileCopy(exTemplateScreenshot, TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName)); // WARNING: To be updated manually!

            // Copy: raylib/examples/<category>/resources/...  // WARNING: To be updated manually!
            
            // Add example to the main collection list, if not already there
            // NOTE: Required format: shapes;shapes_basic_shapes;⭐️☆☆☆;1.0;4.2;"Ray";@raysan5
            //------------------------------------------------------------------------------------------------
            char *exColInfo = LoadFileText(exCollectionListPath);
            if (TextFindIndex(exColInfo, exName) == -1) // Example not found
            {
                char *exColInfoUpdated = (char *)RL_CALLOC(2*1024*1024, 1); // Updated list copy, 2MB
                
                // Add example to the main list, by category
                // by default add it last in the category list
                // NOTE: When populating to other files, lists are sorted by name
                int nextCatIndex = 0;
                if (strcmp(exCategory, "core") == 0) nextCatIndex = 1;
                else if (strcmp(exCategory, "shapes") == 0) nextCatIndex = 2;
                else if (strcmp(exCategory, "textures") == 0) nextCatIndex = 3;
                else if (strcmp(exCategory, "text") == 0) nextCatIndex = 4;
                else if (strcmp(exCategory, "models") == 0) nextCatIndex = 5;
                else if (strcmp(exCategory, "shaders") == 0) nextCatIndex = 6;
                else if (strcmp(exCategory, "audio") == 0) nextCatIndex = 7;
                else if (strcmp(exCategory, "others") == 0) nextCatIndex = -1; // Add to EOF
                
                if (nextCatIndex == -1)
                {
                    // Add example to the end of the list
                    int endIndex = (int)strlen(exColInfo);
                    memcpy(exColInfoUpdated, exColInfo, endIndex);
                    sprintf(exColInfoUpdated + endIndex, TextFormat("\n%s/%s\n", exCategory, exName));
                }
                else
                {
                    // Add example to the end of the category list
                    // TODO: Get required example info from example file header (if provided)
                    // NOTE: If no example info is provided (other than category/name), just using some default values
                    int catIndex = TextFindIndex(exColInfo, exCategories[nextCatIndex]);
                    memcpy(exColInfoUpdated, exColInfo, catIndex);
                    int textWritenSize = sprintf(exColInfoUpdated + catIndex, TextFormat("%s;%s;⭐️☆☆☆;6.0;6.0;\"Ray\";@raysan5\n", exCategory, exName));
                    memcpy(exColInfoUpdated + catIndex + textWritenSize, exColInfo + catIndex, strlen(exColInfo) - catIndex);
                }
                
                SaveFileText(exCollectionListPath, exColInfoUpdated);
                RL_FREE(exColInfoUpdated);
            }
            UnloadFileText(exColInfo);
            //------------------------------------------------------------------------------------------------

            // Edit: raylib/examples/Makefile --> Add new example
            //------------------------------------------------------------------------------------------------
            /*
            char *mkText = LoadFileText(TextFormat("%s/Makefile", exBasePath));
            char *mkTextUpdated = (char *)RL_CALLOC(2*1024*1024, 1); // Updated Makefile copy, 2MB
            
            int exListStartIndex = TextFindIndex(mkText, "#EXAMPLES_LIST_START");
            int exListEndIndex = TextFindIndex(mkText, "#EXAMPLES_LIST_END");
            
            int mkIndex = 0;
            memcpy(mkTextUpdated, mkText, exListStartIndex);
            mkIndex = sprintf(mkTextUpdated + exListStartIndex, "#EXAMPLES_LIST_START\n");

            for (int i = 0; i < MAX_EXAMPLE_CATEGORIES; i++)
            {
                mkIndex += sprintf(mkTextUpdated + exListStartIndex + mkIndex, TextFormat("%s = \\\n", TextToUpper(exCategories[i])));

                int exCount = 0;
                rlExampleInfo *exCatList = LoadExamplesData(exCollectionListPath, exCategories[i], true, &exCount);
                
                for (int x = 0; x < exCount - 1; x++) mkIndex += sprintf(mkTextUpdated + exListStartIndex + mkIndex, TextFormat("    %s/%s \\\n", exCatList[x].category, exCatList[x].name));
                mkIndex += sprintf(mkTextUpdated + exListStartIndex + mkIndex, TextFormat("    %s/%s\n\n", exCatList[exCount - 1].category, exCatList[exCount - 1].name));
                
                UnloadExamplesData(exCatList);
            }

            // Add the remaining part of the original file
            memcpy(mkTextUpdated + exListStartIndex + mkIndex, mkText + exListEndIndex, strlen(mkText) - exListEndIndex);

            // Save updated file
            SaveFileText(TextFormat("%s/Makefile", exBasePath), mkTextUpdated);
            UnloadFileText(mkText);
            RL_FREE(mkTextUpdated);
            */
            //------------------------------------------------------------------------------------------------
            
            // Edit: raylib/examples/Makefile.Web --> Add new example
            //------------------------------------------------------------------------------------------------
            
            // TODO.
            
            //------------------------------------------------------------------------------------------------
            
            // Edit: raylib/examples/README.md --> Add new example
            //------------------------------------------------------------------------------------------------
            // TODO: Use [examples_list.txt] to update/regen README.md
            //Look for "| 01 | "
            // Lines format: | 01 | [core_basic_window](core/core_basic_window.c) | <img src="core/core_basic_window.png" alt="core_basic_window" width="80"> | ⭐️☆☆☆ | 1.0 | 1.0 | [Ray](https://github.com/raysan5) |

            
            //------------------------------------------------------------------------------------------------
            
            // Create: raylib/projects/VS2022/examples/<category>_example_name.vcxproj
            //------------------------------------------------------------------------------------------------
            FileCopy(TextFormat("%s/../projects/VS2022/examples/core_basic_window.vcxproj", exBasePath),
                TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName));
            FileTextReplace(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName), 
                "core_basic_window", exName);
            FileTextReplace(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName), 
                "..\\..\\examples\\core", TextFormat("..\\..\\examples\\%s", exCategory));
            
            // Edit: raylib/projects/VS2022/raylib.sln --> Add new example project
            system(TextFormat("dotnet solution %s/../projects/VS2022/raylib.sln add %s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exBasePath, exName));
            //------------------------------------------------------------------------------------------------
            
            // Edit: raylib.com/common/examples.js --> Add new example
            // NOTE: Entries format: exampleEntry('⭐️☆☆☆' , 'core'    , 'basic_window'),
            //------------------------------------------------------------------------------------------------
            
            char *jsText = LoadFileText(TextFormat("%s/../common/examples.js", exWebPath));
            char *jsTextUpdated = (char *)RL_CALLOC(2*1024*1024, 1); // Updated examples.js copy, 2MB
            
            int exListStartIndex = TextFindIndex(jsText, "//EXAMPLE_DATA_LIST_START");
            int exListEndIndex = TextFindIndex(jsText, "//EXAMPLE_DATA_LIST_END");
            
            int mkIndex = 0;
            memcpy(jsTextUpdated, jsText, exListStartIndex);
            mkIndex = sprintf(jsTextUpdated + exListStartIndex, "#EXAMPLES_LIST_START\n");
            
            int jsIndex = 0;
            memcpy(jsTextUpdated, jsText, exListStartIndex);
            jsIndex = sprintf(jsTextUpdated + exListStartIndex, "//EXAMPLE_DATA_LIST_START\n");
            jsIndex += sprintf(jsTextUpdated + exListStartIndex + jsIndex, "    var exampleData = [\n");

            // NOTE: We avoid "others" category
            for (int i = 0, exCount = 0; i < MAX_EXAMPLE_CATEGORIES - 1; i++)
            {
                rlExampleInfo *exCatList = LoadExamplesData(exCollectionListPath, exCategories[i], false, &exCount); 
                for (int x = 0; x < exCount; x++)
                {
                    char stars[16] = { 0 };
                    for (int s = 0; s < 4; s++)
                    {
                        if (s < exCatList[x].stars) strcpy(stars + 3*s, "⭐️");
                        else strcpy(stars + 3*s, "☆");
                    }

                    if ((i == 6) && (x == (exCount - 1)))
                    {
                        // Last line to add, special case to consider
                        jsIndex += sprintf(jsTextUpdated + exListStartIndex + jsIndex,
                            TextFormat("        exampleEntry('%s', '%s', '%s')];\n", stars, exCatList[x].category, exCatList[x].name + strlen(exCatList[x].category) + 1));
                    }
                    else
                    {
                        jsIndex += sprintf(jsTextUpdated + exListStartIndex + jsIndex,
                            TextFormat("        exampleEntry('%s', '%s', '%s'),\n", stars, exCatList[x].category, exCatList[x].name + strlen(exCatList[x].category) + 1));

                    }
                }

                UnloadExamplesData(exCatList);
            }
            
            // Add the remaining part of the original file
            memcpy(jsTextUpdated + exListStartIndex + jsIndex, jsText + exListEndIndex, strlen(jsText) - exListEndIndex);
            
            // Save updated file
            SaveFileText(TextFormat("%s/../common/examples.js", exWebPath), jsTextUpdated);
            UnloadFileText(jsText);
            RL_FREE(jsTextUpdated);
            
            //------------------------------------------------------------------------------------------------

            // Recompile example (on raylib side)
            // NOTE: Tools requirements: emscripten, w64devkit
            // Compile to: raylib.com/examples/<category>/<category>_example_name.html
            // Compile to: raylib.com/examples/<category>/<category>_example_name.data
            // Compile to: raylib.com/examples/<category>/<category>_example_name.wasm
            // Compile to: raylib.com/examples/<category>/<category>_example_name.js
            // TODO: WARNING: This .BAT is not portable and it does not consider RESOURCES for Web properly,
            // Makefile.Web should be used... but it requires proper editing first!
            system(TextFormat("%s/build_example_web.bat %s/%s", exBasePath, exCategory, exName));

            // Copy results to web side
            FileCopy(TextFormat("%s/%s/%s.html", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.html", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.data", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.data", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.wasm", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.wasm", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.js", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.js", exWebPath, exCategory, exName));
        } break;
        case 3:     // Rename
        {
            // Rename all required files
            rename(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.c", exBasePath, exCategory, exRename));
            rename(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.png", exBasePath, exCategory, exRename));

            FileTextReplace(TextFormat("%s/Makefile", exBasePath), exName, exRename);
            FileTextReplace(TextFormat("%s/Makefile.Web", exBasePath), exName, exRename);
            FileTextReplace(TextFormat("%s/README.md", exBasePath), exName, exRename);

            rename(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName),
                TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exRename));
            FileTextReplace(TextFormat("%s/../projects/VS2022/raylib.sln", exBasePath), exName, exRename);

            // Remove old web compilation
            FileTextReplace(TextFormat("%s/../common/examples.js", exWebPath), exName, exRename);
            remove(TextFormat("%s/%s/%s.html", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.data", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.wasm", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.js", exWebPath, exCategory, exName));

            // Recompile example (on raylib side)
            // NOTE: Tools requirements: emscripten, w64devkit
            system(TextFormat("%s/../build_example_web.bat %s/%s", exBasePath, exCategory, exName));

            // Copy results to web side
            FileCopy(TextFormat("%s/%s/%s.html", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.html", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.data", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.data", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.wasm", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.wasm", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.js", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.js", exWebPath, exCategory, exName));
        } break;
        case 4:     // Remove
        {
            // TODO: Remove and update all required files...

        } break;
        case 5:     // Validate
        {
            // TODO: Validate examples collection against [examples_list.txt]
            
            // Validate: raylib/examples/<category>/<category>_example_name.c
            // Validate: raylib/examples/<category>/<category>_example_name.png
            // Validate: raylib/examples/<category>/resources/.. -> Not possible for now...
            // Validate: raylib/examples/Makefile
            // Validate: raylib/examples/Makefile.Web
            // Validate: raylib/examples/README.md
            // Validate: raylib/projects/VS2022/examples/<category>_example_name.vcxproj
            // Validate: raylib/projects/VS2022/raylib.sln
            // Validate: raylib.com/common/examples.js
            // Validate: raylib.com/examples/<category>/<category>_example_name.html
            // Validate: raylib.com/examples/<category>/<category>_example_name.data
            // Validate: raylib.com/examples/<category>/<category>_example_name.wasm
            // Validate: raylib.com/examples/<category>/<category>_example_name.js
            
        } break;
        default:    // Help
        {
            // Supported commands:
            //    help                          : Provides command-line usage information
            //    create <new_example_name>     : Creates an empty example, from internal template
            //    add <example_name>            : Add existing example, category extracted from name
            //    rename <old_examples_name> <new_example_name> : Rename an existing example
            //    remove <example_name>         : Remove an existing example

            printf("\n////////////////////////////////////////////////////////////////////////////////////////////\n");
            printf("//                                                                                        //\n");
            printf("// rexm [raylib examples manager] - A simple command-line tool to manage raylib examples  //\n");
            printf("// powered by raylib v5.6-dev                                                             //\n");
            printf("//                                                                                        //\n");
            printf("// Copyright (c) 2025 Ramon Santamaria (@raysan5)                                         //\n");
            printf("//                                                                                        //\n");
            printf("////////////////////////////////////////////////////////////////////////////////////////////\n\n");

            printf("USAGE:\n\n");
            printf("    > rexm help|create|add|rename|remove <example_name> [<example_rename>]\n");

            printf("\nOPTIONS:\n\n");
            printf("    help                          : Provides command-line usage information\n");
            printf("    create <new_example_name>     : Creates an empty example, from internal template\n");
            printf("    add <example_name>            : Add existing example, category extracted from name\n");
            printf("                                    Supported categories: core, shapes, textures, text, models\n");
            printf("    rename <old_examples_name> <new_example_name> : Rename an existing example\n");
            printf("    remove <example_name>         : Remove an existing example\n\n");
            printf("\nEXAMPLES:\n\n");
            printf("    > rexm add shapes_custom_stars\n");
            printf("        Add and updates new example provided <shapes_custom_stars>\n\n");
            printf("    > rexm rename core_basic_window core_cool_window\n");
            printf("        Renames and updates example <core_basic_window> to <core_cool_window>\n\n");
        } break;
    }

    return 0;
}

//----------------------------------------------------------------------------------
// Module specific functions definition
//----------------------------------------------------------------------------------
// Load examples collection information
static rlExampleInfo *LoadExamplesData(const char *fileName, const char *category, bool sort, int *exCount)
{
    #define MAX_EXAMPLES_INFO   256
    
    rlExampleInfo *exInfo = (rlExampleInfo *)RL_CALLOC(MAX_EXAMPLES_INFO, sizeof(rlExampleInfo));
    int exCounter = 0;
    *exCount = 0;
    
    char *text = LoadFileText(fileName);
    
    if (text != NULL)
    {
        int lineCount = 0;
        const char **linePtrs = GetTextLines(text, &lineCount);
        
        for (int i = 0; i < lineCount; i++)
        {
            // Basic validation for lines start categories
            if ((linePtrs[i][0] != '#') && 
               ((linePtrs[i][0] == 'c') ||      // core
                (linePtrs[i][0] == 's') ||      // shapes, shaders
                (linePtrs[i][0] == 't') ||      // textures, text
                (linePtrs[i][0] == 'm') ||      // models
                (linePtrs[i][0] == 'a') ||      // audio
                (linePtrs[i][0] == 'o')))       // others
            {
                rlExampleInfo info = { 0 };
                int result = ParseExampleInfoLine(linePtrs[i], &info);
                if (result == 1) // Success on parsing
                {
                    if (strcmp(category, "ALL") == 0)
                    {
                        // Add all examples to the list
                        memcpy(&exInfo[exCounter], &info, sizeof(rlExampleInfo));
                        exCounter++;
                    }
                    else if (strcmp(info.category, category) == 0)
                    {
                        // Get only specific category examples
                        memcpy(&exInfo[exCounter], &info, sizeof(rlExampleInfo));
                        exCounter++;
                    }
                }
            }
        }
    
        UnloadFileText(text);
    }
    
    // Sorting required
    if (sort) SortExampleByName(exInfo, exCounter);

    *exCount = exCounter;
    return exInfo;
}

// Unload examples collection data
static void UnloadExamplesData(rlExampleInfo *exInfo)
{
    RL_FREE(exInfo);
}

// Replace text in an existing file
static int FileTextReplace(const char *fileName, const char *textLookUp, const char *textReplace)
{
    int result = 0;
    char *fileText = NULL;
    char *fileTextUpdated = { 0 };
    
    if (FileExists(fileName))
    {
        fileText = LoadFileText(fileName);
        fileTextUpdated = TextReplace(fileText, textLookUp, textReplace);
        result = SaveFileText(fileName, fileTextUpdated);
        MemFree(fileTextUpdated);
        UnloadFileText(fileText);
    }

    return result;
}

// Copy file from one path to another
// WARNING: Destination path must exist
static int FileCopy(const char *srcPath, const char *dstPath)
{
    int result = 0;
    int srcDataSize = 0;
    unsigned char *srcFileData = LoadFileData(srcPath, &srcDataSize);
    
    // TODO: Create required paths if they do not exist

    if ((srcFileData != NULL) && (srcDataSize > 0)) result = SaveFileData(dstPath, srcFileData, srcDataSize);

    UnloadFileData(srcFileData);

    return result;
}

// Rename file (if exists)
// NOTE: Only rename file name required, not full path
static int FileRename(const char *fileName, const char *fileRename)
{
    int result = 0;

    if (FileExists(fileName)) rename(fileName, TextFormat("%s/%s", GetDirectoryPath(fileName), fileRename));

    return result;
}

// Remove file (if exists)
static int FileRemove(const char *fileName)
{
    int result = 0;

    if (FileExists(fileName)) remove(fileName);

    return result;
}

// Get text lines (by line-breaks '\n')
// WARNING: It does not copy text data, just returns line pointers 
static const char **GetTextLines(const char *text, int *count)
{
    #define MAX_TEXT_LINE_PTRS   512

    static const char *linePtrs[MAX_TEXT_LINE_PTRS] = { 0 };
    for (int i = 0; i < MAX_TEXT_LINE_PTRS; i++) linePtrs[i] = NULL;    // Init NULL pointers to substrings

    int textSize = (int)strlen(text);

    linePtrs[0] = text;
    int len = 0;
    *count = 1;

    for (int i = 0, k = 0; (i < textSize) && (*count < MAX_TEXT_LINE_PTRS); i++)
    {
        if (text[i] == '\n')
        {
            k++;
            linePtrs[k] = &text[i + 1]; // WARNING: next value is valid?
            len = 0;
            *count += 1;
        }
        else len++;
    }

    return linePtrs;
}

// raylib example line info parser
// Parses following line format: core;core_basic_window;⭐️☆☆☆;1.0;1.0;"Ray";@raysan5
static int ParseExampleInfoLine(const char *line, rlExampleInfo *entry)
{
    #define MAX_EXAMPLE_INFO_LINE_LEN   512
    
    char temp[MAX_EXAMPLE_INFO_LINE_LEN] = { 0 };
    strncpy(temp, line, MAX_EXAMPLE_INFO_LINE_LEN); // WARNING: Copy is needed because strtok() modifies string, adds '\0' 
    temp[MAX_EXAMPLE_INFO_LINE_LEN - 1] = '\0'; // Ensure null termination
    
    int tokenCount = 0;
    char **tokens = TextSplit(line, ';', &tokenCount);

    // Get category and name
    strcpy(entry->category, tokens[0]);
    strcpy(entry->name, tokens[1]);

    // Parsing stars
    // NOTE: Counting the unicode char occurrences: ⭐️
    const char *ptr = tokens[2];
    while (*ptr) 
    {
        if (((unsigned char)ptr[0] == 0xE2) && 
            ((unsigned char)ptr[1] == 0xAD) && 
            ((unsigned char)ptr[2] == 0x90))
        {
            entry->stars++;
            ptr += 3; // Advance past multibyte character
        }
        else ptr++;
    }

    // Get raylib creation/update versions
    entry->verCreated = strtof(tokens[3], NULL);
    entry->verUpdated = strtof(tokens[4], NULL);

    // Get author and github
    char *quote1 = strchr(tokens[5], '"');
    char *quote2 = quote1? strchr(quote1 + 1, '"') : NULL;
    if (quote1 && quote2) strcpy(entry->author, quote1 + 1);
    strcpy(entry->authorGitHub, tokens[6]);

    return 1;
}

// Text compare, required for qsort() function
static int rlExampleInfoCompare(const void *a, const void *b)
{
    const rlExampleInfo *ex1 = (const rlExampleInfo *)a;
    const rlExampleInfo *ex2 = (const rlExampleInfo *)b;
    
    return strcmp(ex1->name, ex2->name);
}

// Sort array of strings by name
// WARNING: items[] pointers are reorganized
static void SortExampleByName(rlExampleInfo *items, int count)
{
    qsort(items, count, sizeof(rlExampleInfo), rlExampleInfoCompare);
}
