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

#define REXM_MAX_BUFFER_SIZE    (2*1024*1024)      // 2MB

#define REXM_MAX_RESOURCE_PATHS 256

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

// Paths required for examples management
// TODO: Avoid hardcoding path values...
static const char *exBasePath = "C:/GitHub/raylib/examples";
static const char *exWebPath = "C:/GitHub/raylib.com/examples";
static const char *exTemplateFilePath = "C:/GitHub/raylib/examples/examples_template.c";
static const char *exTemplateScreenshot = "C:/GitHub/raylib/examples/examples_template.png";
static const char *exCollectionListPath = "C:/GitHub/raylib/examples/examples_list.txt";

//----------------------------------------------------------------------------------
// Module specific functions declaration
//----------------------------------------------------------------------------------
static int FileTextReplace(const char *fileName, const char *textLookUp, const char *textReplace);
static int FileCopy(const char *srcPath, const char *dstPath);
static int FileRename(const char *fileName, const char *fileRename);
static int FileRemove(const char *fileName);

// Update required files from examples collection
// UPDATES: Makefile, Makefile.Web, README.md, examples.js
static int UpdateRequiredFiles(void);

// Load examples collection information
// NOTE 1: Load by category: "ALL", "core", "shapes", "textures", "text", "models", "shaders", others"
// NOTE 2: Sort examples list on request flag
static rlExampleInfo *LoadExamplesData(const char *fileName, const char *category, bool sort, int *exCount);
static void UnloadExamplesData(rlExampleInfo *exInfo);

// Get text lines (by line-breaks '\n')
// WARNING: It does not copy text data, just returns line pointers 
static char **LoadTextLines(const char *text, int *count);
static void UnloadTextLines(char **text);

// raylib example line info parser
// Parses following line format: core/core_basic_window;⭐️☆☆☆;1.0;1.0;"Ray"/@raysan5
static int ParseExampleInfoLine(const char *line, rlExampleInfo *entry);

// Sort array of strings by name
// WARNING: items[] pointers are reorganized
static void SortExampleByName(rlExampleInfo *items, int count);

// Scan resource paths in example file
static char **ScanExampleResources(const char *filePath, int *resPathCount);

// Clear resource paths scanned
static void ClearExampleResources(char **resPaths);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    char inFileName[1024] = { 0 };  // Example input filename (to be added)

    char exName[64] = { 0 };        // Example name, without extension: core_basic_window
    char exCategory[32] = { 0 };    // Example category: core
    char exRecategory[32] = { 0 };  // Example re-name category: shapes
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
                // Security checks for file name to verify category is included
                int catIndex = TextFindIndex(argv[2], "_");
                if (catIndex > 3)
                {
                    char cat[12] = { 0 };
                    strncpy(cat, argv[2], catIndex);
                    bool catFound = false;
                    for (int i = 0; i < MAX_EXAMPLE_CATEGORIES; i++) 
                    { 
                        if (TextIsEqual(cat, exCategories[i])) { catFound = true; break; }
                    }

                    if (catFound)
                    {
                        strcpy(exName, argv[2]); // Register filename for new example creation
                        strncpy(exCategory, exName, TextFindIndex(exName, "_"));
                        opCode = OP_CREATE;
                    }
                    else LOG("WARNING: Example category is not valid\n");
                }
                else LOG("WARNING: Example name does not include category\n");
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
                        // Security checks for file name to verify category is included
                        int catIndex = TextFindIndex(argv[2], "_");
                        if (catIndex > 3)
                        {
                            char cat[12] = { 0 };
                            strncpy(cat, argv[2], catIndex);
                            bool catFound = false;
                            for (int i = 0; i < MAX_EXAMPLE_CATEGORIES; i++) 
                            { 
                                if (TextIsEqual(cat, exCategories[i])) { catFound = true; break; }
                            }

                            if (catFound)
                            {
                                strcpy(inFileName, argv[2]); // Register filename for addition
                                strcpy(exName, GetFileNameWithoutExt(argv[2])); // Register example name
                                strncpy(exCategory, exName, TextFindIndex(exName, "_"));
                                opCode = OP_ADD;
                            }
                            else LOG("WARNING: Example category is not valid\n");
                        }
                        else LOG("WARNING: Example name does not include category\n");
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
                // Verify example exists in collection to be removed
                char *exColInfo = LoadFileText(exCollectionListPath);
                if (TextFindIndex(exColInfo, argv[2]) != -1) // Example in the collection
                {
                    strcpy(exName, argv[2]);    // Register example name
                    strncpy(exCategory, exName, TextFindIndex(exName, "_"));
                    strcpy(exRename, argv[3]);
                    strncpy(exRecategory, exRename, TextFindIndex(exRename, "_"));
                    opCode = OP_RENAME;
                }
                else LOG("WARNING: RENAME: Example not available in the collection\n");
                UnloadFileText(exColInfo);
            }
        }
        else if (strcmp(argv[1], "remove") == 0)
        {
            // Check for valid upcoming argument
            if (argc == 2) LOG("WARNING: No filename provided to create\n");
            else if (argc > 3) LOG("WARNING: Too many arguments provided\n");
            else
            {
                // Verify example exists in collection to be removed
                char *exColInfo = LoadFileText(exCollectionListPath);
                if (TextFindIndex(exColInfo, argv[2]) != -1) // Example in the collection
                {
                    strcpy(exName, argv[2]); // Register filename for removal
                    strncpy(exCategory, exName, TextFindIndex(exName, "_"));
                    opCode = OP_REMOVE;
                }
                else LOG("WARNING: REMOVE: Example not available in the collection\n");
                UnloadFileText(exColInfo);
            }
        }
        else if (strcmp(argv[1], "validate") == 0)
        {
            // Validate examples in collection
            // All examples in collection match all requirements on required files

            opCode = OP_VALIDATE;
        }
    }

    switch (opCode)
    {
        case 1:     // Create: New example from template
        {
            // Create: raylib/examples/<category>/<category>_example_name.c
            char *exText = LoadFileText(exTemplateFilePath);
            char *exTextUpdated[6] = { 0 };
            int exIndex = TextFindIndex(exText, "/****************");
            
            // Update required info with some defaults
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
            // Add: raylib/examples/<category>/<category>_example_name.c
            if (opCode != 1) FileCopy(inFileName, TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));

            // Create: raylib/examples/<category>/<category>_example_name.png
            FileCopy(exTemplateScreenshot, TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName)); // WARNING: To be updated manually!

            // Copy: raylib/examples/<category>/resources/...
            // -----------------------------------------------------------------------------------------
            // Scan resources used in example to copy
            int resPathCount = 0;
            char **resPaths = ScanExampleResources(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName), &resPathCount);

            if (resPathCount > 0)
            {
                for (int r = 0; r < resPathCount; r++)
                {
                    // WARNING: Special case to consider: shaders, resource paths could use conditions: "glsl%i"
                    // In this case, multiple resources are required: glsl100, glsl120, glsl330
                    if (TextFindIndex(resPaths[r], "glsl%i") > -1)
                    {
                        int glslVer[3] = { 100, 120, 330 };

                        for (int v = 0; v < 3; v++)
                        {
                            char *resPathUpdated = TextReplace(resPaths[r], "glsl%i", TextFormat("glsl%i", glslVer[v]));

                            LOG("INFO: Example resource required: %s\n", resPathUpdated);

                            if (FileExists(TextFormat("%s/%s", GetDirectoryPath(inFileName), resPathUpdated)))
                            {
                                // Verify the resources are placed in "resources" directory
                                if (TextFindIndex(resPathUpdated, "resources/") > 0)
                                {
                                    // NOTE: Look for resources in the path of the provided .c to be added
                                    // To be copied to <category>/resources directory, extra dirs are automatically created if required
                                    FileCopy(TextFormat("%s/%s", GetDirectoryPath(inFileName), resPathUpdated),
                                        TextFormat("%s/%s/%s", exBasePath, exCategory, resPathUpdated));
                                }
                                else LOG("WARNING: Example resource must be placed in 'resources' directory next to .c file\n");
                            }
                            else LOG("WARNING: Example resource can not be found in: %s\n", TextFormat("%s/%s", GetDirectoryPath(inFileName), resPathUpdated));

                            RL_FREE(resPathUpdated);
                        }
                    }
                    else
                    {
                        LOG("INFO: Example resource required: %s\n", resPaths[r]);

                        if (FileExists(TextFormat("%s/%s", GetDirectoryPath(inFileName), resPaths[r])))
                        {
                            // Verify the resources are placed in "resources" directory
                            if (TextFindIndex(resPaths[r], "resources/") > 0)
                            {
                                // NOTE: Look for resources in the path of the provided .c to be added
                                // To be copied to <category>/resources directory, extra dirs are automatically created if required
                                FileCopy(TextFormat("%s/%s", GetDirectoryPath(inFileName), resPaths[r]),
                                    TextFormat("%s/%s/%s", exBasePath, exCategory, resPaths[r]));
                            }
                            else LOG("WARNING: Example resource must be placed in 'resources' directory next to .c file\n");
                        }
                        else LOG("WARNING: Example resource can not be found in: %s\n", TextFormat("%s/%s", GetDirectoryPath(inFileName), resPaths[r]));
                    }
                }
            }

            ClearExampleResources(resPaths);
            // -----------------------------------------------------------------------------------------
            
            // Add example to the collection list, if not already there
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

                // TODO: Get required example info from example file header (if provided)
                // NOTE: If no example info is provided (other than category/name), just using some default values
                
                if (nextCatIndex == -1)
                {
                    // Add example to collection at the EOF
                    int endIndex = (int)strlen(exColInfo);
                    memcpy(exColInfoUpdated, exColInfo, endIndex);
                    sprintf(exColInfoUpdated + endIndex, TextFormat("%s;%s;⭐️☆☆☆;6.0;6.0;\"Ray\";@raysan5\n", exCategory, exName));
                }
                else
                {
                    // Add example to collection, at the end of the category list
                    int catIndex = TextFindIndex(exColInfo, exCategories[nextCatIndex]);
                    memcpy(exColInfoUpdated, exColInfo, catIndex);
                    int textWritenSize = sprintf(exColInfoUpdated + catIndex, TextFormat("%s;%s;⭐️☆☆☆;6.0;6.0;\"Ray\";@raysan5\n", exCategory, exName));
                    memcpy(exColInfoUpdated + catIndex + textWritenSize, exColInfo + catIndex, strlen(exColInfo) - catIndex);
                }
                
                SaveFileText(exCollectionListPath, exColInfoUpdated);
                RL_FREE(exColInfoUpdated);
            }
            else LOG("WARNING: ADD: Example is already on the collection\n");
            UnloadFileText(exColInfo);
            //------------------------------------------------------------------------------------------------

            // Update: Makefile, Makefile.Web, README.md, examples.js
            //------------------------------------------------------------------------------------------------
            UpdateRequiredFiles();
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

            // Recompile example (on raylib side)
            // NOTE: Tools requirements: emscripten, w64devkit
            // Compile to: raylib.com/examples/<category>/<category>_example_name.html
            // Compile to: raylib.com/examples/<category>/<category>_example_name.data
            // Compile to: raylib.com/examples/<category>/<category>_example_name.wasm
            // Compile to: raylib.com/examples/<category>/<category>_example_name.js
            //------------------------------------------------------------------------------------------------
            // TODO: Avoid platform-specific .BAT, not portable and it does not consider RESOURCES for Web properly,
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
            //------------------------------------------------------------------------------------------------

        } break;
        case 3:     // Rename
        {
            // NOTE: At this point provided values have been validated:
            // exName, exCategory, exRename, exRecategory
            if (strcmp(exCategory, exRecategory) == 0)
            {
                // Rename example on collection
                FileTextReplace(exCollectionListPath, TextFormat("%s;%s", exCategory, exName), 
                    TextFormat("%s;%s", exRecategory, exRename));

                // Rename all required files
                rename(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.c", exBasePath, exCategory, exRename));
                rename(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.png", exBasePath, exCategory, exRename));

                // Rename example on required files
                FileTextReplace(TextFormat("%s/Makefile", exBasePath), exName, exRename);
                FileTextReplace(TextFormat("%s/Makefile.Web", exBasePath), exName, exRename);
                FileTextReplace(TextFormat("%s/README.md", exBasePath), exName, exRename);
                FileTextReplace(TextFormat("%s/../common/examples.js", exWebPath), exName, exRename);

                // Rename example project and solution
                rename(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName),
                    TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exRename));
                FileTextReplace(TextFormat("%s/../projects/VS2022/raylib.sln", exBasePath), exName, exRename);
            }
            else
            {
                // Rename with change of category
                // TODO: Reorder collection as required
                FileTextReplace(exCollectionListPath, TextFormat("%s;%s", exCategory, exName), 
                    TextFormat("%s;%s", exRecategory, exRename));

                // Rename all required files
                FileCopy(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.c", exBasePath, exCategory, exRename));
                remove(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));

                FileCopy(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.png", exBasePath, exCategory, exRename));
                remove(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName));

                UpdateRequiredFiles();
            }

            // Remove old web compilation
            remove(TextFormat("%s/%s/%s.html", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.data", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.wasm", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.js", exWebPath, exCategory, exName));

            // Recompile example (on raylib side)
            // NOTE: Tools requirements: emscripten, w64devkit
            // TODO: Avoid platform-specific .BAT file
            system(TextFormat("%s/build_example_web.bat %s/%s", exBasePath, exRecategory, exRename));

            // Copy results to web side
            FileCopy(TextFormat("%s/%s/%s.html", exBasePath, exRecategory, exRename),
                TextFormat("%s/%s/%s.html", exWebPath, exRecategory, exRename));
            FileCopy(TextFormat("%s/%s/%s.data", exBasePath, exRecategory, exRename),
                TextFormat("%s/%s/%s.data", exWebPath, exRecategory, exRename));
            FileCopy(TextFormat("%s/%s/%s.wasm", exBasePath, exRecategory, exRename),
                TextFormat("%s/%s/%s.wasm", exWebPath, exRecategory, exRename));
            FileCopy(TextFormat("%s/%s/%s.js", exBasePath, exRecategory, exRename),
                TextFormat("%s/%s/%s.js", exWebPath, exRecategory, exRename));

        } break;
        case 4:     // Remove
        {
            // Remove example from collection for files update
            //------------------------------------------------------------------------------------------------
            char *exColInfo = LoadFileText(exCollectionListPath);
            int exIndex = TextFindIndex(exColInfo, TextFormat("%s;%s", exCategory, exName));
            if (exIndex > 0) // Example found
            {
                char *exColInfoUpdated = (char *)RL_CALLOC(2*1024*1024, 1); // Updated list copy, 2MB

                memcpy(exColInfoUpdated, exColInfo, exIndex);
                int lineLen = 0;
                for (int i = exIndex; (exColInfo[i] != '\n') && (exColInfo[i] != '\0'); i++) lineLen++;
                // Remove line and copy the rest next
                memcpy(exColInfoUpdated + exIndex, exColInfo + exIndex + lineLen + 1, strlen(exColInfo) - exIndex - lineLen);

                SaveFileText(exCollectionListPath, exColInfoUpdated);
                RL_FREE(exColInfoUpdated);
            }
            else LOG("WARNING: REMOVE: Example not found in the collection\n");
            UnloadFileText(exColInfo);
            //------------------------------------------------------------------------------------------------

            // Remove: raylib/examples/<category>/<category>_example_name.c
            // Remove: raylib/examples/<category>/<category>_example_name.png
            remove(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName));
            
            // TODO: Remove: raylib/examples/<category>/resources/..
            // Get list of resources from Makefile.Web or examples ResourcesScan()
            
            UpdateRequiredFiles();
            
            // Remove: raylib/projects/VS2022/examples/<category>_example_name.vcxproj
            remove(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName));

            // Edit: raylib/projects/VS2022/raylib.sln --> Remove example project
            //---------------------------------------------------------------------------
            system(TextFormat("dotnet solution %s/../projects/VS2022/raylib.sln remove %s/../projects/VS2022/examples/%s.vcxproj", 
                exBasePath, exBasePath, exName));
            //---------------------------------------------------------------------------
            
            // Remove: raylib.com/examples/<category>/<category>_example_name.html
            // Remove: raylib.com/examples/<category>/<category>_example_name.data
            // Remove: raylib.com/examples/<category>/<category>_example_name.wasm
            // Remove: raylib.com/examples/<category>/<category>_example_name.js
            remove(TextFormat("%s/%s/%s.html", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.data", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.wasm", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.js", exWebPath, exCategory, exName));

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

// Update required files from examples collection
static int UpdateRequiredFiles(void)
{
    int result = 0;

    // Edit: raylib/examples/Makefile --> Update from collection
    //------------------------------------------------------------------------------------------------
    char *mkText = LoadFileText(TextFormat("%s/Makefile", exBasePath));
    char *mkTextUpdated = (char *)RL_CALLOC(2*1024*1024, 1); // Updated Makefile copy, 2MB

    int mkListStartIndex = TextFindIndex(mkText, "#EXAMPLES_LIST_START");
    int mkListEndIndex = TextFindIndex(mkText, "#EXAMPLES_LIST_END");

    int mkIndex = 0;
    memcpy(mkTextUpdated, mkText, mkListStartIndex);
    mkIndex = sprintf(mkTextUpdated + mkListStartIndex, "#EXAMPLES_LIST_START\n");

    for (int i = 0; i < MAX_EXAMPLE_CATEGORIES; i++)
    {
        mkIndex += sprintf(mkTextUpdated + mkListStartIndex + mkIndex, TextFormat("%s = \\\n", TextToUpper(exCategories[i])));

        int exCount = 0;
        rlExampleInfo *exCatList = LoadExamplesData(exCollectionListPath, exCategories[i], true, &exCount);

        for (int x = 0; x < exCount - 1; x++) mkIndex += sprintf(mkTextUpdated + mkListStartIndex + mkIndex, TextFormat("    %s/%s \\\n", exCatList[x].category, exCatList[x].name));
        mkIndex += sprintf(mkTextUpdated + mkListStartIndex + mkIndex, TextFormat("    %s/%s\n\n", exCatList[exCount - 1].category, exCatList[exCount - 1].name));

        UnloadExamplesData(exCatList);
    }

    // Add the remaining part of the original file
    memcpy(mkTextUpdated + mkListStartIndex + mkIndex - 1, mkText + mkListEndIndex, strlen(mkText) - mkListEndIndex);

    // Save updated file
    SaveFileText(TextFormat("%s/Makefile", exBasePath), mkTextUpdated);
    UnloadFileText(mkText);
    RL_FREE(mkTextUpdated);
    //------------------------------------------------------------------------------------------------

    // Edit: raylib/examples/Makefile.Web --> Update from collection
    // NOTE: We avoid the "others" category on web building
    //------------------------------------------------------------------------------------------------
    char *mkwText = LoadFileText(TextFormat("%s/Makefile.Web", exBasePath));
    char *mkwTextUpdated = (char *)RL_CALLOC(2*1024*1024, 1); // Updated Makefile copy, 2MB

    int mkwListStartIndex = TextFindIndex(mkwText, "#EXAMPLES_LIST_START");
    int mkwListEndIndex = TextFindIndex(mkwText, "#EXAMPLES_LIST_END");

    int mkwIndex = 0;
    memcpy(mkwTextUpdated, mkwText, mkwListStartIndex);
    mkwIndex = sprintf(mkwTextUpdated + mkwListStartIndex, "#EXAMPLES_LIST_START\n");

    // NOTE: We avoid the "others" category on web building
    for (int i = 0; i < MAX_EXAMPLE_CATEGORIES - 1; i++)
    {
        mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, TextFormat("%s = \\\n", TextToUpper(exCategories[i])));

        int exCount = 0;
        rlExampleInfo *exCatList = LoadExamplesData(exCollectionListPath, exCategories[i], true, &exCount);

        for (int x = 0; x < exCount - 1; x++) mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, TextFormat("    %s/%s \\\n", exCatList[x].category, exCatList[x].name));
        mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, TextFormat("    %s/%s\n\n", exCatList[exCount - 1].category, exCatList[exCount - 1].name));

        UnloadExamplesData(exCatList);
    }

    // Add the remaining part of the original file
    memcpy(mkwTextUpdated + mkwListStartIndex + mkwIndex - 1, mkwText + mkwListEndIndex, strlen(mkwText) - mkwListEndIndex);

    // TODO: Add new example target, considering resources            

    // Save updated file
    SaveFileText(TextFormat("%s/Makefile.Web", exBasePath), mkwTextUpdated);
    UnloadFileText(mkwText);
    RL_FREE(mkwTextUpdated);
    //------------------------------------------------------------------------------------------------

    // Edit: raylib/examples/README.md --> Update from collection
    //------------------------------------------------------------------------------------------------
    // NOTE: Using [examples_list.txt] to update/regen README.md
    // Lines format: | 01 | [core_basic_window](core/core_basic_window.c) | <img src="core/core_basic_window.png" alt="core_basic_window" width="80"> | ⭐️☆☆☆ | 1.0 | 1.0 | [Ray](https://github.com/raysan5) |
    char *mdText = LoadFileText(TextFormat("%s/README.md", exBasePath));
    char *mdTextUpdated = (char *)RL_CALLOC(2*1024*1024, 1); // Updated examples.js copy, 2MB

    int mdListStartIndex = TextFindIndex(mdText, "| 01 | ");

    int mdIndex = 0;
    memcpy(mdTextUpdated, mdText, mdListStartIndex);

    // NOTE: We keep a global examples counter
    for (int i = 0, catCount = 0, gCount = 0; i < MAX_EXAMPLE_CATEGORIES; i++)
    {
        // Every category includes some introductory text, as it is quite short, just copying it here
        // NOTE: "core" text already placed in the file
        if (i == 1)         // "shapes"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, "\n### category: shapes\n\n");
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib shapes drawing functionality, provided by raylib [shapes](../src/rshapes.c) module.\n\n");
        }
        else if (i == 2)    // "textures"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, "\n### category: textures\n\n");
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib textures functionality, including image/textures loading/generation and drawing, provided by raylib [textures](../src/rtextures.c) module.\n\n");
        }
        else if (i == 3)    // "text"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, "\n### category: text\n\n");
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib text functionality, including sprite fonts loading/generation and text drawing, provided by raylib [text](../src/rtext.c) module.\n\n");
        }
        else if (i == 4)    // "models"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, "\n### category: models\n\n");
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib models functionality, including models loading/generation and drawing, provided by raylib [models](../src/rmodels.c) module.\n\n");
        }
        else if (i == 5)    // "shaders"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, "\n### category: shaders\n\n");
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib shaders functionality, including shaders loading, parameters configuration and drawing using them (model shaders and postprocessing shaders). This functionality is directly provided by raylib [rlgl](../src/rlgl.c) module.\n\n");
        }
        else if (i == 6)    // "audio"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, "\n### category: audio\n\n");
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib audio functionality, including sound/music loading and playing. This functionality is provided by raylib [raudio](../src/raudio.c) module. Note this module can be used standalone independently of raylib.\n\n");
        }
        else if (i == 7)    // "others"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, "\n### category: others\n\n");
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples showing raylib misc functionality that does not fit in other categories, like standalone modules usage or examples integrating external libraries.\n\n");
        }

        if (i > 0)
        {
            // Table header required
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, "| ## | example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |\n");
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, "|----|----------|--------|:-------------------:|:------------------:|:-----------------------:|:----------------------|\n");
        }

        rlExampleInfo *exCatList = LoadExamplesData(exCollectionListPath, exCategories[i], false, &catCount); 
        for (int x = 0; x < catCount; x++)
        {
            char stars[16] = { 0 };
            for (int s = 0; s < 4; s++)
            {
                if (s < exCatList[x].stars) strcpy(stars + 3*s, "⭐️");
                else strcpy(stars + 3*s, "☆");
            }

            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                TextFormat("| %02i | [%s](%s/%s.c) | <img src=\"%s/%s.png\" alt=\"%s\" width=\"80\"> | %s | %.1f | %.1f | [%s](https://github.com/%s) |\n", 
                    gCount + 1, exCatList[x].name, exCatList[x].category, exCatList[x].name, exCatList[x].category, exCatList[x].name, exCatList[x].name,
                    stars, exCatList[x].verCreated, exCatList[x].verUpdated, exCatList[x].author, exCatList[x].authorGitHub + 1));

            gCount++;
        }

        UnloadExamplesData(exCatList);
    }

    // Save updated file
    SaveFileText(TextFormat("%s/README.md", exBasePath), mdTextUpdated);
    UnloadFileText(mdText);
    RL_FREE(mdTextUpdated);
    //------------------------------------------------------------------------------------------------

    // Edit: raylib.com/common/examples.js --> Update from collection
    // NOTE: Entries format: exampleEntry('⭐️☆☆☆' , 'core'    , 'basic_window'),
    //------------------------------------------------------------------------------------------------
    char *jsText = LoadFileText(TextFormat("%s/../common/examples.js", exWebPath));
    char *jsTextUpdated = (char *)RL_CALLOC(2*1024*1024, 1); // Updated examples.js copy, 2MB

    int jsListStartIndex = TextFindIndex(jsText, "//EXAMPLE_DATA_LIST_START");
    int jsListEndIndex = TextFindIndex(jsText, "//EXAMPLE_DATA_LIST_END");

    int jsIndex = 0;
    memcpy(jsTextUpdated, jsText, jsListStartIndex);
    jsIndex = sprintf(jsTextUpdated + jsListStartIndex, "//EXAMPLE_DATA_LIST_START\n");
    jsIndex += sprintf(jsTextUpdated + jsListStartIndex + jsIndex, "    var exampleData = [\n");

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
                // NOTE: Last line to add, special case to consider
                jsIndex += sprintf(jsTextUpdated + jsListStartIndex + jsIndex,
                    TextFormat("        exampleEntry('%s', '%s', '%s')];\n", stars, exCatList[x].category, exCatList[x].name + strlen(exCatList[x].category) + 1));
            }
            else
            {
                jsIndex += sprintf(jsTextUpdated + jsListStartIndex + jsIndex,
                    TextFormat("        exampleEntry('%s', '%s', '%s'),\n", stars, exCatList[x].category, exCatList[x].name + strlen(exCatList[x].category) + 1));
            }
        }

        UnloadExamplesData(exCatList);
    }

    // Add the remaining part of the original file
    memcpy(jsTextUpdated + jsListStartIndex + jsIndex, jsText + jsListEndIndex, strlen(jsText) - jsListEndIndex);

    // Save updated file
    SaveFileText(TextFormat("%s/../common/examples.js", exWebPath), jsTextUpdated);
    UnloadFileText(jsText);
    RL_FREE(jsTextUpdated);
    //------------------------------------------------------------------------------------------------

    return result;
}

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
        const char **lines = LoadTextLines(text, &lineCount);
        
        for (int i = 0; i < lineCount; i++)
        {
            // Basic validation for lines start categories
            if ((lines[i][0] != '#') && 
               ((lines[i][0] == 'c') ||      // core
                (lines[i][0] == 's') ||      // shapes, shaders
                (lines[i][0] == 't') ||      // textures, text
                (lines[i][0] == 'm') ||      // models
                (lines[i][0] == 'a') ||      // audio
                (lines[i][0] == 'o')))       // others
            {
                rlExampleInfo info = { 0 };
                int result = ParseExampleInfoLine(lines[i], &info);
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
    
    // Create required paths if they do not exist
    if (!DirectoryExists(GetDirectoryPath(dstPath)))
        MakeDirectory(GetDirectoryPath(dstPath));

    if ((srcFileData != NULL) && (srcDataSize > 0)) 
        result = SaveFileData(dstPath, srcFileData, srcDataSize);

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

// Load text lines
static char **LoadTextLines(const char *text, int *count)
{
    #define MAX_TEXT_LINES      512
    #define MAX_TEXT_LINE_LEN   256

    char **lines = (char **)RL_CALLOC(MAX_TEXT_LINES, sizeof(char *));
    for (int i = 0; i < MAX_TEXT_LINES; i++) lines[i] = (char *)RL_CALLOC(MAX_TEXT_LINE_LEN, 1);
    int textSize = (int)strlen(text);
    int k = 0;

    for (int i = 0, len = 0; (i < textSize) && (k < MAX_TEXT_LINES); i++)
    {
        if (text[i] == '\n')
        {
            strncpy(lines[k], &text[i - len], len);
            len = 0;
            k++;
        }
        else len++;
    }

    *count += k;
    return lines;
}

// Unload text lines
static void UnloadTextLines(char **lines)
{
    for (int i = 0; i < MAX_TEXT_LINES; i++) RL_FREE(lines[i]);
    RL_FREE(lines);
}

// raylib example line info parser
// Parses following line format: core;core_basic_window;⭐️☆☆☆;1.0;1.0;"Ray";@raysan5
static int ParseExampleInfoLine(const char *line, rlExampleInfo *entry)
{
    #define MAX_EXAMPLE_INFO_LINE_LEN   512
    
    char temp[MAX_EXAMPLE_INFO_LINE_LEN] = { 0 };
    strncpy(temp, line, MAX_EXAMPLE_INFO_LINE_LEN);
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
    if (tokens[5][0] == '"') tokens[5] += 1;
    if (tokens[5][strlen(tokens[5]) - 1] == '"') tokens[5][strlen(tokens[5]) - 1] = '\0';
    strcpy(entry->author, tokens[5]);
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

// Scan resource paths in example file
static char **ScanExampleResources(const char *filePath, int *resPathCount)
{
    #define MAX_RES_PATH_LEN    256

    char **paths = (char **)RL_CALLOC(REXM_MAX_RESOURCE_PATHS, sizeof(char **));
    for (int i = 0; i < REXM_MAX_RESOURCE_PATHS; i++) paths[i] = (char *)RL_CALLOC(MAX_RES_PATH_LEN, sizeof(char));

    int resCounter = 0;
    char *code = LoadFileText(filePath);

    if (code != NULL)
    {
        // Resources extensions to check
        const char *exts[] = { ".png", ".bmp", ".jpg", ".qoi", ".gif", ".raw", ".hdr", ".ttf", ".fnt", ".wav", ".ogg", ".mp3", ".flac", ".mod", ".qoa", ".qoa", ".obj", ".iqm", ".glb", ".m3d", ".vox", ".vs", ".fs" };
        const int extCount = sizeof(exts)/sizeof(exts[0]);

        char *ptr = code;
        while ((ptr = strchr(ptr, '"')) != NULL)
        {
            char *start = ptr + 1;
            char *end = strchr(start, '"');
            if (!end) break;

            int len = end - start;
            if ((len > 0) && (len < MAX_RES_PATH_LEN))
            {
                char buffer[MAX_RES_PATH_LEN] = { 0 };
                strncpy(buffer, start, len);
                buffer[len] = '\0';

                // Check for known extensions
                for (int i = 0; i < extCount; i++)
                {
                    if (IsFileExtension(buffer, exts[i]))
                    {
                        // Avoid duplicates
                        bool found = false;
                        for (int j = 0; j < resCounter; j++)
                        {
                            if (TextIsEqual(paths[j], buffer)) { found = true; break; }
                        }

                        if (!found && (resCounter < REXM_MAX_RESOURCE_PATHS))
                        {
                            strcpy(paths[resCounter], buffer);
                            resCounter++;
                        }

                        break;
                    }
                }
            }

            ptr = end + 1;
        }

        UnloadFileText(code);
    }

    *resPathCount = resCounter;
    return paths;
}

// Clear resource paths scanned
static void ClearExampleResources(char **resPaths)
{
    for (int i = 0; i < REXM_MAX_RESOURCE_PATHS; i++) RL_FREE(resPaths[i]);

    RL_FREE(resPaths);
}
