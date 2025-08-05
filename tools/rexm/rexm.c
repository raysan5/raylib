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

#define REXM_MAX_EXAMPLES               512
#define REXM_MAX_EXAMPLE_CATEGORIES     8

#define REXM_MAX_BUFFER_SIZE            (2*1024*1024)      // 2MB

#define REXM_MAX_RESOURCE_PATHS         256

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
static const char *exCollectionFilePath = "C:/GitHub/raylib/examples/examples_list.txt";

//const char *exBasePath = getenv("REXM_EXAMPLES_PATH");
//if (!exBasePath) exBasePath = "default/path";

//----------------------------------------------------------------------------------
// Module specific functions declaration
//----------------------------------------------------------------------------------
static int FileTextReplace(const char *fileName, const char *textLookUp, const char *textReplace);
static int FileCopy(const char *srcPath, const char *dstPath);
static int FileRename(const char *fileName, const char *fileRename);
static int FileRemove(const char *fileName);
static int FileMove(const char *srcPath, const char *dstPath);

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
                    for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES; i++) 
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
                            for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES; i++) 
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
                char *exColInfo = LoadFileText(exCollectionFilePath);
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
                char *exColInfo = LoadFileText(exCollectionFilePath);
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
            // Validate examples in collection (report results)
            // All examples in collection match all files requirements

            opCode = OP_VALIDATE;
        }
    }

    switch (opCode)
    {
        case OP_CREATE:     // Create: New example from template
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
        case OP_ADD:     // Add: Example from command-line input filename
        {
            // Add: raylib/examples/<category>/<category>_example_name.c
            if (opCode != 1) FileCopy(inFileName, TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));

            // Create: raylib/examples/<category>/<category>_example_name.png
            FileCopy(exTemplateScreenshot, TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName)); // WARNING: To be updated manually!

            // Copy: raylib/examples/<category>/resources/...
            // -----------------------------------------------------------------------------------------
            // Scan resources used in example to copy
            int resPathCount = 0;
            char **resPaths = ScanExampleResources(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName), &resPathCount);

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
            char *exCollectionList = LoadFileText(exCollectionFilePath);
            if (TextFindIndex(exCollectionList, exName) == -1) // Example not found
            {
                char *exCollectionListUpdated = (char *)RL_CALLOC(2*1024*1024, 1); // Updated list copy, 2MB
                
                // Add example to the main list, by category
                // by default add it last in the category list
                // NOTE: When populating to other files, lists are sorted by name
                int nextCategoryIndex = 0;
                if (strcmp(exCategory, "core") == 0) nextCategoryIndex = 1;
                else if (strcmp(exCategory, "shapes") == 0) nextCategoryIndex = 2;
                else if (strcmp(exCategory, "textures") == 0) nextCategoryIndex = 3;
                else if (strcmp(exCategory, "text") == 0) nextCategoryIndex = 4;
                else if (strcmp(exCategory, "models") == 0) nextCategoryIndex = 5;
                else if (strcmp(exCategory, "shaders") == 0) nextCategoryIndex = 6;
                else if (strcmp(exCategory, "audio") == 0) nextCategoryIndex = 7;
                else if (strcmp(exCategory, "others") == 0) nextCategoryIndex = -1; // Add to EOF

                // Get required example info from example file header (if provided)
                // NOTE: If no example info is provided (other than category/name), just using some default values
                char *exText = LoadFileText(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));

                rlExampleInfo exInfo = { 0 };
                strcpy(exInfo.category, exCategory);
                strcpy(exInfo.name, exName);
                
                // Get example difficulty stars
                char starsText[16] = { 0 };
                int starsIndex = TextFindIndex(exText, "★");
                if (starsIndex > 0) strncpy(starsText, exText + starsIndex, 3*4); // NOTE: Every UTF-8 star are 3 bytes
                else strcpy(starsText, "★☆☆☆");

                // Get example create with raylib version
                char verCreateText[4] = { 0 };
                int verCreateIndex = TextFindIndex(exText, "created with raylib "); // Version = index + 20
                if (verCreateIndex > 0) strncpy(verCreateText, exText + verCreateIndex + 20, 3);
                else strncpy(verCreateText, RAYLIB_VERSION, 3); // Only pick MAJOR.MINOR

                // Get example update with raylib version
                char verUpdateText[4] = { 0 };
                int verUpdateIndex = TextFindIndex(exText, "updated with raylib "); // Version = index + 20
                if (verUpdateIndex > 0) strncpy(verUpdateText, exText + verUpdateIndex + 20, 3);
                else strncpy(verUpdateText, RAYLIB_VERSION, 3); // Only pick MAJOR.MINOR

                // Get example creator and github user
                int authorIndex = TextFindIndex(exText, "Example contributed by "); // Author = index + 23
                int authorGitIndex = TextFindIndex(exText, "(@"); // Author GitHub user = index + 2
                if (authorIndex > 0)
                {
                    int authorNameLen = 0;
                    if (authorGitIndex > 0) authorNameLen = (authorGitIndex - 1) - (authorIndex + 23);
                    else
                    {
                        int authorNameEndIndex = TextFindIndex(exText + authorIndex, " and reviewed by Ramon Santamaria");
                        if (authorNameEndIndex == -1) authorNameEndIndex = TextFindIndex(exText + authorIndex, "\n");

                        authorNameLen = authorNameEndIndex - (authorIndex + 23);
                    }
                    strncpy(exInfo.author, exText + authorIndex + 23, authorNameLen);
                }
                else strcpy(exInfo.author, "<author_name>");
                if (authorGitIndex > 0)
                {
                    int authorGitEndIndex = TextFindIndex(exText + authorGitIndex, ")");
                    if (authorGitEndIndex > 0) strncpy(exInfo.authorGitHub, exText + authorGitIndex + 2, authorGitEndIndex - (authorGitIndex + 2));
                }
                else strcpy(exInfo.author, "<user_github>");
              
                // TODO: Verify copyright line
                // Copyright (c) <year_created>-<year_updated> <user_name> (@<user_github>)

                UnloadFileText(exText);
                
                if (nextCategoryIndex == -1)
                {
                    // Add example to collection at the EOF
                    int endIndex = (int)strlen(exCollectionList);
                    memcpy(exCollectionListUpdated, exCollectionList, endIndex);
                    sprintf(exCollectionListUpdated + endIndex, TextFormat("%s;%s;%s;%s;%s;\"%s\";@%s\n", 
                        exInfo.category, exInfo.name, starsText, verCreateText, verUpdateText, exInfo.author, exInfo.authorGitHub));
                }
                else
                {
                    // Add example to collection, at the end of the category list
                    int categoryIndex = TextFindIndex(exCollectionList, exCategories[nextCategoryIndex]);
                    memcpy(exCollectionListUpdated, exCollectionList, categoryIndex);
                    int textWritenSize = sprintf(exCollectionListUpdated + categoryIndex, TextFormat("%s;%s;%s;%s;%s;\"%s\";@%s\n",
                        exInfo.category, exInfo.name, starsText, verCreateText, verUpdateText, exInfo.author, exInfo.authorGitHub));
                    memcpy(exCollectionListUpdated + categoryIndex + textWritenSize, exCollectionList + categoryIndex, strlen(exCollectionList) - categoryIndex);
                }
                
                SaveFileText(exCollectionFilePath, exCollectionListUpdated);
                RL_FREE(exCollectionListUpdated);
            }
            else LOG("WARNING: ADD: Example is already on the collection\n");
            UnloadFileText(exCollectionList);
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
            // TODO: Avoid platform-specific .BAT file
            /*
            SET RAYLIB_PATH=C:\GitHub\raylib
            SET COMPILER_PATH=C:\raylib\w64devkit\bin
            ENV_SET PATH=$(COMPILER_PATH)
            SET MAKE=mingw32-make
            $(MAKE) -f Makefile.Web shaders/shaders_deferred_render PLATFORM=$(PLATFORM) -B

            //int putenv(char *string);   // putenv takes a string of the form NAME=VALUE
            //int setenv(const char *envname, const char *envval, int overwrite);
            //int unsetenv(const char *name); //unset variable
            putenv("RAYLIB_DIR=C:\\GitHub\\raylib");
            putenv("PATH=%PATH%;C:\\raylib\\w64devkit\\bin");
            setenv("RAYLIB_DIR", "C:\\GitHub\\raylib", 1);
            unsetenv("RAYLIB_DIR");
            getenv("RAYLIB_DIR");
            system(TextFormat("make -f Makefile.Web  %s/%s PLATFORM=PLATFORM_WEB -B", exCategory, exName));
            */
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
        case OP_RENAME:     // Rename
        {
            // NOTE: At this point provided values have been validated:
            // exName, exCategory, exRename, exRecategory
            if (strcmp(exCategory, exRecategory) == 0)
            {
                // Rename example on collection
                FileTextReplace(exCollectionFilePath, TextFormat("%s;%s", exCategory, exName), 
                    TextFormat("%s;%s", exRecategory, exRename));

                // Edit: Rename example code and screenshot files .c and .png
                rename(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.c", exBasePath, exCategory, exRename));
                rename(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.png", exBasePath, exCategory, exRename));

                // NOTE: Example resource files do not need to be changed...
                // unless the example is moved from one caegory to another

                // Edit: Rename example on required files
                FileTextReplace(TextFormat("%s/Makefile", exBasePath), exName, exRename);
                FileTextReplace(TextFormat("%s/Makefile.Web", exBasePath), exName, exRename);
                FileTextReplace(TextFormat("%s/README.md", exBasePath), exName, exRename);
                FileTextReplace(TextFormat("%s/../common/examples.js", exWebPath), exName, exRename);

                // Edit: Rename example project and solution
                rename(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName),
                    TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exRename));
                FileTextReplace(TextFormat("%s/../projects/VS2022/raylib.sln", exBasePath), exName, exRename);
            }
            else
            {
                // WARNING: Rename with change of category
                // TODO: Reorder collection to place renamed example at the end of category
                FileTextReplace(exCollectionFilePath, TextFormat("%s;%s", exCategory, exName), 
                    TextFormat("%s;%s", exRecategory, exRename));

                // TODO: Move example resources from <src_category>/resources to <dst_category>/resources
                // WARNING: Resources can be shared with other examples in the category

                // Edit: Rename example code file (copy and remove)
                FileCopy(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.c", exBasePath, exCategory, exRename));
                remove(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));
                // Edit: Rename example screenshot file (copy and remove)
                FileCopy(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.png", exBasePath, exCategory, exRename));
                remove(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName));

                // Edit: Update required files: Makefile, Makefile.Web, README.md, examples.js
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
        case OP_REMOVE:     // Remove
        {
            // Remove example from collection for files update
            //------------------------------------------------------------------------------------------------
            char *exCollectionList = LoadFileText(exCollectionFilePath);
            int exIndex = TextFindIndex(exCollectionList, TextFormat("%s;%s", exCategory, exName));
            if (exIndex > 0) // Example found
            {
                char *exCollectionListUpdated = (char *)RL_CALLOC(2*1024*1024, 1); // Updated list copy, 2MB

                memcpy(exCollectionListUpdated, exCollectionList, exIndex);
                int lineLen = 0;
                for (int i = exIndex; (exCollectionList[i] != '\n') && (exCollectionList[i] != '\0'); i++) lineLen++;
                // Remove line and copy the rest next
                memcpy(exCollectionListUpdated + exIndex, exCollectionList + exIndex + lineLen + 1, strlen(exCollectionList) - exIndex - lineLen);

                SaveFileText(exCollectionFilePath, exCollectionListUpdated);
                RL_FREE(exCollectionListUpdated);
            }
            else LOG("WARNING: REMOVE: Example not found in the collection\n");
            UnloadFileText(exCollectionList);
            //------------------------------------------------------------------------------------------------
           
            // Remove: raylib/examples/<category>/resources/..
            // WARNING: Some of those resources could be used by other examples,
            // just leave this process to manual update for now!
            // -----------------------------------------------------------------------------------------
            /*
            // Scan resources used in example to be removed
            int resPathCount = 0;
            char **resPaths = ScanExampleResources(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName), &resPathCount);

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
                            remove(TextFormat("%s/%s/%s", exBasePath, exCategory, resPathUpdated));
                            RL_FREE(resPathUpdated);
                        }
                    }
                    else remove(TextFormat("%s/%s/%s", exBasePath, exCategory, resPaths[r]));
                }
            }

            ClearExampleResources(resPaths);
            */
            // -----------------------------------------------------------------------------------------

            // Remove: raylib/examples/<category>/<category>_example_name.c
            // Remove: raylib/examples/<category>/<category>_example_name.png
            remove(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName));
            
            // Edit: Update required files: Makefile, Makefile.Web, README.md, examples.js
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
        case OP_VALIDATE:     // Validate: report and actions
        {
            // TODO: Validate examples in collection list [examples_list.txt] -> Source of truth!
            // Validate: raylib/examples/<category>/<category>_example_name.c        -> File exists?
            // Validate: raylib/examples/<category>/<category>_example_name.png      -> File exists?
            // Validate: raylib/examples/<category>/resources/..                     -> Example resources available?
            // Validate: raylib/examples/Makefile                                    -> Example listed?
            // Validate: raylib/examples/Makefile.Web                                -> Example listed?
            // Validate: raylib/examples/README.md                                   -> Example listed?
            // Validate: raylib/projects/VS2022/examples/<category>_example_name.vcxproj -> File exists?
            // Validate: raylib/projects/VS2022/raylib.sln                           -> Example listed?
            // Validate: raylib.com/common/examples.js                               -> Example listed?
            // Validate: raylib.com/examples/<category>/<category>_example_name.html -> File exists?
            // Validate: raylib.com/examples/<category>/<category>_example_name.data -> File exists?
            // Validate: raylib.com/examples/<category>/<category>_example_name.wasm -> File exists?
            // Validate: raylib.com/examples/<category>/<category>_example_name.js   -> File exists?

            // Additional validation elements
            // Validate: Example naming conventions: <category>/<category>_example_name
            // Validate: Duplicate entries in collection list
            // Validate: Example info (stars, author, github) missmatches with example content

            // After validation, update required files for consistency
            // Update files: Makefile, Makefile.Web, README.md, examples.js
            UpdateRequiredFiles();
            
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

    for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES; i++)
    {
        mkIndex += sprintf(mkTextUpdated + mkListStartIndex + mkIndex, TextFormat("%s = \\\n", TextToUpper(exCategories[i])));

        int exCollectionCount = 0;
        rlExampleInfo *exCollection = LoadExamplesData(exCollectionFilePath, exCategories[i], true, &exCollectionCount);

        for (int x = 0; x < exCollectionCount - 1; x++) mkIndex += sprintf(mkTextUpdated + mkListStartIndex + mkIndex, TextFormat("    %s/%s \\\n", exCollection[x].category, exCollection[x].name));
        mkIndex += sprintf(mkTextUpdated + mkListStartIndex + mkIndex, TextFormat("    %s/%s\n\n", exCollection[exCollectionCount - 1].category, exCollection[exCollectionCount - 1].name));

        UnloadExamplesData(exCollection);
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
    for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES - 1; i++)
    {
        mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, TextFormat("%s = \\\n", TextToUpper(exCategories[i])));

        int exCollectionCount = 0;
        rlExampleInfo *exCollection = LoadExamplesData(exCollectionFilePath, exCategories[i], true, &exCollectionCount);

        for (int x = 0; x < exCollectionCount - 1; x++) mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, TextFormat("    %s/%s \\\n", exCollection[x].category, exCollection[x].name));
        mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, TextFormat("    %s/%s\n\n", exCollection[exCollectionCount - 1].category, exCollection[exCollectionCount - 1].name));

        UnloadExamplesData(exCollection);
    }

    // Add examples individual targets, considering every example resources
    // Some required makefile code...
    mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, "# Default target entry\n");
    mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, "all: $(CORE) $(SHAPES) $(TEXT) $(TEXTURES) $(MODELS) $(SHADERS) $(AUDIO)\n\n");
    mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, "core: $(CORE)\n");
    mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, "shapes: $(SHAPES)\n");
    mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, "textures: $(TEXTURES)\n");
    mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, "text: $(TEXT)\n");
    mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, "models: $(MODELS)\n");
    mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, "shaders: $(SHADERS)\n");
    mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, "audio: $(AUDIO)\n\n");

    // NOTE: We avoid the "others" category on web building
    for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES - 1; i++)
    {
        mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, TextFormat("# Compile %s examples\n", TextToUpper(exCategories[i])));

        int exCollectionCount = 0;
        rlExampleInfo *exCollection = LoadExamplesData(exCollectionFilePath, exCategories[i], true, &exCollectionCount);

        for (int x = 0; x < exCollectionCount; x++)
        {
            // Scan resources used in example to list
            int resPathCount = 0;
            char **resPaths = ScanExampleResources(TextFormat("%s/%s/%s.c", exBasePath, exCollection[x].category, exCollection[x].name), &resPathCount);

            if (resPathCount > 0)
            {
                /*
                // WARNING: Compilation line starts with [TAB]
                shaders/shaders_vertex_displacement: shaders/shaders_vertex_displacement.c
                    $(CC) -o $@$(EXT) $< $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM) \
                    --preload-file shaders/resources/shaders/glsl100/vertex_displacement.vs@resources/shaders/glsl100/vertex_displacement.vs \
                    --preload-file shaders/resources/shaders/glsl330/vertex_displacement.vs@resources/shaders/glsl330/vertex_displacement.vs \
                    --preload-file shaders/resources/shaders/glsl100/vertex_displacement.fs@resources/shaders/glsl100/vertex_displacement.fs \
                    --preload-file shaders/resources/shaders/glsl330/vertex_displacement.fs@resources/shaders/glsl330/vertex_displacement.fs
                */
                mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, 
                    TextFormat("%s/%s: %s/%s.c\n", exCollection[x].category, exCollection[x].name, exCollection[x].category, exCollection[x].name));
                mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, "	$(CC) -o $@$(EXT) $< $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM) \\\n");

                for (int r = 0; r < resPathCount; r++)
                {
                    // WARNING: Special case to consider: shaders, resource paths could use conditions: "glsl%i"
                    // In this case, we focus on web building for: glsl100
                    if (TextFindIndex(resPaths[r], "glsl%i") > -1)
                    {
                        char *resPathUpdated = TextReplace(resPaths[r], "glsl%i", "glsl100");
                        memset(resPaths[r], 0, 256);
                        strcpy(resPaths[r], resPathUpdated);
                        RL_FREE(resPathUpdated);
                    }

                    if (r < (resPathCount - 1))
                    {
                        mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex,
                            TextFormat("    --preload-file %s/%s@%s \\\n", exCollection[x].category, resPaths[r], resPaths[r]));
                    }
                    else
                    {
                        mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex,
                            TextFormat("    --preload-file %s/%s@%s\n\n", exCollection[x].category, resPaths[r], resPaths[r]));
                    }
                }
            }
            else  // Example does not require resources
            {
                /*
                // WARNING: Compilation line starts with [TAB]
                core/core_2d_camera: core/core_2d_camera.c
                    $(CC) -o $@$(EXT) $< $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM)
                */
                mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, 
                    TextFormat("%s/%s: %s/%s.c\n", exCollection[x].category, exCollection[x].name, exCollection[x].category, exCollection[x].name));
                mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, "	$(CC) -o $@$(EXT) $< $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM)\n\n");
            }

            ClearExampleResources(resPaths);
        }

        UnloadExamplesData(exCollection);
    }

    // Add the remaining part of the original file
    memcpy(mkwTextUpdated + mkwListStartIndex + mkwIndex - 1, mkwText + mkwListEndIndex, strlen(mkwText) - mkwListEndIndex);

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

    int mdListStartIndex = TextFindIndex(mdText, "## EXAMPLES COLLECTION");

    int mdIndex = 0;
    memcpy(mdTextUpdated, mdText, mdListStartIndex);

    int exCollectionFullCount = 0;
    rlExampleInfo *exCollectionFull = LoadExamplesData(exCollectionFilePath, "ALL", false, &exCollectionFullCount);
    UnloadExamplesData(exCollectionFull);

    mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, TextFormat("## EXAMPLES COLLECTION [TOTAL: %i]\n\n", exCollectionFullCount));

    // NOTE: We keep a global examples counter
    for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES; i++)
    {
        int exCollectionCount = 0;
        rlExampleInfo *exCollection = LoadExamplesData(exCollectionFilePath, exCategories[i], false, &exCollectionCount); 

        // Every category includes some introductory text, as it is quite short, just copying it here
        if (i == 0)         // "core"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, TextFormat("\n### category: core [%i]\n\n", exCollectionCount));
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib[core](../src/rcore.c) platform functionality like window creation, inputs, drawing modes and system functionality.\n\n");
        }
        else if (i == 1)    // "shapes"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, TextFormat("\n### category: shapes [%i]\n\n", exCollectionCount));
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib shapes drawing functionality, provided by raylib [shapes](../src/rshapes.c) module.\n\n");
        }
        else if (i == 2)    // "textures"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, TextFormat("\n### category: textures [%i]\n\n", exCollectionCount));
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib textures functionality, including image/textures loading/generation and drawing, provided by raylib [textures](../src/rtextures.c) module.\n\n");
        }
        else if (i == 3)    // "text"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, TextFormat("\n### category: text [%i]\n\n", exCollectionCount));
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib text functionality, including sprite fonts loading/generation and text drawing, provided by raylib [text](../src/rtext.c) module.\n\n");
        }
        else if (i == 4)    // "models"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, TextFormat("\n### category: models [%i]\n\n", exCollectionCount));
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib models functionality, including models loading/generation and drawing, provided by raylib [models](../src/rmodels.c) module.\n\n");
        }
        else if (i == 5)    // "shaders"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, TextFormat("\n### category: shaders [%i]\n\n", exCollectionCount));
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib shaders functionality, including shaders loading, parameters configuration and drawing using them (model shaders and postprocessing shaders). This functionality is directly provided by raylib [rlgl](../src/rlgl.c) module.\n\n");
        }
        else if (i == 6)    // "audio"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, TextFormat("\n### category: audio [%i]\n\n", exCollectionCount));
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib audio functionality, including sound/music loading and playing. This functionality is provided by raylib [raudio](../src/raudio.c) module. Note this module can be used standalone independently of raylib.\n\n");
        }
        else if (i == 7)    // "others"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, TextFormat("\n### category: others [%i]\n\n", exCollectionCount));
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples showing raylib misc functionality that does not fit in other categories, like standalone modules usage or examples integrating external libraries.\n\n");
        }

        // Table header required
        mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, "|  example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |\n");
        mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, "|-----------|--------|:-------------------:|:------------------:|:-----------------------:|:----------------------|\n");

        for (int x = 0; x < exCollectionCount; x++)
        {
            char stars[16] = { 0 };
            for (int s = 0; s < 4; s++)
            {
                if (s < exCollection[x].stars) strcpy(stars + 3*s, "⭐️");
                else strcpy(stars + 3*s, "☆");
            }

            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                TextFormat("| [%s](%s/%s.c) | <img src=\"%s/%s.png\" alt=\"%s\" width=\"80\"> | %s | %.1f | %.1f | [%s](https://github.com/%s) |\n", 
                    exCollection[x].name, exCollection[x].category, exCollection[x].name, exCollection[x].category, exCollection[x].name, exCollection[x].name,
                    stars, exCollection[x].verCreated, exCollection[x].verUpdated, exCollection[x].author, exCollection[x].authorGitHub + 1));
        }

        UnloadExamplesData(exCollection);
    }

    mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
        "\nSome example missing? As always, contributions are welcome, feel free to send new examples!\n");
    mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
        "Here is an[examples template](examples_template.c) with instructions to start with!\n");

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
    for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES - 1; i++)
    {
        int exCollectionCount = 0;
        rlExampleInfo *exCollection = LoadExamplesData(exCollectionFilePath, exCategories[i], false, &exCollectionCount); 
        for (int x = 0; x < exCollectionCount; x++)
        {
            char stars[16] = { 0 };
            for (int s = 0; s < 4; s++)
            {
                if (s < exCollection[x].stars) strcpy(stars + 3*s, "⭐️");
                else strcpy(stars + 3*s, "☆");
            }

            if ((i == 6) && (x == (exCollectionCount - 1)))
            {
                // NOTE: Last line to add, special case to consider
                jsIndex += sprintf(jsTextUpdated + jsListStartIndex + jsIndex,
                    TextFormat("        exampleEntry('%s', '%s', '%s')];\n", stars, exCollection[x].category, exCollection[x].name + strlen(exCollection[x].category) + 1));
            }
            else
            {
                jsIndex += sprintf(jsTextUpdated + jsListStartIndex + jsIndex,
                    TextFormat("        exampleEntry('%s', '%s', '%s'),\n", stars, exCollection[x].category, exCollection[x].name + strlen(exCollection[x].category) + 1));
            }
        }

        UnloadExamplesData(exCollection);
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
        char **lines = LoadTextLines(text, &lineCount);
        
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
    
        UnloadTextLines(lines);
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

// Move file from one directory to another
// NOTE: If dst directories do not exists they are created
static int FileMove(const char *srcPath, const char *dstPath)
{
    int result = 0;

    if (FileExists(srcPath))
    {
        FileCopy(srcPath, dstPath);
        remove(srcPath);
    }

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
// WARNING: Supported resource file extensions is hardcoded by used file types
// but new examples could require other file extensions to be added,
// maybe it should look for '.xxx")' patterns instead
static char **ScanExampleResources(const char *filePath, int *resPathCount)
{
    #define REXM_MAX_RESOURCE_PATH_LEN    256

    char **paths = (char **)RL_CALLOC(REXM_MAX_RESOURCE_PATHS, sizeof(char **));
    for (int i = 0; i < REXM_MAX_RESOURCE_PATHS; i++) paths[i] = (char *)RL_CALLOC(REXM_MAX_RESOURCE_PATH_LEN, sizeof(char));

    int resCounter = 0;
    char *code = LoadFileText(filePath);

    if (code != NULL)
    {
        // Resources extensions to check
        const char *exts[] = { ".png", ".bmp", ".jpg", ".qoi", ".gif", ".raw", ".hdr", ".ttf", ".fnt", ".wav", ".ogg", ".mp3", ".flac", ".mod", ".qoa", ".qoa", ".obj", ".iqm", ".glb", ".m3d", ".vox", ".vs", ".fs", ".txt" };
        const int extCount = sizeof(exts)/sizeof(char *);

        char *ptr = code;
        while ((ptr = strchr(ptr, '"')) != NULL)
        {
            char *start = ptr + 1;
            char *end = strchr(start, '"');
            if (!end) break;

            int len = end - start;
            if ((len > 0) && (len < REXM_MAX_RESOURCE_PATH_LEN))
            {
                char buffer[REXM_MAX_RESOURCE_PATH_LEN] = { 0 };
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
