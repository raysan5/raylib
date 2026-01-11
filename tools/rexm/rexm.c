/*******************************************************************************************
*
*   rexm [raylib examples manager] - A simple and easy-to-use raylib examples collection manager
*
*   Supported processes:
*    - create <new_example_name>
*    - add <example_name>
*    - rename <old_examples_name> <new_example_name>
*    - remove <example_name>
*    - build <example_name>
*    - test <example_name>
*    - validate                 // All examples
*    - update                   // All examples
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
*    - ...
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2025-2026 Ramon Santamaria (@raysan5)
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

#include <stdio.h>      // Required for: rename(), remove()
#include <string.h>     // Required for: strcmp(), strcpy()
#include <stdlib.h>     // Required for: NULL, calloc(), free()

#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO) //&& defined(_DEBUG)
    #define LOG(...) printf("REXM: "__VA_ARGS__)
#else
    #define LOG(...)
#endif

#define REXM_MAX_EXAMPLES               512
#define REXM_MAX_EXAMPLE_CATEGORIES     8

#define REXM_MAX_BUFFER_SIZE            (2*1024*1024)      // 2MB

#define REXM_MAX_RESOURCE_PATHS         256

// Create local commit with changes on example renaming
//#define RENAME_AUTO_COMMIT_CREATION

#if defined(__GNUC__) // GCC and Clang
    #pragma GCC diagnostic push
    // Avoid GCC/Clang complaining about sprintf() second parameter not being a string literal (being TextFormat())
    #pragma GCC diagnostic ignored "-Wformat-security"
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// raylib example info struct
typedef struct {
    char category[16];      // Example category: core, shapes, textures, text, models, shaders, audio, [others]
    char name[128];         // Example name: <category>_name_part
    int stars;              // Example stars count: ★☆☆☆
    char verCreated[12];    // Example raylib creation version
    char verUpdated[12];    // Example raylib last update version
    int yearCreated;        // Example year created
    int yearReviewed;       // Example year reviewed
    char author[64];        // Example author
    char authorGitHub[64];  // Example author, GitHub user name

    int status;             // Example validation status flags
    int resCount;           // Example resources counter
    char **resPaths;        // Example resources paths (MAX: 256)
} rlExampleInfo;

// Automated testing data
typedef struct {
    int buildwarns;         // Example building warnings count (by GCC compiler)
    int warnings;           // Example run output log warnings count
    int status;             // Example run testing status flags (>0 = FAILS)
} rlExampleTesting;

// Validation status for a single example
typedef enum {
    VALID_OK                    = 0,        // All required files and entries are present
    VALID_MISSING_C             = 1 << 0,   // Missing .c source file
    VALID_MISSING_PNG           = 1 << 1,   // Missing screenshot .png
    VALID_INVALID_PNG           = 1 << 2,   // Invalid screenshot .png (using template one)
    VALID_MISSING_RESOURCES     = 1 << 3,   // Missing resources listed in the code
    VALID_MISSING_VCXPROJ       = 1 << 4,   // Missing Visual Studio .vcxproj file
    VALID_NOT_IN_VCXSOL         = 1 << 5,   // Project not included in solution file
    VALID_NOT_IN_MAKEFILE       = 1 << 6,   // Not listed in Makefile
    VALID_NOT_IN_MAKEFILE_WEB   = 1 << 7,   // Not listed in Makefile.Web
    VALID_NOT_IN_README         = 1 << 8,   // Not listed in README.md
    VALID_NOT_IN_JS             = 1 << 9,   // Not listed in examples.js
    VALID_INCONSISTENT_INFO     = 1 << 10,  // Inconsistent info between collection and example header (stars, author...)
    VALID_MISSING_WEB_OUTPUT    = 1 << 11,  // Missing .html/.data/.wasm/.js
    VALID_MISSING_WEB_METADATA  = 1 << 12,  // Missing .html example metadata
    VALID_INVALID_CATEGORY      = 1 << 13,  // Not a recognized category
    VALID_UNKNOWN_ERROR         = 1 << 14   // Unknown failure case (fallback)
} rlExampleValidationStatus;

typedef enum {
    TESTING_OK                  = 0,        // All automated testing ok
    TESTING_FAIL_INIT           = 1 << 0,   // Initialization (InitWindow())    -> "INFO: DISPLAY: Device initialized successfully"
    TESTING_FAIL_CLOSE          = 1 << 1,   // Closing (CloseWindow())          -> "INFO: Window closed successfully"
    TESTING_FAIL_ASSETS         = 1 << 2,   // Assets loading (WARNING: FILE:)  -> "WARNING: FILEIO:"
    TESTING_FAIL_RLGL           = 1 << 3,   // OpenGL-wrapped initialization    -> "INFO: RLGL: Default OpenGL state initialized successfully"
    TESTING_FAIL_PLATFORM       = 1 << 4,   // Platform initialization          -> "INFO: PLATFORM: DESKTOP (GLFW - Win32): Initialized successfully"
    TESTING_FAIL_FONT           = 1 << 5,   // Font deefault initialization     -> "INFO: FONT: Default font loaded successfully (224 glyphs)"
    TESTING_FAIL_TIMER          = 1 << 6,   // Timer initialization             -> "INFO: TIMER: Target time per frame: 16.667 milliseconds"
    TESTING_FAIL_OTHER          = 1 << 7,   // Other types of warnings (WARNING:)
} rlExampleTestingStatus;

// Example management operations
typedef enum {
    OP_NONE     = 0,        // No process to do
    OP_CREATE   = 1,        // Create new example, using default template
    OP_ADD      = 2,        // Add existing examples (hopefully following template)
    OP_RENAME   = 3,        // Rename existing example
    OP_REMOVE   = 4,        // Remove existing example
    OP_VALIDATE = 5,        // Validate examples, using [examples_list.txt] as main source by default
    OP_UPDATE   = 6,        // Validate and update required examples (as far as possible): ALL
    OP_BUILD    = 7,        // Build example(s) for desktop and web, copy web output - Multiple examples supported
    OP_TEST     = 8,        // Test example(s), checking output log "WARNING" - Multiple examples supported
    OP_TESTLOG  = 9,        // Process available examples logs to generate report
} rlExampleOperation;

static const char *exCategories[REXM_MAX_EXAMPLE_CATEGORIES] = { "core", "shapes", "textures", "text", "models", "shaders", "audio", "others" };

// Paths required for examples management
// NOTE: Paths can be provided with environment variables
static const char *exBasePath = NULL;           // Env: REXM_EXAMPLES_BASE_PATH
static const char *exWebPath = NULL;            // Env: REXM_EXAMPLES_WEB_PATH
static const char *exTemplateFilePath = NULL;   // Env: REXM_EXAMPLES_TEMPLATE_FILE_PATH
static const char *exTemplateScreenshot = NULL; // Env: REXM_EXAMPLES_TEMPLATE_SCREENSHOT_PATH
static const char *exCollectionFilePath = NULL; // Env: REXM_EXAMPLES_COLLECTION_FILE_PATH
static const char *exVSProjectSolutionFile = NULL; // Env REXM_EXAMPLES_VS2022_SLN_FILE

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
// Update required files from examples collection
// UPDATES: Makefile, Makefile.Web, README.md, examples.js
static int UpdateRequiredFiles(void);

// Load examples collection information
// NOTE 1: Load by category: "ALL", "core", "shapes", "textures", "text", "models", "shaders", others"
// NOTE 2: Sort examples list on request flag
static rlExampleInfo *LoadExampleData(const char *filter, bool sort, int *exCount);
static void UnloadExampleData(rlExampleInfo *exInfo);

// Load example info from file header
static rlExampleInfo *LoadExampleInfo(const char *exFileName);
static void UnloadExampleInfo(rlExampleInfo *exInfo);

// raylib example line info parser
// Parses following line format: core/core_basic_window;★☆☆☆;1.0;1.0;"Ray"/@raysan5
static int ParseExampleInfoLine(const char *line, rlExampleInfo *entry);

// Sort array of strings by name
// WARNING: items[] pointers are reorganized
static void SortExampleByName(rlExampleInfo *items, int count);

// Scan resource paths in example file
static char **LoadExampleResourcePaths(const char *filePath, int *resPathCount);

// Clear resource paths scanned
static void UnloadExampleResourcePaths(char **resPaths);

// Add/remove VS project (.vcxproj) tofrom existing VS solution (.sln)
static int AddVSProjectToSolution(const char *slnFile, const char *projFile, const char *category);
static int RemoveVSProjectFromSolution(const char *slnFile, const char *exName);

// Generate unique UUID v4 string
// Output format: {9A2F48CC-0DA8-47C0-884E-02E37F9BE6C1}
// TODO: It seems generated UUID is not valid for VS2022
static const char *GenerateUUIDv4(void);

// Update source code header and comments metadata
static void UpdateSourceMetadata(const char *exSrcPath, const rlExampleInfo *info);
// Update generated Web example .html file metadata
static void UpdateWebMetadata(const char *exHtmlPath, const char *exFilePath);

// Check if text string is in a list of strings and get index, -1 if not found
static int GetTextListIndex(const char *text, const char **list, int listCount);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    SetTraceLogLevel(LOG_NONE);

    // Path values can be configured with environment variables
    exBasePath = getenv("REXM_EXAMPLES_BASE_PATH");
    exWebPath = getenv("REXM_EXAMPLES_WEB_PATH");
    exTemplateFilePath = getenv("REXM_EXAMPLES_TEMPLATE_FILE_PATH");
    exTemplateScreenshot = getenv("REXM_EXAMPLES_TEMPLATE_SCREENSHOT_PATH");
    exCollectionFilePath = getenv("REXM_EXAMPLES_COLLECTION_FILE_PATH");
    exVSProjectSolutionFile = getenv("REXM_EXAMPLES_VS2022_SLN_FILE");

#if defined(_WIN32)
    if (!exBasePath) exBasePath = "C:/GitHub/raylib/examples";
    if (!exWebPath) exWebPath = "C:/GitHub/raylib.com/examples";
    if (!exTemplateFilePath) exTemplateFilePath = "C:/GitHub/raylib/examples/examples_template.c";
    if (!exTemplateScreenshot) exTemplateScreenshot = "C:/GitHub/raylib/examples/examples_template.png";
    if (!exCollectionFilePath) exCollectionFilePath = "C:/GitHub/raylib/examples/examples_list.txt";
    if (!exVSProjectSolutionFile) exVSProjectSolutionFile = "C:/GitHub/raylib/projects/VS2022/raylib.sln";
#else
    // Cross-platform relative fallbacks (run from tools/rexm directory)
    if (!exBasePath) exBasePath = "../../examples";
    if (!exWebPath) exWebPath = "../../raylib.com/examples";
    if (!exTemplateFilePath) exTemplateFilePath = "../../examples/examples_template.c";
    if (!exTemplateScreenshot) exTemplateScreenshot = "../../examples/examples_template.png";
    if (!exCollectionFilePath) exCollectionFilePath = "../../examples/examples_list.txt";
    if (!exVSProjectSolutionFile) exVSProjectSolutionFile = "../../projects/VS2022/raylib.sln";
#endif

    char inFileName[1024] = { 0 };  // Example input filename (to be added)

    char exName[64] = { 0 };        // Example name, without extension: core_basic_window
    char exCategory[32] = { 0 };    // Example category: core
    char exRecategory[32] = { 0 };  // Example re-name category: shapes
    char exRename[64] = { 0 };      // Example re-name, without extension

    char *exBuildList[256] = { 0 }; // Example build list for: ALL, <category>, single-example
    int exBuildListCount = 0;       // Example build list file count

    int opCode = OP_NONE;           // Operation code: 0-None(Help), 1-Create, 2-Add, 3-Rename, 4-Remove
    bool showUsage = false;         // Flag to show usage help
    bool verbose = false;           // Flag for verbose log info

    // Command-line usage mode: command args processing
    //--------------------------------------------------------------------------------------
    if (argc > 1)
    {
        // Supported commands:
        //    create <new_example_name>     : Creates an empty example, from internal template
        //    add <example_name>            : Add existing example, category extracted from name
        //    rename <old_examples_name> <new_example_name> : Rename an existing example
        //    remove <example_name>         : Remove an existing example
        //    build <example_name>          : Build example for Desktop and Web platforms
        //    validate                      : Validate examples collection, generates report
        //    update                        : Validate and update examples collection, generates report

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
                    if (FileExists(argv[2]))
                    {
                        // Security checks for file name to verify category is included
                        int catIndex = TextFindIndex(GetFileName(argv[2]), "_");
                        if (catIndex > 3)
                        {
                            char cat[12] = { 0 };
                            strncpy(cat, GetFileName(argv[2]), catIndex);
                            bool catFound = false;
                            for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES; i++)
                            {
                                if (TextIsEqual(cat, exCategories[i])) { catFound = true; break; }
                            }

                            if (catFound)
                            {
                                strcpy(inFileName, argv[2]); // Register filename for addition
                                strcpy(exName, GetFileNameWithoutExt(inFileName)); // Register example name
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
            else if (argc == 3) LOG("WARNING: No new filename provided to be renamed\n");
            else if (argc > 4) LOG("WARNING: Too many arguments provided\n");
            else
            {
                // Verify example exists in collection to be removed
                char *exColInfo = LoadFileText(exCollectionFilePath);
                if (TextFindIndex(exColInfo, argv[2]) != -1) // Example in the collection
                {
                    // Security checks for new file name to verify category is included
                    int newCatIndex = TextFindIndex(argv[3], "_");
                    if (newCatIndex > 3)
                    {
                        char cat[12] = { 0 };
                        strncpy(cat, argv[3], newCatIndex);
                        bool newCatFound = false;
                        for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES; i++)
                        {
                            if (TextIsEqual(cat, exCategories[i])) { newCatFound = true; break; }
                        }

                        if (newCatFound)
                        {
                            strcpy(exName, argv[2]);    // Register example name
                            strncpy(exCategory, exName, TextFindIndex(exName, "_"));
                            strcpy(exRename, argv[3]);
                            strncpy(exRecategory, exRename, TextFindIndex(exRename, "_"));
                            opCode = OP_RENAME;
                        }
                        else LOG("WARNING: Example new category is not valid\n");
                    }
                    else LOG("WARNING: Example new name does not include category\n");
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
        else if (strcmp(argv[1], "update") == 0)
        {
            // Validate and update examples in collection
            // All examples in collection match all files requirements

            opCode = OP_UPDATE;
        }
        else if ((strcmp(argv[1], "build") == 0) || (strcmp(argv[1], "test") == 0) || (strcmp(argv[1], "testlog") == 0))
        {
            // Build/Test example(s) for PLATFORM_DESKTOP and PLATFORM_WEB
            // NOTE: Build outputs to default directory, usually where the .c file is located,
            // to avoid issues with copying resources (at least on Desktop)
            // Web build files (.html, .wasm, .js, .data) are copied to raylib.com/examples repo
            // Check for valid upcoming argument
            if (argc == 2) LOG("WARNING: No example name/category provided\n");
            else if (argc > 3) LOG("WARNING: Too many arguments provided\n");
            else
            {
                // Support building/testing not only individual examples but multiple: ALL/<category>
                int exBuildListInfoCount = 0;
                rlExampleInfo *exBuildListInfo = LoadExampleData(argv[2], false, &exBuildListInfoCount);

                for (int i = 0; i < exBuildListInfoCount; i++)
                {
                    if (!TextIsEqual(exBuildListInfo[i].category, "others"))
                    {
                        exBuildList[exBuildListCount] = (char *)RL_CALLOC(256, sizeof(char));
                        strcpy(exBuildList[exBuildListCount], exBuildListInfo[i].name);
                        exBuildListCount++;
                    }
                }

                UnloadExampleData(exBuildListInfo);

                if (exBuildListCount == 0) LOG("WARNING: BUILD: Example requested not available in the collection\n");
                else
                {
                    if (strcmp(argv[1], "build") == 0) opCode = OP_BUILD;
                    else if (strcmp(argv[1], "test") == 0) opCode = OP_TEST;
                    else if (strcmp(argv[1], "testlog") == 0) opCode = OP_TESTLOG;
                }
            }
        }

        // Process command line options arguments
        for (int i = 1; i < argc; i++)
        {
            if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) showUsage = true;
            else if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--verbose") == 0)) verbose = true;
        }
    }

    // Command-line usage mode: command execution
    switch (opCode)
    {
        case OP_CREATE:     // Create: New example from template
        {
            LOG("INFO: Command requested: CREATE\n");
            LOG("INFO: Example to create: %s\n", exName);

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
            if (opCode != OP_CREATE) LOG("INFO: Command requested: ADD\n");
            LOG("INFO: Example file to be added: %s\n", inFileName);

            // Add: raylib/examples/<category>/<category>_example_name.c
            if (opCode != OP_CREATE) FileCopy(inFileName, TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));

            // Create: raylib/examples/<category>/<category>_example_name.png
            if (FileExists(TextFormat("%s/%s.png", GetDirectoryPath(inFileName), exName)))
            {
                LOG("INFO: [%s] Copying file screenshot...\n", GetFileName(inFileName));
                FileCopy(TextFormat("%s/%s.png", GetDirectoryPath(inFileName), exName),
                    TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName));
            }
            else // No screenshot available next to source file
            {
                // Copy screenshot template
                LOG("WARNING: [%s] No screenshot found, using placeholder screenshot\n", GetFileName(inFileName));
                FileCopy(exTemplateScreenshot, TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName));
            }

            // Copy: raylib/examples/<category>/resources/...
            // -----------------------------------------------------------------------------------------
            // Scan resources used in example to copy
            // NOTE: resources path will be relative to example source file directory
            int resPathCount = 0;
            LOG("INFO: [%s] Scanning file for resources...\n", GetFileName(inFileName));
            char **resPaths = LoadExampleResourcePaths(TextFormat("%s/%s.c", GetDirectoryPath(inFileName), exName), &resPathCount);

            if (resPathCount > 0)
            {
                LOG("INFO: [%s] Required resources found: %i\n", GetFileName(inFileName), resPathCount);

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

                            LOG("INFO: [%s] Resource required [%i/%i]: %s\n", GetFileName(inFileName), r, resPathCount, resPathUpdated);

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
                        LOG("INFO: [%s] Resource required [%i/%i]: %s\n", GetFileName(inFileName), r, resPathCount, resPaths[r]);

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

            UnloadExampleResourcePaths(resPaths);
            // -----------------------------------------------------------------------------------------

            // Add example to the collection list, if not already there
            // NOTE: Required format: shapes;shapes_basic_shapes;★☆☆☆;1.0;4.2;2014;2026;"Ray";@raysan5
            //------------------------------------------------------------------------------------------------
            char *exCollectionList = LoadFileText(exCollectionFilePath);
            if (TextFindIndex(exCollectionList, exName) == -1) // Example not found
            {
                LOG("INFO: [%s] Adding example to collection list (%s)\n", GetFileName(inFileName), exCategory);

                char *exCollectionListUpdated = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1); // Updated list copy, 2MB

                // Add example to the main list, by category
                // by default add it last in the category list
                // NOTE: When populating to other files, lists are sorted by name
                int nextCategoryIndex = 0;
                if (TextIsEqual(exCategory, "core")) nextCategoryIndex = 1;
                else if (TextIsEqual(exCategory, "shapes")) nextCategoryIndex = 2;
                else if (TextIsEqual(exCategory, "textures")) nextCategoryIndex = 3;
                else if (TextIsEqual(exCategory, "text")) nextCategoryIndex = 4;
                else if (TextIsEqual(exCategory, "models")) nextCategoryIndex = 5;
                else if (TextIsEqual(exCategory, "shaders")) nextCategoryIndex = 6;
                else if (TextIsEqual(exCategory, "audio")) nextCategoryIndex = 7;
                else if (TextIsEqual(exCategory, "others")) nextCategoryIndex = -1; // Add to EOF

                // Get required example info from example file header (if provided)

                // NOTE: Load example info from provided example header
                rlExampleInfo *exInfo = LoadExampleInfo(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));

                LOG("INFO: [%s] Example info: \n", GetFileName(inFileName));
                LOG("      > Author: %s (@%s)\n", exInfo->author, exInfo->authorGitHub);
                LOG("      > Stars: %i\n", exInfo->stars);
                LOG("      > Version-Update: %s-%s\n", exInfo->verCreated, exInfo->verUpdated);
                LOG("      > Created-Reviewed: %i-%i\n", exInfo->yearCreated, exInfo->yearReviewed);

                // Get example difficulty stars text
                char starsText[16] = { 0 };
                for (int i = 0; i < 4; i++)
                {
                    // NOTE: Every UTF-8 star are 3 bytes
                    if (i < exInfo->stars) strcpy(starsText + 3*i, "★");
                    else strcpy(starsText + 3*i, "☆");
                }

                if (nextCategoryIndex == -1)
                {
                    // Add example to collection at the EOF
                    int endIndex = (int)strlen(exCollectionList);
                    memcpy(exCollectionListUpdated, exCollectionList, endIndex);
                    sprintf(exCollectionListUpdated + endIndex, TextFormat("%s;%s;%s;%s;%s;%i;%i;\"%s\";@%s\n",
                        exInfo->category, exInfo->name, starsText, exInfo->verCreated, exInfo->verUpdated, exInfo->yearCreated, exInfo->yearReviewed, exInfo->author, exInfo->authorGitHub));
                }
                else
                {
                    // Add example to collection, at the end of the category list
                    int categoryIndex = TextFindIndex(exCollectionList, exCategories[nextCategoryIndex]);
                    memcpy(exCollectionListUpdated, exCollectionList, categoryIndex);
                    int textWritenSize = sprintf(exCollectionListUpdated + categoryIndex, TextFormat("%s;%s;%s;%s;%s;%i;%i;\"%s\";@%s\n",
                        exInfo->category, exInfo->name, starsText, exInfo->verCreated, exInfo->verUpdated, exInfo->yearCreated, exInfo->yearReviewed, exInfo->author, exInfo->authorGitHub));
                    memcpy(exCollectionListUpdated + categoryIndex + textWritenSize, exCollectionList + categoryIndex, strlen(exCollectionList) - categoryIndex);
                }

                UnloadExampleInfo(exInfo);

                SaveFileText(exCollectionFilePath, exCollectionListUpdated);
                RL_FREE(exCollectionListUpdated);
            }
            else LOG("WARNING: ADD: Example is already on the collection\n");
            UnloadFileText(exCollectionList);
            //------------------------------------------------------------------------------------------------

            // Update: Metadata, Makefile, Makefile.Web, README.md, examples.js
            //------------------------------------------------------------------------------------------------
            UpdateRequiredFiles();
            //------------------------------------------------------------------------------------------------

            // Create: raylib/projects/VS2022/examples/<category>_example_name.vcxproj
            //------------------------------------------------------------------------------------------------
            LOG("INFO: [%s] Creating example project\n", TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName));
            // WARNING: When adding new project a unique UUID should be assigned!
            FileCopy(TextFormat("%s/../projects/VS2022/examples/core_basic_window.vcxproj", exBasePath),
                TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName));
            FileTextReplace(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName),
                "core_basic_window", exName);
            FileTextReplace(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName),
                "..\\..\\examples\\core", TextFormat("..\\..\\examples\\%s", exCategory));

            // Edit: raylib/projects/VS2022/raylib.sln --> Add new example project
            // WARNING: This function uses TextFormat() extensively inside,
            // we must store provided file paths because pointers will be overwriten
            // TODO: It seems projects are added to solution BUT not to required solution folder,
            // that process still requires to be done manually
            LOG("INFO: [%s] Adding project to raylib solution (.sln)\n", 
                TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName));
            AddVSProjectToSolution(exVSProjectSolutionFile,
                TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName), exCategory);
            //------------------------------------------------------------------------------------------------

            // Recompile example (on raylib side)
            // NOTE: Tools requirements: emscripten, w64devkit
            // Compile to: raylib.com/examples/<category>/<category>_example_name.html
            // Compile to: raylib.com/examples/<category>/<category>_example_name.data
            // Compile to: raylib.com/examples/<category>/<category>_example_name.wasm
            // Compile to: raylib.com/examples/<category>/<category>_example_name.js
            //------------------------------------------------------------------------------------------------
            // WARNING 1: EMSDK_PATH must be set to proper location when calling from GitHub Actions
            // WARNING 2: raylib.a and raylib.web.a must be available when compiling locally
#if defined(_WIN32)
            LOG("INFO: [%s] Building example for PLATFORM_WEB (Host: Win32)\n", GetFileNameWithoutExt(inFileName));
            //putenv("RAYLIB_DIR=C:\\GitHub\\raylib");
            _putenv("PATH=%PATH%;C:\\raylib\\w64devkit\\bin");
            system(TextFormat("mingw32-make -C %s -f Makefile.Web %s/%s PLATFORM=PLATFORM_WEB -B", exBasePath, exCategory, exName));
#else
            LOG("INFO: [%s] Building example for PLATFORM_WEB (Host: POSIX)\n", GetFileNameWithoutExt(inFileName));
            system(TextFormat("make -C %s -f Makefile.Web %s/%s PLATFORM=PLATFORM_WEB -B", exBasePath, exCategory, exName));
#endif
            // Update generated .html metadata
            LOG("INFO: [%s] Updating HTML Metadata...\n", TextFormat("%s.html", exName));
            UpdateWebMetadata(TextFormat("%s/%s/%s.html", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));

            // Copy results to web side
            LOG("INFO: [%s] Copy example build to raylib.com\n", exName);
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
            LOG("INFO: Command requested: RENAME\n");
            LOG("INFO: Example to be renamed: %s --> %s\n", exName, exRename);

            // NOTE: At this point provided values have been validated:
            // exName, exCategory, exRename, exRecategory
            if (strcmp(exCategory, exRecategory) == 0)
            {
                // Rename example on collection
                FileTextReplace(exCollectionFilePath, TextFormat("%s;%s", exCategory, exName),
                    TextFormat("%s;%s", exRecategory, exRename));

                // Edit: Rename example code and screenshot files .c and .png
                FileRename(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.c", exBasePath, exCategory, exRename));
                FileRename(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.png", exBasePath, exCategory, exRename));

                // Edit: Update example source code metadata
                int exListCount = 0;
                rlExampleInfo *exList = LoadExampleData(exCategory, false, &exListCount);
                for (int i = 0; i < exListCount; i++)
                {
                    if (strcmp(exList[i].name, exRename) == 0)
                        UpdateSourceMetadata(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exRename), &exList[i]);
                }
                UnloadExampleData(exList);

                // NOTE: Example resource files do not need to be changed...
                // unless the example is moved from one caegory to another

                // Edit: Rename example on required files
                FileTextReplace(TextFormat("%s/Makefile", exBasePath), exName, exRename);
                FileTextReplace(TextFormat("%s/Makefile.Web", exBasePath), exName, exRename);
                FileTextReplace(TextFormat("%s/README.md", exBasePath), exName, exRename);
                FileTextReplace(TextFormat("%s/../common/examples.js", exWebPath),
                    exName + strlen(exCategory) + 1, exRename + strlen(exRecategory) + 1); // Skip category

                // Edit: Rename example project and solution
                FileTextReplace(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName), exName, exRename);
                FileRename(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName),
                    TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exRename));
                FileTextReplace(TextFormat("%s/../projects/VS2022/raylib.sln", exBasePath), exName, exRename);
            }
            else
            {
                // WARNING: Rename with change of category
                // TODO: Reorder collection to place renamed example at the end of category
                FileTextReplace(exCollectionFilePath, TextFormat("%s;%s", exCategory, exName),
                    TextFormat("%s;%s", exRecategory, exRename));

                // TODO: Move example resources from <exCategory>/resources to <exRecategory>/resources
                // WARNING: Resources can be shared with other examples in the category

                // Edit: Rename example code file (copy and remove)
                FileCopy(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.c", exBasePath, exCategory, exRename));
                FileRemove(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));
                // Edit: Rename example screenshot file (copy and remove)
                FileCopy(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.png", exBasePath, exCategory, exRename));
                FileRemove(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName));

                // Edit: Update required files: Makefile, Makefile.Web, README.md, examples.js
                UpdateRequiredFiles();
            }

            // Remove old web compilation
            FileRemove(TextFormat("%s/%s/%s.html", exWebPath, exCategory, exName));
            FileRemove(TextFormat("%s/%s/%s.data", exWebPath, exCategory, exName));
            FileRemove(TextFormat("%s/%s/%s.wasm", exWebPath, exCategory, exName));
            FileRemove(TextFormat("%s/%s/%s.js", exWebPath, exCategory, exName));

            // Recompile example (on raylib side)
            // WARNING: EMSDK_PATH must be set to proper location when calling from GitHub Actions
#if defined(_WIN32)
            _putenv("PATH=%PATH%;C:\\raylib\\w64devkit\\bin");
            system(TextFormat("mingw32-make -C %s -f Makefile.Web %s/%s PLATFORM=PLATFORM_WEB -B", exBasePath, exRecategory, exRename));
#else
            system(TextFormat("make -C %s -f Makefile.Web %s/%s PLATFORM=PLATFORM_WEB -B", exBasePath, exRecategory, exRename));
#endif
            // Update generated .html metadata
            UpdateWebMetadata(TextFormat("%s/%s/%s.html", exBasePath, exCategory, exRename),
                TextFormat("%s/%s/%s.c", exBasePath, exCategory, exRename));

            // Copy results to web side
            FileCopy(TextFormat("%s/%s/%s.html", exBasePath, exRecategory, exRename),
                TextFormat("%s/%s/%s.html", exWebPath, exRecategory, exRename));
            FileCopy(TextFormat("%s/%s/%s.data", exBasePath, exRecategory, exRename),
                TextFormat("%s/%s/%s.data", exWebPath, exRecategory, exRename));
            FileCopy(TextFormat("%s/%s/%s.wasm", exBasePath, exRecategory, exRename),
                TextFormat("%s/%s/%s.wasm", exWebPath, exRecategory, exRename));
            FileCopy(TextFormat("%s/%s/%s.js", exBasePath, exRecategory, exRename),
                TextFormat("%s/%s/%s.js", exWebPath, exRecategory, exRename));

#if defined(RENAME_AUTO_COMMIT_CREATION)
            // Create GitHub commit with changes (local)
            putenv("PATH=%PATH%;C:\\Program Files\\Git\\bin");
            ChangeDirectory("C:\\GitHub\\raylib");
            system("git --version");
            system("git status");
            system("git add -A");
            int result = system(TextFormat("git commit -m \"REXM: RENAME: example: `%s` --> `%s`\"", exName, exRename)); // Commit changes (only tracked files)
            if (result != 0) LOG("WARNING: Error committing changes\n");
            ChangeDirectory("C:/GitHub/raylib.com");
            system("git add -A");
            result = system(TextFormat("git commit -m \"REXM: RENAME: example: `%s` --> `%s`\"", exName, exRename)); // Commit changes (only tracked files)
            if (result != 0) LOG("WARNING: Error committing changes\n");
            //result = system("git push"); // Push to the remote (origin, current branch)
            //if (result != 0) LOG("WARNING: Error pushing changes\n");
#endif

        } break;
        case OP_REMOVE:     // Remove
        {
            LOG("INFO: Command requested: REMOVE\n");
            LOG("INFO: Example to be removed: %s\n", exName);

            // Remove example from collection for files update
            //------------------------------------------------------------------------------------------------
            LOG("INFO: [%s] Removing example from collection\n", exName);
            char *exCollectionList = LoadFileText(exCollectionFilePath);
            int exIndex = TextFindIndex(exCollectionList, TextFormat("%s;%s", exCategory, exName));
            if (exIndex > 0) // Example found
            {
                char *exCollectionListUpdated = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1); // Updated list copy, 2MB

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
                            FileRemove(TextFormat("%s/%s/%s", exBasePath, exCategory, resPathUpdated));
                            RL_FREE(resPathUpdated);
                        }
                    }
                    else FileRemove(TextFormat("%s/%s/%s", exBasePath, exCategory, resPaths[r]));
                }
            }

            ClearExampleResources(resPaths);
            */
            // -----------------------------------------------------------------------------------------

            // Remove: raylib/examples/<category>/<category>_example_name.c
            // Remove: raylib/examples/<category>/<category>_example_name.png
            LOG("INFO: [%s] Removing example code file\n", TextFormat("%s.c", exName));
            FileRemove(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));
            LOG("INFO: [%s] Removing example screenshot file\n", TextFormat("%s.png", exName));
            FileRemove(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName));

            // Edit: Update required files: Makefile, Makefile.Web, README.md, examples.js
            UpdateRequiredFiles();

            // Remove: raylib/projects/VS2022/examples/<category>_example_name.vcxproj
            LOG("INFO: [%s] Removing example project file\n", TextFormat("%s.vcxproj", exName));
            FileRemove(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName));

            // Edit: raylib/projects/VS2022/raylib.sln --> Remove example project
            LOG("INFO: [%s] Removing example from raylib solution (.sln)\n", exName);
            RemoveVSProjectFromSolution(TextFormat("%s/../projects/VS2022/raylib.sln", exBasePath), exName);

            // Remove: Delete example build from local copy (if exists)
            FileRemove(TextFormat("%s/%s/%s.html", exBasePath, exCategory, exName));
            FileRemove(TextFormat("%s/%s/%s.data", exBasePath, exCategory, exName));
            FileRemove(TextFormat("%s/%s/%s.wasm", exBasePath, exCategory, exName));
            FileRemove(TextFormat("%s/%s/%s.js", exBasePath, exCategory, exName));

            // Remove: raylib.com/examples/<category>/<category>_example_name.html
            // Remove: raylib.com/examples/<category>/<category>_example_name.data
            // Remove: raylib.com/examples/<category>/<category>_example_name.wasm
            // Remove: raylib.com/examples/<category>/<category>_example_name.js
            LOG("INFO: [%s] Deleting example from raylib.com\n", exName);
            FileRemove(TextFormat("%s/%s/%s.html", exWebPath, exCategory, exName));
            FileRemove(TextFormat("%s/%s/%s.data", exWebPath, exCategory, exName));
            FileRemove(TextFormat("%s/%s/%s.wasm", exWebPath, exCategory, exName));
            FileRemove(TextFormat("%s/%s/%s.js", exWebPath, exCategory, exName));

        } break;
        case OP_BUILD:
        {
            LOG("INFO: Command requested: BUILD\n");
            LOG("INFO: Example(s) to be built: %i [%s]\n", exBuildListCount, (exBuildListCount == 1)? exBuildList[0] : argv[2]);

#if defined(_WIN32)
            // Set required environment variables
            //putenv(TextFormat("RAYLIB_DIR=%s\\..", exBasePath));
            _putenv("PATH=%PATH%;C:\\raylib\\w64devkit\\bin");
            //putenv("MAKE=mingw32-make");
            //ChangeDirectory(exBasePath);
#endif
            for (int i = 0; i < exBuildListCount; i++)
            {
                // Get example name and category
                memset(exName, 0, 64);
                strcpy(exName, exBuildList[i]);
                memset(exCategory, 0, 32);
                strncpy(exCategory, exName, TextFindIndex(exName, "_"));

                LOG("INFO: [%i/%i] Building example: [%s]\n", i + 1, exBuildListCount, exName);

                // Build example for PLATFORM_DESKTOP
#if defined(_WIN32)
                LOG("INFO: [%s] Building example for PLATFORM_DESKTOP (Host: Win32)\n", exName);
                system(TextFormat("mingw32-make -C %s %s/%s PLATFORM=PLATFORM_DESKTOP -B", exBasePath, exCategory, exName));
#elif defined(PLATFORM_DRM)
                LOG("INFO: [%s] Building example for PLATFORM_DRM (Host: POSIX)\n", exName);
                system(TextFormat("make -C %s %s/%s PLATFORM=PLATFORM_DRM -B > %s/%s/logs/%s.build.log 2>&1",
                    exBasePath, exCategory, exName, exBasePath, exCategory, exName));
#else
                LOG("INFO: [%s] Building example for PLATFORM_DESKTOP (Host: POSIX)\n", exName);
                system(TextFormat("make -C %s %s/%s PLATFORM=PLATFORM_DESKTOP -B", exBasePath, exCategory, exName));
#endif

#if !defined(PLATFORM_DRM)
                // Build example for PLATFORM_WEB
                // Build: raylib.com/examples/<category>/<category>_example_name.html
                // Build: raylib.com/examples/<category>/<category>_example_name.data
                // Build: raylib.com/examples/<category>/<category>_example_name.wasm
                // Build: raylib.com/examples/<category>/<category>_example_name.js
    #if defined(_WIN32)
                LOG("INFO: [%s] Building example for PLATFORM_WEB (Host: Win32)\n", exName);
                system(TextFormat("mingw32-make -C %s -f Makefile.Web %s/%s PLATFORM=PLATFORM_WEB -B", exBasePath, exCategory, exName));
    #else
                LOG("INFO: [%s] Building example for PLATFORM_WEB (Host: POSIX)\n", exName);
                system(TextFormat("make -C %s -f Makefile.Web %s/%s PLATFORM=PLATFORM_WEB -B", exBasePath, exCategory, exName));
    #endif
                // Update generated .html metadata
                LOG("INFO: [%s] Updating HTML Metadata...\n", TextFormat("%s.html", exName));
                UpdateWebMetadata(TextFormat("%s/%s/%s.html", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));

                // Copy results to web side
                LOG("INFO: [%s] Copy example build to raylib.com\n", exName);
                FileCopy(TextFormat("%s/%s/%s.html", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.html", exWebPath, exCategory, exName));
                FileCopy(TextFormat("%s/%s/%s.data", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.data", exWebPath, exCategory, exName));
                FileCopy(TextFormat("%s/%s/%s.wasm", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.wasm", exWebPath, exCategory, exName));
                FileCopy(TextFormat("%s/%s/%s.js", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.js", exWebPath, exCategory, exName));
#endif // !PLATFORM_DRM

                // Once example processed, free memory from list
                RL_FREE(exBuildList[i]);
            }

        } break;
        case OP_VALIDATE:     // Validate: report and actions
        case OP_UPDATE:
        {
            LOG("INFO: Command requested: %s\n", (opCode == OP_VALIDATE)? "VALIDATE" : "UPDATE");
            LOG("INFO: Example collection is being %s\n", (opCode == OP_VALIDATE)? "validated" : "validated and updated");
            /*
            // Validation flags available:
            VALID_MISSING_C
            VALID_MISSING_PNG
            VALID_INVALID_PNG
            VALID_MISSING_RESOURCES
            VALID_MISSING_VCXPROJ
            VALID_NOT_IN_VCXSOL
            VALID_NOT_IN_MAKEFILE
            VALID_NOT_IN_MAKEFILE_WEB
            VALID_NOT_IN_README
            VALID_NOT_IN_JS
            VALID_INCONSISTENT_INFO
            VALID_MISSING_WEB_OUTPUT
            VALID_INVALID_CATEGORY
            */

            // Validate and update examples collection list
            // NOTE: New .c examples found are added at the end of its category
            //---------------------------------------------------------------------------------------------------
            // Scan available example .c files and add to collection missing ones
            // NOTE: Source of truth is what we have in the examples directories (on validation/update)
            LOG("INFO: Scanning available example (.c) files to be added to collection...\n");
            FilePathList clist = LoadDirectoryFilesEx(exBasePath, ".c", true);

            // Load examples collection list file (raylib/examples/examples_list.txt)
            char *exList = LoadFileText(exCollectionFilePath);
            int exListLen = (int)strlen(exList);

            char *exListUpdated = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1);
            bool listUpdated = false;

            // Add new examples to the collection list if not found
            // WARNING: Added to the end of category, order defines place on raylib webpage
            for (unsigned int i = 0; i < clist.count; i++)
            {
                // NOTE: Skipping "examples_template" from checks
                if (!TextIsEqual(GetFileNameWithoutExt(clist.paths[i]), "examples_template") &&
                    (TextFindIndex(exList, GetFileNameWithoutExt(clist.paths[i])) == -1))
                {
                    // Get new example data
                    rlExampleInfo *exInfo = LoadExampleInfo(clist.paths[i]);

                    // Get example category, -1 if not found in list
                    int catIndex = GetTextListIndex(exInfo->category, exCategories, REXM_MAX_EXAMPLE_CATEGORIES);

                    if (catIndex > -1)
                    {
                        int nextCatIndex = catIndex + 1;
                        if (nextCatIndex > (REXM_MAX_EXAMPLE_CATEGORIES - 1)) nextCatIndex = -1; // EOF

                        // Find position to add new example on list, just before the following category
                        // Category order: core, shapes, textures, text, models, shaders, audio, [others]
                        int exListNextCatIndex = -1;
                        if (nextCatIndex != -1) exListNextCatIndex = TextFindIndex(exList, exCategories[nextCatIndex]);
                        else exListNextCatIndex = exListLen; // EOF

                        strncpy(exListUpdated, exList, exListNextCatIndex);

                        // Get example difficulty stars
                        char starsText[16] = { 0 };
                        for (int s = 0; s < 4; s++)
                        {
                            // NOTE: Every UTF-8 star are 3 bytes
                            if (s < exInfo->stars) strcpy(starsText + 3*s, "★");
                            else strcpy(starsText + 3*s, "☆");
                        }

                        // Add new example to the list
                        int exListNewExLen = sprintf(exListUpdated + exListNextCatIndex,
                            TextFormat("%s;%s;%s;%s;%s;%i;%i;\"%s\";@%s\n",
                                exInfo->category, exInfo->name, starsText, exInfo->verCreated,
                                exInfo->verUpdated, exInfo->yearCreated, exInfo->yearReviewed,
                                exInfo->author, exInfo->authorGitHub));

                        // Add the following examples to the end of collection list
                        strncpy(exListUpdated + exListNextCatIndex + exListNewExLen, exList + exListNextCatIndex, exListLen - exListNextCatIndex);

                        listUpdated = true;
                    }

                    UnloadExampleInfo(exInfo);
                }
            }

            /*
            // Check and remove duplicate example entries
            int lineCount = 0;
            char **exListLines = LoadTextLines(exList, &lineCount);
            int exListUpdatedOffset = 0;
            exListUpdatedOffset = sprintf(exListUpdated, "%s\n", exListLines[0]);

            for (int i = 1; i < lineCount; i++)
            {
                if ((TextFindIndex(exListUpdated, exListLines[i]) == -1) || (exListLines[i][0] == '#'))
                    exListUpdatedOffset += sprintf(exListUpdated + exListUpdatedOffset, "%s\n", exListLines[i]);
                else listUpdated = true;
            }

            UnloadTextLines(exListLines, lineCount);
            */

            if (listUpdated) SaveFileText(exCollectionFilePath, exListUpdated);

            UnloadFileText(exList);
            RL_FREE(exListUpdated);

            UnloadDirectoryFiles(clist);
            //---------------------------------------------------------------------------------------------------

            // Check all examples in collection [examples_list.txt] -> Source of truth!
            LOG("INFO: Validating examples in collection...\n");
            int exCollectionCount = 0;
            rlExampleInfo *exCollection = LoadExampleData("ALL", false, &exCollectionCount);

            // Set status information for all examples, using "status" field in the struct
            for (int i = 0; i < exCollectionCount; i++)
            {
                rlExampleInfo *exInfo = &exCollection[i];
                exInfo->status = 0;

                LOG("INFO: [%s] Validating example...\n", exInfo->name);

                // Validate: raylib/examples/<category>/<category>_example_name.c       -> File exists?
                if (!FileExists(TextFormat("%s/%s/%s.c", exBasePath, exInfo->category, exInfo->name))) exInfo->status |= VALID_MISSING_C;

                // Validate: raylib/examples/<category>/<category>_example_name.png     -> File exists?
                if (!FileExists(TextFormat("%s/%s/%s.png", exBasePath, exInfo->category, exInfo->name))) exInfo->status |= VALID_MISSING_PNG;

                // Validate: example screenshot is not the template default one
                Image imScreenshot = LoadImage(TextFormat("%s/%s/%s.png", exBasePath, exInfo->category, exInfo->name));
                Image imTemplate = LoadImage(TextFormat("%s/examples_template.png", exBasePath));
                if (memcmp(imScreenshot.data, imTemplate.data, GetPixelDataSize(imScreenshot.width, imScreenshot.height, imScreenshot.format)) == 0)
                    exInfo->status |= VALID_INVALID_PNG;
                UnloadImage(imTemplate);
                UnloadImage(imScreenshot);

                // Validate: raylib/examples/Makefile                                   -> Example listed?
                if (FileTextFindIndex(TextFormat("%s/Makefile", exBasePath), exInfo->name) == -1) exInfo->status |= VALID_NOT_IN_MAKEFILE;

                // Validate: raylib/examples/Makefile.Web                               -> Example listed?
                if (FileTextFindIndex(TextFormat("%s/Makefile.Web", exBasePath), exInfo->name) == -1) exInfo->status |= VALID_NOT_IN_MAKEFILE_WEB;

                // Validate: raylib/examples/README.md                                  -> Example listed?
                if (FileTextFindIndex(TextFormat("%s/README.md", exBasePath), exInfo->name) == -1) exInfo->status |= VALID_NOT_IN_README;

                // Validate: raylib.com/common/examples.js                              -> Example listed?
                if (FileTextFindIndex(TextFormat("%s/../common/examples.js", exWebPath), exInfo->name + TextFindIndex(exInfo->name, "_") + 1) == -1) exInfo->status |= VALID_NOT_IN_JS;

                // Validate: raylib/projects/VS2022/examples/<category>_example_name.vcxproj -> File exists?
                if (!FileExists(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exInfo->name))) exInfo->status |= VALID_MISSING_VCXPROJ;

                // Validate: raylib/projects/VS2022/raylib.sln                          -> Example listed?
                if (FileTextFindIndex(TextFormat("%s/../projects/VS2022/raylib.sln", exBasePath), exInfo->name) == -1) exInfo->status |= VALID_NOT_IN_VCXSOL;

                // Validate: raylib/examples/<category>/resources/..                    -> Example resources available?
                // Scan resources used in example to check for missing resource files
                // WARNING: Some paths could be for files to save, not files to load, verify it
                char **resPaths = LoadExampleResourcePaths(TextFormat("%s/%s/%s.c", exBasePath, exInfo->category, exInfo->name), &exInfo->resCount);
                if (exInfo->resCount > 0)
                {
                    for (int r = 0; r < exInfo->resCount; r++)
                    {
                        // WARNING: Special case to consider: shaders, resource paths could use conditions: "glsl%i"
                        // In this case, multiple resources are required: glsl100, glsl120, glsl330
                        if (TextFindIndex(resPaths[r], "glsl%i") > -1)
                        {
                            int glslVer[3] = { 100, 120, 330 };

                            for (int v = 0; v < 3; v++)
                            {
                                char *resPathUpdated = TextReplace(resPaths[r], "glsl%i", TextFormat("glsl%i", glslVer[v]));
                                if (!FileExists(TextFormat("%s/%s/%s", exBasePath, exInfo->category, resPathUpdated)))
                                {
                                    exInfo->status |= VALID_MISSING_RESOURCES;
                                    // Logging missing resources for convenience
                                    LOG("WARNING: [%s] Missing resource: %s\n", exInfo->name, resPathUpdated);
                                }
                                RL_FREE(resPathUpdated);
                            }
                        }
                        else
                        {
                            if (!FileExists(TextFormat("%s/%s/%s", exBasePath, exInfo->category, resPaths[r])))
                            {
                                exInfo->status |= VALID_MISSING_RESOURCES;
                                LOG("WARNING: [%s] Missing resource: %s\n", exInfo->name, resPaths[r]);
                            }
                        }
                    }
                }
                UnloadExampleResourcePaths(resPaths);

                // Validate: raylib.com/examples/<category>/<category>_example_name.html -> File exists?
                // Validate: raylib.com/examples/<category>/<category>_example_name.data -> File exists?
                // Validate: raylib.com/examples/<category>/<category>_example_name.wasm -> File exists?
                // Validate: raylib.com/examples/<category>/<category>_example_name.js   -> File exists?
                if (!TextIsEqual(exInfo->category, "others") &&
                    (!FileExists(TextFormat("%s/%s/%s.html", exWebPath, exInfo->category, exInfo->name)) ||
                     !FileExists(TextFormat("%s/%s/%s.wasm", exWebPath, exInfo->category, exInfo->name)) ||
                     !FileExists(TextFormat("%s/%s/%s.js", exWebPath, exInfo->category, exInfo->name)) ||
                     ((exInfo->resCount > 0) && !FileExists(TextFormat("%s/%s/%s.data", exWebPath, exInfo->category, exInfo->name)))))
                {
                     exInfo->status |= VALID_MISSING_WEB_OUTPUT;
                }

                // Validate: raylib.com/examples/<category>/<category>_example_name.html -> Metadata
                if (FileExists(TextFormat("%s/%s/%s.html", exWebPath, exInfo->category, exInfo->name)))
                {
                    char *exHtmlText = LoadFileText(TextFormat("%s/%s/%s.html", exWebPath, exInfo->category, exInfo->name));

                    if ((TextFindIndex(exHtmlText, "raylib web game") > -1) || // title
                        (TextFindIndex(exHtmlText, "New raylib web videogame, developed using raylib videogames library") > -1) || // description
                        (TextFindIndex(exHtmlText, "https://www.raylib.com/common/raylib_logo.png") > -1) || // image
                        (TextFindIndex(exHtmlText, "https://www.raylib.com/games.html") > -1) || // url
                        (TextFindIndex(exHtmlText, "https://github.com/raysan5/raylib';") > -1)) // source code button
                    {
                        exInfo->status |= VALID_MISSING_WEB_METADATA;
                    }

                    UnloadFileText(exHtmlText);
                }

                // NOTE: Additional validation elements
                // Validate: Example naming conventions: <category>/<category>_example_name, valid category
                if ((TextFindIndex(exInfo->name, exInfo->category) == -1) ||
                    (!TextIsEqual(exInfo->category, "core") && !TextIsEqual(exInfo->category, "shapes") &&
                     !TextIsEqual(exInfo->category, "textures") && !TextIsEqual(exInfo->category, "text") &&
                     !TextIsEqual(exInfo->category, "models") && !TextIsEqual(exInfo->category, "shaders") &&
                     !TextIsEqual(exInfo->category, "audio") && !TextIsEqual(exInfo->category, "others"))) exInfo->status |= VALID_INVALID_CATEGORY;

                // Validate: Example info (stars, author, github) missmatches with example header content
                rlExampleInfo *exInfoHeader = LoadExampleInfo(TextFormat("%s/%s/%s.c", exBasePath, exInfo->category, exInfo->name));

                if ((strcmp(exInfo->name, exInfoHeader->name) != 0) ||     // NOTE: Get it from example, not file
                    (strcmp(exInfo->category, exInfoHeader->category) != 0) ||
                    (strcmp(exInfo->author, exInfoHeader->author) != 0) ||
                    (strcmp(exInfo->authorGitHub, exInfoHeader->authorGitHub) != 0) ||
                    (exInfo->stars != exInfoHeader->stars) ||
                    (strcmp(exInfo->verCreated, exInfoHeader->verCreated) != 0) ||
                    (strcmp(exInfo->verUpdated, exInfoHeader->verUpdated) != 0))
                {
                    exInfo->status |= VALID_INCONSISTENT_INFO;
                }

                if (exInfo->status == 0) LOG("INFO: [%s] Validation result: OK\n", exInfo->name);
                else LOG("WARNING: [%s] Validation result: ISSUES FOUND\n", exInfo->name);

                UnloadExampleInfo(exInfoHeader);
            }

            if (opCode == OP_UPDATE)
            {
                LOG("INFO: Updating examples with issues in collection...\n");

                // Actions to fix/review anything possible from validation results
                //------------------------------------------------------------------------------------------------
                // Update files: Makefile, Makefile.Web, README.md, examples.js
                // Solves: VALID_NOT_IN_MAKEFILE, VALID_NOT_IN_MAKEFILE_WEB, VALID_NOT_IN_README, VALID_NOT_IN_JS
                // WARNING: Makefile.Web needs to be updated before trying to rebuild web example!
                UpdateRequiredFiles();
                for (int i = 0; i < exCollectionCount; i++)
                {
                    exCollection[i].status &= ~VALID_NOT_IN_MAKEFILE;
                    exCollection[i].status &= ~VALID_NOT_IN_MAKEFILE_WEB;
                    exCollection[i].status &= ~VALID_NOT_IN_README;
                    exCollection[i].status &= ~VALID_NOT_IN_JS;
                }
                
                // Check examples "status" information
                for (int i = 0; i < exCollectionCount; i++)
                {
                    rlExampleInfo *exInfo = &exCollection[i];

                    if (exInfo->status & VALID_MISSING_C) LOG("WARNING: [%s] Missing code file\n", exInfo->name);
                    else
                    {
                        // NOTE: Some issues can not be automatically fixed, only logged
                        if (exInfo->status & VALID_MISSING_PNG) LOG("WARNING: [%s] Missing screenshot file\n", exInfo->name);
                        if (exInfo->status & VALID_INVALID_PNG) LOG("WARNING: [%s] Invalid screenshot file (using template)\n", exInfo->name);
                        if (exInfo->status & VALID_MISSING_RESOURCES) LOG("WARNING: [%s] Missing resources detected\n", exInfo->name);
                        if (exInfo->status & VALID_INCONSISTENT_INFO) LOG("WARNING: [%s] Inconsistent example header info\n", exInfo->name);
                        if (exInfo->status & VALID_INVALID_CATEGORY) LOG("WARNING: [%s] Invalid example category\n", exInfo->name);

                        // NOTE: Some examples should be excluded from VS2022 solution because
                        // they have specific platform/linkage requirements:
                        if ((strcmp(exInfo->name, "web_basic_window") == 0) ||
                            (strcmp(exInfo->name, "raylib_opengl_interop") == 0)) continue;

                        // Review: Add: raylib/projects/VS2022/examples/<category>_example_name.vcxproj
                        // Review: Add: raylib/projects/VS2022/raylib.sln
                        // Solves: VALID_MISSING_VCXPROJ, VALID_NOT_IN_VCXSOL
                        if (exInfo->status & VALID_MISSING_VCXPROJ)
                        {
                            LOG("WARNING: [%s] Missing VS2022 project file\n", exInfo->name);
                            LOG("INFO: [%s.vcxproj] Creating VS2022 project file\n", exInfo->name);
                            FileCopy(TextFormat("%s/../projects/VS2022/examples/core_basic_window.vcxproj", exBasePath),
                                TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exInfo->name));
                            FileTextReplace(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exInfo->name),
                                "core_basic_window", exInfo->name);
                            FileTextReplace(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exInfo->name),
                                "..\\..\\examples\\core", TextFormat("..\\..\\examples\\%s", exInfo->category));

                            exInfo->status &= ~VALID_MISSING_VCXPROJ;
                        }

                        // Add project (.vcxproj) to raylib solution (.sln)
                        if (exInfo->status & VALID_NOT_IN_VCXSOL)
                        {
                            LOG("WARNING: [%s.vcxproj] Project not included in raylib solution (.sln)\n", exInfo->name);
                            LOG("INFO: [%s.vcxproj] Adding project to raylib solution (.sln)\n", exInfo->name);
                            AddVSProjectToSolution(exVSProjectSolutionFile,
                                TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exInfo->name), exInfo->category);

                            exInfo->status &= ~VALID_NOT_IN_VCXSOL;
                        }

                        // Review: Add/Remove: raylib.com/examples/<category>/<category>_example_name.html
                        // Review: Add/Remove: raylib.com/examples/<category>/<category>_example_name.data
                        // Review: Add/Remove: raylib.com/examples/<category>/<category>_example_name.wasm
                        // Review: Add/Remove: raylib.com/examples/<category>/<category>_example_name.js
                        // Solves: VALID_MISSING_WEB_OUTPUT
                        if ((strcmp(exInfo->category, "others") != 0) && // Skipping "others" category
                            ((exInfo->status & VALID_MISSING_WEB_OUTPUT) || (exInfo->status & VALID_MISSING_WEB_METADATA)))
                        {
                            LOG("WARNING: [%s] Example not available on raylib web\n", exInfo->name);

                            // Build example for PLATFORM_WEB
                        #if defined(_WIN32)
                            LOG("INFO: [%s] Building example for PLATFORM_WEB (Host: Win32)\n", exInfo->name);
                            _putenv("PATH=%PATH%;C:\\raylib\\w64devkit\\bin");
                            system(TextFormat("mingw32-make -C %s -f Makefile.Web %s/%s PLATFORM=PLATFORM_WEB -B", exBasePath, exInfo->category, exInfo->name));
                        #else
                            LOG("INFO: [%s] Building example for PLATFORM_WEB (Host: POSIX)\n", exInfo->name);
                            system(TextFormat("make -C %s -f Makefile.Web %s/%s PLATFORM=PLATFORM_WEB -B", exBasePath, exInfo->category, exInfo->name));
                        #endif

                            // Update generated .html metadata
                            LOG("INFO: [%s.html] Updating HTML Metadata...\n", exInfo->name);
                            UpdateWebMetadata(TextFormat("%s/%s/%s.html", exBasePath, exInfo->category, exInfo->name),
                                TextFormat("%s/%s/%s.c", exBasePath, exInfo->category, exInfo->name));

                            // Copy results to web side
                            LOG("INFO: [%s] Copy example build to raylib.com\n", exInfo->name);
                            FileCopy(TextFormat("%s/%s/%s.html", exBasePath, exInfo->category, exInfo->name),
                                TextFormat("%s/%s/%s.html", exWebPath, exInfo->category, exInfo->name));
                            FileCopy(TextFormat("%s/%s/%s.data", exBasePath, exInfo->category, exInfo->name),
                                TextFormat("%s/%s/%s.data", exWebPath, exInfo->category, exInfo->name));
                            FileCopy(TextFormat("%s/%s/%s.wasm", exBasePath, exInfo->category, exInfo->name),
                                TextFormat("%s/%s/%s.wasm", exWebPath, exInfo->category, exInfo->name));
                            FileCopy(TextFormat("%s/%s/%s.js", exBasePath, exInfo->category, exInfo->name),
                                TextFormat("%s/%s/%s.js", exWebPath, exInfo->category, exInfo->name));

                            exInfo->status &= ~VALID_MISSING_WEB_OUTPUT;
                            exInfo->status &= ~VALID_MISSING_WEB_METADATA;
                        }

                        if (exInfo->status & VALID_INCONSISTENT_INFO)
                        {
                            // Update source code header info
                            LOG("WARNING: [%s.c] Inconsistent source code metadata\n", exInfo->name);
                            LOG("INFO: [%s.c] Updating source code metadata...\n", exInfo->name);
                            UpdateSourceMetadata(TextFormat("%s/%s/%s.c", exBasePath, exInfo->category, exInfo->name), exInfo);

                            exInfo->status &= ~VALID_INCONSISTENT_INFO;
                        }
                    }
                }
                //------------------------------------------------------------------------------------------------
            }

            // Generate validation report/table with results (.md)
            //-----------------------------------------------------------------------------------------------------
            /*
            Columns:
            [C]     VALID_MISSING_C             // Missing .c source file
            [PNG]   VALID_MISSING_PNG           // Missing screenshot .png
            [WPNG]  VALID_INVALID_PNG           // Invalid png screenshot (using template one)
            [RES]   VALID_MISSING_RESOURCES     // Missing resources listed in the code
            [VCX]   VALID_MISSING_VCXPROJ       // Missing Visual Studio .vcxproj file
            [SOL]   VALID_NOT_IN_VCXSOL         // Project not included in solution file
            [MK]    VALID_NOT_IN_MAKEFILE       // Not listed in Makefile
            [MKWEB] VALID_NOT_IN_MAKEFILE_WEB   // Not listed in Makefile.Web
            [RDME]  VALID_NOT_IN_README         // Not listed in README.md
            [JS]    VALID_NOT_IN_JS             // Not listed in examples.js
            [WOUT]  VALID_MISSING_WEB_OUTPUT    // Missing .html/.data/.wasm/.js
            [WMETA] VALID_MISSING_WEB_METADATA  // Missing .html example metadata
            [INFO]  VALID_INCONSISTENT_INFO     // Inconsistent info between collection and example header (stars, author...)
            [CAT]   VALID_INVALID_CATEGORY      // Not a recognized category

            | [EXAMPLE NAME]               | [C] |[CAT]|[INFO]|[PNG]|[WPNG]|[RES]|[MK] |[MKWEB]|[VCX]|[SOL]|[RDME]|[JS] |[WOUT]|[WMETA]|
            |:-----------------------------|:---:|:---:|:----:|:---:|:----:|:---:|:---:|:-----:|:---:|:---:|:----:|:---:|:----:|:-----:|
            | core_basic_window            |  ✔ |  ✔  |  ✔  |  ✔ |  ✔  |  ✔  |  ✔ |   ✔  |  ✔  |  ✔ |  ✔  |  ✔ |   ✔  |   ✔  |
            | shapes_colors_palette        |  ✘ |  ✔  |  ✘  |  ✔ |  ✘  |  ✔  |  ✔ |   ✘  |  ✔  |  ✔ |  ✔  |  ✔ |   ✔  |   ✔  |
            | text_format_text             |  ✘ |  ✘  |  ✘  |  ✘ |  ✘  |  ✘  |  ✘ |   ✘  |  ✔  |  ✘ |  ✔  |  ✔ |   ✔  |   ✔  |
            */
            LOG("INFO: [examples_validation.md] Generating examples validation report...\n");

            char *report = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1);

            int repIndex = 0;
            repIndex += sprintf(report + repIndex, "# EXAMPLES COLLECTION - VALIDATION REPORT\n\n");

            repIndex += sprintf(report + repIndex, "```\nExample elements validated:\n\n");
            repIndex += sprintf(report + repIndex, " - [C]     : Missing .c source file\n");
            repIndex += sprintf(report + repIndex, " - [CAT]   : Not a recognized category\n");
            repIndex += sprintf(report + repIndex, " - [INFO]  : Inconsistent example header info (stars, author...)\n");
            repIndex += sprintf(report + repIndex, " - [PNG]   : Missing screenshot .png\n");
            repIndex += sprintf(report + repIndex, " - [WPNG]  : Invalid png screenshot (using default one)\n");
            repIndex += sprintf(report + repIndex, " - [RES]   : Missing resources listed in the code\n");
            repIndex += sprintf(report + repIndex, " - [MK]    : Not listed in Makefile\n");
            repIndex += sprintf(report + repIndex, " - [MKWEB] : Not listed in Makefile.Web\n");
            repIndex += sprintf(report + repIndex, " - [VCX]   : Missing Visual Studio project file\n");
            repIndex += sprintf(report + repIndex, " - [SOL]   : Project not included in solution file\n");
            repIndex += sprintf(report + repIndex, " - [RDME]  : Not listed in README.md\n");
            repIndex += sprintf(report + repIndex, " - [JS]    : Not listed in Web (examples.js)\n");
            repIndex += sprintf(report + repIndex, " - [WOUT]  : Missing Web build (.html/.data/.wasm/.js)\n");
            repIndex += sprintf(report + repIndex, " - [WMETA] : Missing Web .html example metadata\n```\n");

            repIndex += sprintf(report + repIndex, "| **EXAMPLE NAME**                 | [C] | [CAT]| [INFO]|[PNG]|[WPNG]| [RES]| [MK] |[MKWEB]| [VCX]| [SOL]|[RDME]|[JS] | [WOUT]|[WMETA]|\n");
            repIndex += sprintf(report + repIndex, "|:---------------------------------|:---:|:----:|:-----:|:---:|:----:|:----:|:----:|:-----:|:----:|:----:|:----:|:---:|:-----:|:-----:|\n");

            for (int i = 0; i < exCollectionCount; i++)
            {
                repIndex += sprintf(report + repIndex, "| %-32s |  %s |  %s  |  %s  |  %s |  %s  |  %s  |  %s |   %s  |  %s  |  %s |  %s  |  %s |  %s  |  %s  |\n",
                    exCollection[i].name,
                    (exCollection[i].status & VALID_MISSING_C)? "❌" : "✔",
                    (exCollection[i].status & VALID_INVALID_CATEGORY)? "❌" : "✔",
                    (exCollection[i].status & VALID_INCONSISTENT_INFO)? "❌" : "✔",
                    (exCollection[i].status & VALID_MISSING_PNG)? "❌" : "✔",
                    (exCollection[i].status & VALID_INVALID_PNG)? "❌" : "✔",
                    (exCollection[i].status & VALID_MISSING_RESOURCES)? "❌" : "✔",
                    (exCollection[i].status & VALID_NOT_IN_MAKEFILE)? "❌" : "✔",
                    (exCollection[i].status & VALID_NOT_IN_MAKEFILE_WEB)? "❌" : "✔",
                    (exCollection[i].status & VALID_MISSING_VCXPROJ)? "❌" : "✔",
                    (exCollection[i].status & VALID_NOT_IN_VCXSOL)? "❌" : "✔",
                    (exCollection[i].status & VALID_NOT_IN_README)? "❌" : "✔",
                    (exCollection[i].status & VALID_NOT_IN_JS)? "❌" : "✔",
                    (exCollection[i].status & VALID_MISSING_WEB_OUTPUT)? "❌" : "✔",
                    (exCollection[i].status & VALID_MISSING_WEB_METADATA)? "❌" : "✔");
            }

            SaveFileText(TextFormat("%s/../tools/rexm/reports/%s", exBasePath, "examples_validation.md"), report);
            RL_FREE(report);
            //-----------------------------------------------------------------------------------------------------

            // Generate a report with only the examples missing some elements
            //-----------------------------------------------------------------------------------------------------
            LOG("INFO: [examples_issues.md] Generating examples issues report...\n");

            char *reportIssues = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1);

            repIndex = 0;
            repIndex += sprintf(reportIssues + repIndex, "# EXAMPLES COLLECTION - VALIDATION REPORT\n\n");

            repIndex += sprintf(reportIssues + repIndex, "```\nExample elements validated:\n\n");
            repIndex += sprintf(reportIssues + repIndex, " - [C]     : Missing .c source file\n");
            repIndex += sprintf(reportIssues + repIndex, " - [CAT]   : Not a recognized category\n");
            repIndex += sprintf(reportIssues + repIndex, " - [INFO]  : Inconsistent example header info (stars, author...)\n");
            repIndex += sprintf(reportIssues + repIndex, " - [PNG]   : Missing screenshot .png\n");
            repIndex += sprintf(reportIssues + repIndex, " - [WPNG]  : Invalid png screenshot (using default one)\n");
            repIndex += sprintf(reportIssues + repIndex, " - [RES]   : Missing resources listed in the code\n");
            repIndex += sprintf(reportIssues + repIndex, " - [MK]    : Not listed in Makefile\n");
            repIndex += sprintf(reportIssues + repIndex, " - [MKWEB] : Not listed in Makefile.Web\n");
            repIndex += sprintf(reportIssues + repIndex, " - [VCX]   : Missing Visual Studio project file\n");
            repIndex += sprintf(reportIssues + repIndex, " - [SOL]   : Project not included in solution file\n");
            repIndex += sprintf(reportIssues + repIndex, " - [RDME]  : Not listed in README.md\n");
            repIndex += sprintf(reportIssues + repIndex, " - [JS]    : Not listed in Web (examples.js)\n");
            repIndex += sprintf(reportIssues + repIndex, " - [WOUT]  : Missing Web build (.html/.data/.wasm/.js)\n");
            repIndex += sprintf(reportIssues + repIndex, " - [WMETA] : Missing Web .html example metadata\n```\n");

            repIndex += sprintf(reportIssues + repIndex, "| **EXAMPLE NAME**                 | [C] | [CAT]| [INFO]|[PNG]|[WPNG]| [RES]| [MK] |[MKWEB]| [VCX]| [SOL]|[RDME]|[JS] | [WOUT]|[WMETA]|\n");
            repIndex += sprintf(reportIssues + repIndex, "|:---------------------------------|:---:|:----:|:-----:|:---:|:----:|:----:|:----:|:-----:|:----:|:----:|:----:|:---:|:-----:|:-----:|\n");

            for (int i = 0; i < exCollectionCount; i++)
            {
                if (exCollection[i].status > 0)
                {
                    repIndex += sprintf(reportIssues + repIndex, "| %-32s |  %s |  %s  |  %s  |  %s |  %s  |  %s  |  %s |   %s  |  %s  |  %s |  %s  |  %s |  %s  |  %s  |\n",
                        exCollection[i].name,
                        (exCollection[i].status & VALID_MISSING_C)? "❌" : "✔",
                        (exCollection[i].status & VALID_INVALID_CATEGORY)? "❌" : "✔",
                        (exCollection[i].status & VALID_INCONSISTENT_INFO)? "❌" : "✔",
                        (exCollection[i].status & VALID_MISSING_PNG)? "❌" : "✔",
                        (exCollection[i].status & VALID_INVALID_PNG)? "❌" : "✔",
                        (exCollection[i].status & VALID_MISSING_RESOURCES)? "❌" : "✔",
                        (exCollection[i].status & VALID_NOT_IN_MAKEFILE)? "❌" : "✔",
                        (exCollection[i].status & VALID_NOT_IN_MAKEFILE_WEB)? "❌" : "✔",
                        (exCollection[i].status & VALID_MISSING_VCXPROJ)? "❌" : "✔",
                        (exCollection[i].status & VALID_NOT_IN_VCXSOL)? "❌" : "✔",
                        (exCollection[i].status & VALID_NOT_IN_README)? "❌" : "✔",
                        (exCollection[i].status & VALID_NOT_IN_JS)? "❌" : "✔",
                        (exCollection[i].status & VALID_MISSING_WEB_OUTPUT)? "❌" : "✔",
                        (exCollection[i].status & VALID_MISSING_WEB_METADATA)? "❌" : "✔");
                }
            }

            SaveFileText(TextFormat("%s/../tools/rexm/reports/%s", exBasePath, "examples_issues.md"), reportIssues);
            RL_FREE(reportIssues);
            //-----------------------------------------------------------------------------------------------------

            UnloadExampleData(exCollection);
            //------------------------------------------------------------------------------------------------

        } break;
        case OP_TEST:
        {
            LOG("INFO: Command requested: TEST\n");
            LOG("INFO: Example(s) to be build and tested: %i [%s]\n", exBuildListCount, (exBuildListCount == 1)? exBuildList[0] : argv[2]);

#if defined(_WIN32)
            // Set required environment variables
            //putenv(TextFormat("RAYLIB_DIR=%s\\..", exBasePath));
            //_putenv("PATH=%PATH%;C:\\raylib\\w64devkit\\bin");
            //putenv("MAKE=mingw32-make");
            //ChangeDirectory(exBasePath);
            //_putenv("MAKE_PATH=C:\\raylib\\w64devkit\\bin");
            //_putenv("EMSDK_PATH = C:\\raylib\\emsdk");
            //_putenv("PYTHON_PATH=$(EMSDK_PATH)\\python\\3.9.2-nuget_64bit");
            //_putenv("NODE_PATH=$(EMSDK_PATH)\\node\\20.18.0_64bit\\bin");
            //_putenv("PATH=%PATH%;$(MAKE_PATH);$(EMSDK_PATH);$(NODE_PATH);$(PYTHON_PATH)");

            _putenv("PATH=%PATH%;C:\\raylib\\w64devkit\\bin;C:\\raylib\\emsdk\\python\\3.9.2-nuget_64bit;C:\\raylib\\emsdk\\node\\20.18.0_64bit\\bin");
#endif

            for (int i = 0; i < exBuildListCount; i++)
            {
                // Get example name and category
                memset(exName, 0, 64);
                strcpy(exName, exBuildList[i]);
                memset(exCategory, 0, 32);
                strncpy(exCategory, exName, TextFindIndex(exName, "_"));

                // Skip some examples from building
                if ((strcmp(exName, "core_custom_logging") == 0) ||
                    (strcmp(exName, "core_window_should_close") == 0) ||
                    (strcmp(exName, "core_custom_frame_control") == 0)) continue;

                LOG("INFO: [%i/%i] Testing example: [%s]\n", i + 1, exBuildListCount, exName);

                // Create directory for logs (build and run logs)
                MakeDirectory(TextFormat("%s/%s/logs", exBasePath, exCategory));

                // Steps to follow
                // STEP 1: Load example.c and replace required code to inject basic testing code: frames to run
                //    OPTION 1: Code injection required multiple changes for testing but it does not require raylib changes!
                //    OPTION 2: Support testing on raylib side: Args processing and events injection: SUPPORT_AUTOMATD_TESTING_SYSTEM, EVENTS_TESTING_MODE
                // STEP 2: Build example (PLATFORM_DESKTOP)
                // STEP 3: Run example with arguments: --frames 2 > <example>.out.log
                // STEP 4: Load <example>.out.log and check "WARNING:" messages -> Some could maybe be ignored
                // STEP 5: Generate report with results

                // STEP 1: Load example and inject required code
                //    PROBLEM: As we need to modify the example source code for building, we need to keep a copy or something
                //      WARNING: If we make a copy and something fails, it could not be restored at the end
                //    PROBLEM: Trying to build a copy won't work because Makefile is setup to look for specific example on specific path -> No output dir config
                //    IDEA: Create directory for testing data -> It implies moving files and set working dir...
                //    SOLUTION: Make a copy of original file -> Modify original -> Build -> Rename to <example>.test.exe
                FileCopy(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.original.c", exBasePath, exCategory, exName));
                char *srcText = LoadFileText(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));

//#define BUILD_TESTING_WEB
#if defined(BUILD_TESTING_WEB)
                static const char *mainReplaceText =
                    "#include <stdio.h>\n"
                    "#include <string.h>\n"
                    "#include <stdlib.h>\n"
                    "#include <emscripten/emscripten.h>\n\n"
                    "static char logText[4096] = {0};\n"
                    "static int logTextOffset = 0;\n\n"
                    "void CustomTraceLog(int msgType, const char *text, va_list args)\n{\n"
                    "    if (logTextOffset < 3800)\n    {\n"
                    "    switch (msgType)\n    {\n"
                    "        case LOG_INFO: logTextOffset += sprintf(logText + logTextOffset, \"INFO: \"); break;\n"
                    "        case LOG_ERROR: logTextOffset += sprintf(logText + logTextOffset, \"ERROR: \"); break;\n"
                    "        case LOG_WARNING: logTextOffset += sprintf(logText + logTextOffset, \"WARNING: \"); break;\n"
                    "        case LOG_DEBUG: logTextOffset += sprintf(logText + logTextOffset, \"DEBUG: \"); break;\n"
                    "        default: break;\n    }\n"
                    "    logTextOffset += vsprintf(logText + logTextOffset, text, args);\n"
                    "    logTextOffset += sprintf(logText + logTextOffset, \"\\n\");\n}\n}\n\n"
                    "int main(int argc, char *argv[])\n{\n"
                    "    SetTraceLogCallback(CustomTraceLog);\n"
                    "    int requestedTestFrames = 0;\n"
                    "    int testFramesCount = 0;\n"
                    "    if ((argc > 1) && (argc == 3) && (strcmp(argv[1], \"--frames\") != 0)) requestedTestFrames = atoi(argv[2]);\n";

                static const char *returnReplaceText =
                    "    SaveFileText(\"outputLogFileName\", logText);\n"
                    "    emscripten_run_script(\"saveFileFromMEMFSToDisk('outputLogFileName','outputLogFileName')\");\n\n"
                    "    return 0";
                char *returnReplaceTextUpdated = TextReplace(returnReplaceText, "outputLogFileName", TextFormat("%s.log", exName));

                char *srcTextUpdated[4] = { 0 };
                srcTextUpdated[0] = TextReplace(srcText, "int main(void)\n{", mainReplaceText);
                srcTextUpdated[1] = TextReplace(srcTextUpdated[0], "WindowShouldClose()", "WindowShouldClose() && (testFramesCount < requestedTestFrames)");
                srcTextUpdated[2] = TextReplace(srcTextUpdated[1], "EndDrawing();", "EndDrawing(); testFramesCount++;");
                srcTextUpdated[3] = TextReplace(srcTextUpdated[2], "    return 0", returnReplaceTextUpdated);
                MemFree(returnReplaceTextUpdated);
                UnloadFileText(srcText);

                SaveFileText(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName), srcTextUpdated[3]);
                for (int i = 0; i < 4; i++) { MemFree(srcTextUpdated[i]); srcTextUpdated[i] = NULL; }

                // Build example for PLATFORM_WEB
                // Build: raylib.com/examples/<category>/<category>_example_name.html
                // Build: raylib.com/examples/<category>/<category>_example_name.data
                // Build: raylib.com/examples/<category>/<category>_example_name.wasm
                // Build: raylib.com/examples/<category>/<category>_example_name.js
    #if defined(_WIN32)
                LOG("INFO: [%s] Building example for PLATFORM_WEB (Host: Win32)\n", exName);
                system(TextFormat("mingw32-make -C %s -f Makefile.Web %s/%s PLATFORM=PLATFORM_WEB -B > %s/%s/logs/%s.build.log 2>&1",
                    exBasePath, exCategory, exName, exBasePath, exCategory, exName));
    #else
                LOG("INFO: [%s] Building example for PLATFORM_WEB (Host: POSIX)\n", exName);
                system(TextFormat("make -C %s -f Makefile.Web %s/%s PLATFORM=PLATFORM_WEB -B", exBasePath, exCategory, exName));
    #endif
                // Restore original source code before continue
                FileCopy(TextFormat("%s/%s/%s.original.c", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));
                FileRemove(TextFormat("%s/%s/%s.original.c", exBasePath, exCategory, exName));

                // STEP 3: Run example on browser
                if (FileExists(TextFormat("%s/%s/%s.html", exBasePath, exCategory, exName)) &&
                    FileExists(TextFormat("%s/%s/%s.wasm", exBasePath, exCategory, exName)) &&
                    FileExists(TextFormat("%s/%s/%s.js", exBasePath, exCategory, exName)))
                {
                    // WARNING: Example download is asynchronous so reading fails on next step
                    // when looking for a file that could not have been downloaded yet
                    ChangeDirectory(TextFormat("%s", exBasePath));
                    if (i == 0) system("start python -m http.server 8080"); // Init localhost just once
                    system(TextFormat("start explorer \"http:\\localhost:8080/%s/%s.html", exCategory, exName));
                }

                // NOTE: Example .log is automatically downloaded into system Downloads directory on browser-example exectution

#else // BUILD_TESTING_DESKTOP

                static const char *mainReplaceText =
                    "#include <string.h>\n"
                    "#include <stdlib.h>\n"
                    "int main(int argc, char *argv[])\n{\n"
                    "    int requestedTestFrames = 0;\n"
                    "    int testFramesCount = 0;\n"
                    "    if ((argc > 1) && (argc == 3) && (strcmp(argv[1], \"--frames\") != 0)) requestedTestFrames = atoi(argv[2]);\n";

                char *srcTextUpdated[3] = { 0 };
                srcTextUpdated[0] = TextReplace(srcText, "int main(void)\n{", mainReplaceText);
                srcTextUpdated[1] = TextReplace(srcTextUpdated[0], "WindowShouldClose()", "WindowShouldClose() && (testFramesCount < requestedTestFrames)");
                srcTextUpdated[2] = TextReplace(srcTextUpdated[1], "EndDrawing();", "EndDrawing(); testFramesCount++;");
                UnloadFileText(srcText);

                SaveFileText(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName), srcTextUpdated[2]);
                for (int i = 0; i < 3; i++) { MemFree(srcTextUpdated[i]); srcTextUpdated[i] = NULL; }

                // STEP 2: Build example for DESKTOP platform
    #if defined(_WIN32)
                // Set required environment variables
                //putenv(TextFormat("RAYLIB_DIR=%s\\..", exBasePath));
                _putenv("PATH=%PATH%;C:\\raylib\\w64devkit\\bin");
                //putenv("MAKE=mingw32-make");
                //ChangeDirectory(exBasePath);
    #endif
                // Build example for PLATFORM_DESKTOP
    #if defined(_WIN32)
                LOG("INFO: [%s] Building example for PLATFORM_DESKTOP (Host: Win32)\n", exName);
                system(TextFormat("mingw32-make -C %s %s/%s PLATFORM=PLATFORM_DESKTOP -B > %s/%s/logs/%s.build.log 2>&1",
                    exBasePath, exCategory, exName, exBasePath, exCategory, exName));
    #elif defined(PLATFORM_DRM)
                LOG("INFO: [%s] Building example for PLATFORM_DRM (Host: POSIX)\n", exName);
                system(TextFormat("make -C %s %s/%s PLATFORM=PLATFORM_DRM -B > %s/%s/logs/%s.build.log 2>&1",
                    exBasePath, exCategory, exName, exBasePath, exCategory, exName));
    #else
                LOG("INFO: [%s] Building example for PLATFORM_DESKTOP (Host: POSIX)\n", exName);
                system(TextFormat("make -C %s %s/%s PLATFORM=PLATFORM_DESKTOP -B > %s/%s/logs/%s.build.log 2>&1",
                    exBasePath, exCategory, exName, exBasePath, exCategory, exName));
    #endif
                // Restore original source code before continue
                FileCopy(TextFormat("%s/%s/%s.original.c", exBasePath, exCategory, exName),
                    TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));
                FileRemove(TextFormat("%s/%s/%s.original.c", exBasePath, exCategory, exName));

                // STEP 3: Run example with required arguments
                // NOTE: Not easy to retrieve process return value from system(), it's platform dependant
                ChangeDirectory(TextFormat("%s/%s", exBasePath, exCategory));

    #if defined(_WIN32)
                system(TextFormat("%s --frames 2 > logs/%s.log", exName, exName));
    #else
                system(TextFormat("./%s --frames 2 > logs/%s.log", exName, exName));
    #endif
#endif
            }
        } break;
        case OP_TESTLOG:
        {
            // STEP 4: Load and validate available logs info
            //---------------------------------------------------------------------------------------------
            rlExampleTesting *testing = (rlExampleTesting *)RL_CALLOC(exBuildListCount, sizeof(rlExampleTesting));

            for (int i = 0; i < exBuildListCount; i++)
            {
                // Get example name and category
                memset(exName, 0, 64);
                strcpy(exName, exBuildList[i]);
                memset(exCategory, 0, 32);
                strncpy(exCategory, exName, TextFindIndex(exName, "_"));

                // Skip some examples from building
                if ((strcmp(exName, "core_custom_logging") == 0) ||
                    (strcmp(exName, "core_window_should_close") == 0) ||
                    (strcmp(exName, "core_custom_frame_control") == 0)) continue;

                LOG("INFO: [%i/%i] Checking example log: [%s]\n", i + 1, exBuildListCount, exName);

                // Load <example_name>.build.log to check for compilation warnings
                char *exTestBuildLog = LoadFileText(TextFormat("%s/%s/logs/%s.build.log", exBasePath, exCategory, exName));
                if (exTestBuildLog == NULL)
                {
                    LOG("WARNING: [%s] Build log could not be loaded\n", exName);
                    continue;
                }

                // Load build log text lines
                int exTestBuildLogLinesCount = 0;
                char **exTestBuildLogLines = LoadTextLines(exTestBuildLog, &exTestBuildLogLinesCount);

                for (int k = 0; k < exTestBuildLogLinesCount; k++)
                {
                    // Checking compilation warnings generated
                    if (TextFindIndex(exTestBuildLogLines[k], "warning:") >= 0) testing[i].buildwarns++;
                }

                UnloadTextLines(exTestBuildLogLines, exTestBuildLogLinesCount);
                UnloadFileText(exTestBuildLog);

#if defined(BUILD_TESTING_WEB)
                // TODO: REVIEW: Hardcoded path where web logs are copied after automatic download
                char *exTestLog = LoadFileText(TextFormat("D:/testing_logs_web/%s.log", exName));
#else
                char *exTestLog = LoadFileText(TextFormat("%s/%s/logs/%s.log", exBasePath, exCategory, exName));
#endif
                if (exTestLog == NULL)
                {
                    LOG("WARNING: [%s] Execution log could not be loaded\n", exName);
                    testing[i].status = 0b1111111;
                    continue;
                }

                /*
                TESTING_FAIL_INIT      = 1 << 0,   // Initialization (InitWindow())    -> "INFO: DISPLAY: Device initialized successfully"
                TESTING_FAIL_CLOSE     = 1 << 1,   // Closing (CloseWindow())          -> "INFO: Window closed successfully"
                TESTING_FAIL_ASSETS    = 1 << 2,   // Assets loading (WARNING: FILE:)  -> "WARNING: FILEIO:"
                TESTING_FAIL_RLGL      = 1 << 3,   // OpenGL-wrapped initialization    -> "INFO: RLGL: Default OpenGL state initialized successfully"
                TESTING_FAIL_PLATFORM  = 1 << 4,   // Platform initialization          -> "INFO: PLATFORM: DESKTOP (GLFW - Win32): Initialized successfully"
                TESTING_FAIL_FONT      = 1 << 5,   // Font default initialization      -> "INFO: FONT: Default font loaded successfully (224 glyphs)"
                TESTING_FAIL_TIMER     = 1 << 6,   // Timer initialization             -> "INFO: TIMER: Target time per frame: 16.667 milliseconds"
                */

                if (TextFindIndex(exTestLog, "INFO: DISPLAY: Device initialized successfully") == -1) testing[i].status |= TESTING_FAIL_INIT;
                if (TextFindIndex(exTestLog, "INFO: Window closed successfully") == -1) testing[i].status |= TESTING_FAIL_CLOSE;
                if (TextFindIndex(exTestLog, "WARNING: FILEIO:") >= 0) testing[i].status |= TESTING_FAIL_ASSETS;
                if (TextFindIndex(exTestLog, "INFO: RLGL: Default OpenGL state initialized successfully") == -1) testing[i].status |= TESTING_FAIL_RLGL;
                if (TextFindIndex(exTestLog, "INFO: PLATFORM:") == -1) testing[i].status |= TESTING_FAIL_PLATFORM;
                if (TextFindIndex(exTestLog, "INFO: FONT: Default font loaded successfully") == -1) testing[i].status |= TESTING_FAIL_FONT;
                if (TextFindIndex(exTestLog, "INFO: TIMER: Target time per frame:") == -1) testing[i].status |= TESTING_FAIL_TIMER;

                // Load build log text lines
                int exTestLogLinesCount = 0;
                char **exTestLogLines = LoadTextLines(exTestLog, &exTestLogLinesCount);
                for (int k = 0; k < exTestLogLinesCount; k++)
                {
#if defined(BUILD_TESTING_WEB)
                    if (TextFindIndex(exTestLogLines[k], "WARNING: GL: NPOT") >= 0) continue; // Ignore web-specific warning
#endif
#if defined(PLATFORM_DRM)
                    if (TextFindIndex(exTestLogLines[k], "WARNING: DISPLAY: No graphic") >= 0) continue; // Ignore specific warning
                    if (TextFindIndex(exTestLogLines[k], "WARNING: GetCurrentMonitor()") >= 0) continue; // Ignore specific warning
                    if (TextFindIndex(exTestLogLines[k], "WARNING: SetWindowPosition()") >= 0) continue; // Ignore specific warning
#endif
                    if (TextFindIndex(exTestLogLines[k], "WARNING") >= 0) testing[i].warnings++;
                }
                UnloadTextLines(exTestLogLines, exTestLogLinesCount);
                UnloadFileText(exTestLog);
            }
            //---------------------------------------------------------------------------------------------

            // STEP 5: Generate testing report/table with results (.md)
            //-----------------------------------------------------------------------------------------------------
#if defined(BUILD_TESTING_WEB)
            const char *osName = "Web";
#else
    #if defined(PLATFORM_DRM)
            const char *osName = "DRM";
    #elif defined(PLATFORM_DESKTOP)
        #if defined(_WIN32)
            const char *osName = "Windows";
        #elif defined(__linux__)
            const char *osName = "Linux";
        #elif defined(__FreeBSD__)
            const char *osName = "FreeBSD";
        #elif defined(__APPLE__)
            const char *osName = "macOS";
        #endif // Desktop OSs
    #endif
#endif
            /*
            Columns:
             - [CWARN]  : Compilation WARNING messages
             - [LWARN]  : Log WARNING messages count
             - [INIT]   : Initialization
             - [CLOSE]  : Closing
             - [ASSETS] : Assets loading
             - [RLGL]   : OpenGL-wrapped initialization
             - [PLAT]   : Platform initialization
             - [FONT]   : Font default initialization
             - [TIMER]  : Timer initialization

            | **EXAMPLE NAME**                 | [CWARN] | [LWARN] | [INIT] | [CLOSE] | [ASSETS] | [RLGL] | [PLAT] | [FONT] | [TIMER] |
            |:---------------------------------|:-------:|:-------:|:------:|:-------:|:--------:|:------:|:------:|:------:|:-------:|
            | core_basic window                |    0    |    0    |   ✔   |    ✔    |    ✔    |   ✔   |    ✔   |   ✔   |    ✔   |
            */
            LOG("INFO: [examples_testing_os.md] Generating examples testing report...\n");

            char *report = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1);

            int repIndex = 0;
            repIndex += sprintf(report + repIndex, "# EXAMPLES COLLECTION - TESTING REPORT\n\n");
            repIndex += sprintf(report + repIndex, TextFormat("## Tested Platform: %s\n\n", osName));

            repIndex += sprintf(report + repIndex, "```\nExample automated testing elements validated:\n");
            repIndex += sprintf(report + repIndex, " - [CWARN]  : Compilation WARNING messages\n");
            repIndex += sprintf(report + repIndex, " - [LWARN]  : Log WARNING messages count\n");
            repIndex += sprintf(report + repIndex, " - [INIT]   : Initialization\n");
            repIndex += sprintf(report + repIndex, " - [CLOSE]  : Closing\n");
            repIndex += sprintf(report + repIndex, " - [ASSETS] : Assets loading\n");
            repIndex += sprintf(report + repIndex, " - [RLGL]   : OpenGL-wrapped initialization\n");
            repIndex += sprintf(report + repIndex, " - [PLAT]   : Platform initialization\n");
            repIndex += sprintf(report + repIndex, " - [FONT]   : Font default initialization\n");
            repIndex += sprintf(report + repIndex, " - [TIMER]  : Timer initialization\n```\n");

            repIndex += sprintf(report + repIndex, "| **EXAMPLE NAME**                 | [CWARN] | [LWARN] | [INIT] | [CLOSE] | [ASSETS] | [RLGL] | [PLAT] | [FONT] | [TIMER] |\n");
            repIndex += sprintf(report + repIndex, "|:---------------------------------|:-------:|:-------:|:------:|:-------:|:--------:|:------:|:------:|:------:|:-------:|\n");

            /*
            TESTING_FAIL_INIT      = 1 << 0,   // Initialization (InitWindow())    -> "INFO: DISPLAY: Device initialized successfully"
            TESTING_FAIL_CLOSE     = 1 << 1,   // Closing (CloseWindow())          -> "INFO: Window closed successfully"
            TESTING_FAIL_ASSETS    = 1 << 2,   // Assets loading (WARNING: FILE:)  -> "WARNING: FILEIO:"
            TESTING_FAIL_RLGL      = 1 << 3,   // OpenGL-wrapped initialization    -> "INFO: RLGL: Default OpenGL state initialized successfully"
            TESTING_FAIL_PLATFORM  = 1 << 4,   // Platform initialization          -> "INFO: PLATFORM: DESKTOP (GLFW - Win32): Initialized successfully"
            TESTING_FAIL_FONT      = 1 << 5,   // Font default initialization      -> "INFO: FONT: Default font loaded successfully (224 glyphs)"
            TESTING_FAIL_TIMER     = 1 << 6,   // Timer initialization             -> "INFO: TIMER: Target time per frame: 16.667 milliseconds"
            */
            for (int i = 0; i < exBuildListCount; i++)
            {
                if ((testing[i].buildwarns > 0) || (testing[i].warnings > 0) || (testing[i].status > 0))
                {
                    repIndex += sprintf(report + repIndex, "| %-32s |    %i    |    %i    |   %s   |    %s    |   %s    |   %s   |   %s   |   %s   |   %s   |\n",
                        exBuildList[i],
                        testing[i].buildwarns,
                        testing[i].warnings,
                        (testing[i].status & TESTING_FAIL_INIT)? "❌" : "✔",
                        (testing[i].status & TESTING_FAIL_CLOSE)? "❌" : "✔",
                        (testing[i].status & TESTING_FAIL_ASSETS)? "❌" : "✔",
                        (testing[i].status & TESTING_FAIL_RLGL)? "❌" : "✔",
                        (testing[i].status & TESTING_FAIL_PLATFORM)? "❌" : "✔",
                        (testing[i].status & TESTING_FAIL_FONT)? "❌" : "✔",
                        (testing[i].status & TESTING_FAIL_TIMER)? "❌" : "✔");
                }
            }

            repIndex += sprintf(report + repIndex, "\n");

            SaveFileText(TextFormat("%s/../tools/rexm/reports/examples_testing_%s.md", exBasePath, TextToLower(osName)), report);

            RL_FREE(report);
            //-----------------------------------------------------------------------------------------------------

        } break;
        default:    // Help
        {
            // Supported commands:
            //    create <new_example_name>     : Creates an empty example, from internal template
            //    add <example_name>            : Add existing example, category extracted from name
            //    rename <old_examples_name> <new_example_name> : Rename an existing example
            //    remove <example_name>         : Remove an existing example
            //    build <example_name>          : Build example for Desktop and Web platforms
            //    validate                      : Validate examples collection, generates report
            //    update                        : Validate and update examples collection, generates report

            printf("\n////////////////////////////////////////////////////////////////////////////////////////////\n");
            printf("//                                                                                        //\n");
            printf("// rexm [raylib examples manager] - A simple command-line tool to manage raylib examples  //\n");
            printf("// powered by raylib v5.6-dev                                                             //\n");
            printf("//                                                                                        //\n");
            printf("// Copyright (c) 2025-2026 Ramon Santamaria (@raysan5)                                    //\n");
            printf("//                                                                                        //\n");
            printf("////////////////////////////////////////////////////////////////////////////////////////////\n\n");

            printf("USAGE:\n\n");
            printf("    > rexm <command> <example_name> [<example_rename>]\n\n");

            printf("COMMANDS:\n\n");
            printf("    create <new_example_name>     : Creates an empty example, from internal template\n");
            printf("    add <example_name>            : Add existing example, category extracted from name\n");
            printf("                                    Supported categories: core, shapes, textures, text, models\n");
            printf("    rename <old_examples_name> <new_example_name> : Rename an existing example\n");
            printf("    remove <example_name>         : Remove an existing example\n");
            printf("    build <example_name>          : Build example for Desktop and Web platforms\n");
            printf("    test <example_name>           : Build and Test example for Desktop and Web platforms\n");
            printf("    validate                      : Validate examples collection, generates report\n");
            printf("    update                        : Validate and update examples collection, generates report\n\n");
            printf("OPTIONS:\n\n");
            printf("    -h, --help                    : Show tool version and command line usage help\n");
            printf("    -v, --verbose                 : Verbose mode, show additional logs on processes\n");
            printf("\nEXAMPLES:\n\n");
            printf("    > rexm add shapes_custom_stars\n");
            printf("        Add and updates new example provided <shapes_custom_stars>\n\n");
            printf("    > rexm rename core_basic_window core_cool_window\n");
            printf("        Renames and updates example <core_basic_window> to <core_cool_window>\n\n");
            printf("    > rexm update\n");
            printf("        Validates all examples in collection and updates missing elements\n\n");
        } break;
    }

    return 0;
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------
// Update required files from examples collection
static int UpdateRequiredFiles(void)
{
    int result = 0;

    // Edit: Example source code metadata for consistency
    //------------------------------------------------------------------------------------------------
    LOG("INFO: Updating all examples metadata...\n");
    int exListCount = 0;
    rlExampleInfo *exList = LoadExampleData("ALL", true, &exListCount);
    for (int i = 0; i < exListCount; i++)
    {
        rlExampleInfo *info = &exList[i];
        UpdateSourceMetadata(TextFormat("%s/%s/%s.c", exBasePath, info->category, info->name), info);
    }
    UnloadExampleData(exList);
    //------------------------------------------------------------------------------------------------

    // Edit: raylib/examples/Makefile --> Update from collection
    //------------------------------------------------------------------------------------------------
    LOG("INFO: Updating raylib/examples/Makefile\n");
    char *mkText = LoadFileText(TextFormat("%s/Makefile", exBasePath));
    char *mkTextUpdated = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1); // Updated Makefile copy, 2MB

    int mkListStartIndex = TextFindIndex(mkText, "#EXAMPLES_LIST_START");
    int mkListEndIndex = TextFindIndex(mkText, "#EXAMPLES_LIST_END");

    int mkIndex = 0;
    memcpy(mkTextUpdated, mkText, mkListStartIndex);
    mkIndex = sprintf(mkTextUpdated + mkListStartIndex, "#EXAMPLES_LIST_START\n");

    for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES; i++)
    {
        mkIndex += sprintf(mkTextUpdated + mkListStartIndex + mkIndex, TextFormat("%s = \\\n", TextToUpper(exCategories[i])));

        int exCollectionCount = 0;
        rlExampleInfo *exCollection = LoadExampleData(exCategories[i], true, &exCollectionCount);

        for (int x = 0; x < exCollectionCount - 1; x++) mkIndex += sprintf(mkTextUpdated + mkListStartIndex + mkIndex, TextFormat("    %s/%s \\\n", exCollection[x].category, exCollection[x].name));
        mkIndex += sprintf(mkTextUpdated + mkListStartIndex + mkIndex, TextFormat("    %s/%s\n\n", exCollection[exCollectionCount - 1].category, exCollection[exCollectionCount - 1].name));

        UnloadExampleData(exCollection);
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
    LOG("INFO: Updating raylib/examples/Makefile.Web\n");
    char *mkwText = LoadFileText(TextFormat("%s/Makefile.Web", exBasePath));
    char *mkwTextUpdated = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1); // Updated Makefile copy, 2MB

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
        rlExampleInfo *exCollection = LoadExampleData(exCategories[i], true, &exCollectionCount);

        for (int x = 0; x < exCollectionCount - 1; x++) mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, TextFormat("    %s/%s \\\n", exCollection[x].category, exCollection[x].name));
        mkwIndex += sprintf(mkwTextUpdated + mkwListStartIndex + mkwIndex, TextFormat("    %s/%s\n\n", exCollection[exCollectionCount - 1].category, exCollection[exCollectionCount - 1].name));

        UnloadExampleData(exCollection);
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
        rlExampleInfo *exCollection = LoadExampleData(exCategories[i], true, &exCollectionCount);

        for (int x = 0; x < exCollectionCount; x++)
        {
            // Scan resources used in example to list
            int resPathCount = 0;
            char **resPaths = LoadExampleResourcePaths(TextFormat("%s/%s/%s.c", exBasePath, exCollection[x].category, exCollection[x].name), &resPathCount);

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

            UnloadExampleResourcePaths(resPaths);
        }

        UnloadExampleData(exCollection);
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
    LOG("INFO: Updating raylib/examples/README.md\n");
    // NOTE: Using [examples_list.txt] to update/regen README.md
    // Lines format: | 01 | [core_basic_window](core/core_basic_window.c) | <img src="core/core_basic_window.png" alt="core_basic_window" width="80"> | ⭐️☆☆☆ | 1.0 | 1.0 | [Ray](https://github.com/raysan5) |
    char *mdText = LoadFileText(TextFormat("%s/README.md", exBasePath));
    char *mdTextUpdated = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1); // Updated examples.js copy, 2MB

    int mdListStartIndex = TextFindIndex(mdText, "## EXAMPLES COLLECTION");

    int mdIndex = 0;
    memcpy(mdTextUpdated, mdText, mdListStartIndex);

    int exCollectionFullCount = 0;
    rlExampleInfo *exCollectionFull = LoadExampleData("ALL", false, &exCollectionFullCount);
    UnloadExampleData(exCollectionFull);

    mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, TextFormat("## EXAMPLES COLLECTION [TOTAL: %i]\n", exCollectionFullCount));

    // NOTE: We keep a global examples counter
    for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES; i++)
    {
        int exCollectionCount = 0;
        rlExampleInfo *exCollection = LoadExampleData(exCategories[i], false, &exCollectionCount);

        // Every category includes some introductory text, as it is quite short, just copying it here
        if (i == 0)         // "core"
        {
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex, TextFormat("\n### category: core [%i]\n\n", exCollectionCount));
            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                "Examples using raylib [core](../src/rcore.c) module platform functionality: window creation, inputs, drawing modes and system functionality.\n\n");
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

        char starsText[16] = { 0 };
        for (int x = 0; x < exCollectionCount; x++)
        {
            for (int s = 0; s < 4; s++)
            {
                if (s < exCollection[x].stars) strcpy(starsText + 3*s, "⭐️"); // WARNING: Different than '★', more visual
                else strcpy(starsText + 3*s, "☆");
            }

            mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
                TextFormat("| [%s](%s/%s.c) | <img src=\"%s/%s.png\" alt=\"%s\" width=\"80\"> | %s | %s | %s | [%s](https://github.com/%s) |\n",
                    exCollection[x].name, exCollection[x].category, exCollection[x].name, exCollection[x].category, exCollection[x].name, exCollection[x].name,
                    starsText, exCollection[x].verCreated, exCollection[x].verUpdated, exCollection[x].author, exCollection[x].authorGitHub));
        }

        UnloadExampleData(exCollection);
    }

    mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
        "\nSome example missing? As always, contributions are welcome, feel free to send new examples!\n");
    mdIndex += sprintf(mdTextUpdated + mdListStartIndex + mdIndex,
        "Here is an [examples template](examples_template.c) with instructions to start with!\n");

    // Save updated file
    SaveFileText(TextFormat("%s/README.md", exBasePath), mdTextUpdated);
    UnloadFileText(mdText);
    RL_FREE(mdTextUpdated);
    //------------------------------------------------------------------------------------------------

    // Edit: raylib.com/common/examples.js --> Update from collection
    // NOTE: Entries format: exampleEntry('⭐️☆☆☆' , 'core'    , 'basic_window'),
    //------------------------------------------------------------------------------------------------
    LOG("INFO: Updating raylib.com/common/examples.js\n");
    char *jsText = LoadFileText(TextFormat("%s/../common/examples.js", exWebPath));
    if (!jsText)
    {
        LOG("INFO: examples.js not found, skipping web examples list update\n");
    }
    else
    {
        int jsListStartIndex = TextFindIndex(jsText, "//EXAMPLE_DATA_LIST_START");
        int jsListEndIndex = TextFindIndex(jsText, "//EXAMPLE_DATA_LIST_END");
        if ((jsListStartIndex < 0) || (jsListEndIndex < 0))
        {
            LOG("WARNING: examples.js markers not found, skipping update\n");
            UnloadFileText(jsText);
        }
        else
        {
            char *jsTextUpdated = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1); // Updated examples.js copy, 2MB
            int jsIndex = 0;
            memcpy(jsTextUpdated, jsText, jsListStartIndex);
            jsIndex = sprintf(jsTextUpdated + jsListStartIndex, "//EXAMPLE_DATA_LIST_START\n");
            jsIndex += sprintf(jsTextUpdated + jsListStartIndex + jsIndex, "    var exampleData = [\n");

            char starsText[16] = { 0 };

            // NOTE: We avoid "others" category
            for (int i = 0; i < REXM_MAX_EXAMPLE_CATEGORIES - 1; i++)
            {
                int exCollectionCount = 0;
                rlExampleInfo *exCollection = LoadExampleData(exCategories[i], false, &exCollectionCount);
                for (int x = 0; x < exCollectionCount; x++)
                {
                    for (int s = 0; s < 4; s++)
                    {
                        if (s < exCollection[x].stars) strcpy(starsText + 3*s, "⭐️"); // WARNING: Different than '★', more visual
                        else strcpy(starsText + 3*s, "☆");
                    }

                    if ((i == 6) && (x == (exCollectionCount - 1)))
                    {
                        // NOTE: Last line to add, special case to consider
                        jsIndex += sprintf(jsTextUpdated + jsListStartIndex + jsIndex,
                            TextFormat("        exampleEntry('%s', '%s', '%s')];\n", starsText, exCollection[x].category, exCollection[x].name + strlen(exCollection[x].category) + 1));
                    }
                    else
                    {
                        jsIndex += sprintf(jsTextUpdated + jsListStartIndex + jsIndex,
                            TextFormat("        exampleEntry('%s', '%s', '%s'),\n", starsText, exCollection[x].category, exCollection[x].name + strlen(exCollection[x].category) + 1));
                    }
                }

                UnloadExampleData(exCollection);
            }

            // Add the remaining part of the original file
            memcpy(jsTextUpdated + jsListStartIndex + jsIndex, jsText + jsListEndIndex, strlen(jsText) - jsListEndIndex);

            // Save updated file
            SaveFileText(TextFormat("%s/../common/examples.js", exWebPath), jsTextUpdated);
            UnloadFileText(jsText);
            RL_FREE(jsTextUpdated);
        }
    }
    //------------------------------------------------------------------------------------------------

    return result;
}

// Load examples information from collection data
static rlExampleInfo *LoadExampleData(const char *filter, bool sort, int *exCount)
{
    #define MAX_EXAMPLES_INFO   256

    rlExampleInfo *exInfo = (rlExampleInfo *)RL_CALLOC(MAX_EXAMPLES_INFO, sizeof(rlExampleInfo));
    int exCounter = 0;
    *exCount = 0;

    // Load main collection list file: "raylib/examples/examples_list.txt"
    char *text = LoadFileText(exCollectionFilePath);

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
                 (lines[i][0] == 'o')))       // TODO: Get others category?
            {
                rlExampleInfo info = { 0 };
                int result = ParseExampleInfoLine(lines[i], &info);
                if (result == 1) // Success on parsing
                {
                    if (strcmp(filter, "ALL") == 0)
                    {
                        // Add all examples to the list
                        memcpy(&exInfo[exCounter], &info, sizeof(rlExampleInfo));
                        exCounter++;
                    }
                    else if (strcmp(info.category, filter) == 0)
                    {
                        // Get only specific category examples
                        memcpy(&exInfo[exCounter], &info, sizeof(rlExampleInfo));
                        exCounter++;
                    }
                    else if (strcmp(info.name, filter) == 0)
                    {
                        // Get only requested example
                        memcpy(&exInfo[exCounter], &info, sizeof(rlExampleInfo));
                        exCounter++;
                        break;
                    }
                }
            }
        }

        UnloadTextLines(lines, lineCount);
        UnloadFileText(text);
    }

    // Sorting required
    if (sort) SortExampleByName(exInfo, exCounter);

    *exCount = exCounter;
    return exInfo;
}

// Unload examples collection data
static void UnloadExampleData(rlExampleInfo *exInfo)
{
    RL_FREE(exInfo);
}

// Get example info from example file header
// WARNING: Expecting the example to follow raylib_example_template.c
static rlExampleInfo *LoadExampleInfo(const char *exFileName)
{
    rlExampleInfo *exInfo = NULL;

    if (FileExists(exFileName) && IsFileExtension(exFileName, ".c"))
    {
        // Example found in collection
        exInfo = (rlExampleInfo *)RL_CALLOC(1, sizeof(rlExampleInfo));

        strncpy(exInfo->name, GetFileNameWithoutExt(exFileName), 128 - 1);
        strncpy(exInfo->category, exInfo->name, TextFindIndex(exInfo->name, "_"));

        char *exText = LoadFileText(exFileName);

        // Get example difficulty stars
        // NOTE: Counting the unicode char occurrences: ★
        int starsIndex = TextFindIndex(exText, "★");
        if (starsIndex > 0)
        {
            const char *starPtr = exText + starsIndex;
            while (*starPtr)
            {
                if (((unsigned char)starPtr[0] == 0xe2) &&
                    ((unsigned char)starPtr[1] == 0x98) &&
                    ((unsigned char)starPtr[2] == 0x85))
                {
                    exInfo->stars++;
                    starPtr += 3; // Advance past multibyte character
                }
                else starPtr++;
            }
        }

        // Get example create with raylib version
        int verCreateIndex = TextFindIndex(exText, "created with raylib "); // Version = index + 20
        int verCreatedLen = 0;
        for (int i = verCreateIndex + 20; (exText[i] != ' ') && (exText[i] != '\n') && (exText[i] != ','); i++) verCreatedLen++;
        if (verCreateIndex > 0) strncpy(exInfo->verCreated, exText + verCreateIndex + 20, verCreatedLen);
        else strcpy(exInfo->verCreated, RAYLIB_VERSION); // Use current raylib version

        // Get example update with raylib version
        int verUpdateIndex = TextFindIndex(exText, "updated with raylib "); // Version = index + 20
        int verUpdateLen = 0;
        for (int i = verUpdateIndex + 20; (exText[i] != ' ') && (exText[i] != '\n') && (exText[i] != ','); i++) verUpdateLen++;
        if (verUpdateIndex > 0) strncpy(exInfo->verUpdated, exText + verUpdateIndex + 20, verUpdateLen);
        else strcpy(exInfo->verUpdated, RAYLIB_VERSION); // Use current raylib version

        // Get example years created/reviewed and creator and github user
        // NOTE: Using copyright line instead of "Example contributed by " because
        // most examples do not contain that line --> TODO: Review examples header formating?
        // Expected format: Copyright (c) <year_created>-<year_updated> <user_name> (@<user_github>)
        // Alternatives:  Copyright (c) <year_created> <author_name> (@<user_github>) and <contrib_name> (@<contrib_user>)
        int copyrightIndex = TextFindIndex(exText, "Copyright (c) ");
        int yearStartIndex = copyrightIndex + 14;
        char yearText[5] = { 0 };
        strncpy(yearText, exText + yearStartIndex, 4);
        exInfo->yearCreated = TextToInteger(yearText);
        // Check for review year included (or just use creation year)
        if (exText[yearStartIndex + 4] == '-') strncpy(yearText, exText + yearStartIndex + 5, 4);
        exInfo->yearReviewed = TextToInteger(yearText);
        int yearEndIndex = TextFindIndex(exText + yearStartIndex, " ");
        int authorStartIndex = yearStartIndex + yearEndIndex + 1;
        int authorEndIndex = TextFindIndex(exText + authorStartIndex, " (@");
        if (authorEndIndex != -1) // Github user also available
        {
            authorEndIndex += authorStartIndex;
            strncpy(exInfo->author, exText + authorStartIndex, authorEndIndex - authorStartIndex);

            // Get GitHub user
            int userStartIndex = authorEndIndex + 3;
            int userEndIndex = TextFindIndex(exText + userStartIndex, ")");
            userEndIndex += userStartIndex;
            strncpy(exInfo->authorGitHub, exText + userStartIndex, userEndIndex - userStartIndex);
        }
        else // GitHub user not found to set end, using '\n'
        {
            authorEndIndex = TextFindIndex(exText + authorStartIndex, "\n");
            authorEndIndex += authorStartIndex;
            strncpy(exInfo->author, exText + authorStartIndex, authorEndIndex - authorStartIndex);
        }

        UnloadFileText(exText);

        exInfo->resPaths = LoadExampleResourcePaths(exFileName, &exInfo->resCount);
    }

    return exInfo;
}

// Unload example information
static void UnloadExampleInfo(rlExampleInfo *exInfo)
{
    UnloadExampleResourcePaths(exInfo->resPaths);
    RL_FREE(exInfo);
}

// raylib example line info parser
// Parses following line format: core;core_basic_window;★☆☆☆;1.0;1.0;2013;2026;"Ray";@raysan5
static int ParseExampleInfoLine(const char *line, rlExampleInfo *entry)
{
    #define MAX_EXAMPLE_INFO_LINE_LEN   512

    char temp[MAX_EXAMPLE_INFO_LINE_LEN] = { 0 };
    strncpy(temp, line, MAX_EXAMPLE_INFO_LINE_LEN);
    temp[MAX_EXAMPLE_INFO_LINE_LEN - 1] = '\0'; // Ensure null termination

    int tokenCount = 0;
    char **tokens = TextSplit(line, ';', &tokenCount);

    if (tokenCount != 9)
    {
        LOG("REXM: WARNING: Example collection line contains invalid number of tokens: %i\n", tokenCount);
    }

    // Get category and name
    strcpy(entry->category, tokens[0]);
    strcpy(entry->name, tokens[1]);

    // Parsing stars
    // NOTE: Counting the unicode char occurrences: ★
    const char *starPtr = tokens[2];
    while (*starPtr)
    {
        if (((unsigned char)starPtr[0] == 0xe2) &&
            ((unsigned char)starPtr[1] == 0x98) &&
            ((unsigned char)starPtr[2] == 0x85))
        {
            entry->stars++;
            starPtr += 3; // Advance past multibyte character
        }
        else starPtr++;
    }

    // Get raylib creation/update versions
    strcpy(entry->verCreated, tokens[3]);
    strcpy(entry->verUpdated, tokens[4]);

    // Get year created and year reviewed
    entry->yearCreated = TextToInteger(tokens[5]);
    entry->yearReviewed = TextToInteger(tokens[6]);

    // Get author and github
    if (tokens[7][0] == '"') tokens[7] += 1;
    if (tokens[7][strlen(tokens[7]) - 1] == '"') tokens[7][strlen(tokens[7]) - 1] = '\0';
    strcpy(entry->author, tokens[7]);
    strcpy(entry->authorGitHub, tokens[8] + 1); // Skip '@'

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
// TODO: WARNING: Some resources could require linked resources: .fnt --> .png, .mtl --> .png, .gltf --> .png, ...
static char **LoadExampleResourcePaths(const char *filePath, int *resPathCount)
{
    #define REXM_MAX_RESOURCE_PATH_LEN    256

    char **paths = (char **)RL_CALLOC(REXM_MAX_RESOURCE_PATHS, sizeof(char **));
    for (int i = 0; i < REXM_MAX_RESOURCE_PATHS; i++) paths[i] = (char *)RL_CALLOC(REXM_MAX_RESOURCE_PATH_LEN, sizeof(char));

    int resCounter = 0;
    char *code = LoadFileText(filePath);

    if (code != NULL)
    {
        // Resources extensions to check
        const char *exts[] = { ".png", ".bmp", ".jpg", ".qoi", ".gif", ".raw", ".hdr", ".ttf", ".fnt", ".wav", ".ogg", ".mp3", ".flac", ".mod", ".qoa", ".obj", ".iqm", ".glb", ".m3d", ".vox", ".vs", ".fs", ".txt" };
        const int extCount = sizeof(exts)/sizeof(char *);

        char *ptr = code;
        while ((ptr = strchr(ptr, '"')) != NULL)
        {
            char *start = ptr + 1;
            char *end = strchr(start, '"');
            if (!end) break;

            // WARNING: Some paths could be for saving files, not loading, those "resource" files must be omitted
            // TODO: HACK: Just check previous position from pointer for function name including the string and the index "distance"
            // This is a quick solution, the good one would be getting the data loading function names...
            int functionIndex01 = TextFindIndex(ptr - 40, "ExportImage");       // Check ExportImage()
            int functionIndex02 = TextFindIndex(ptr - 10, "TraceLog");          // Check TraceLog()
            int functionIndex03 = TextFindIndex(ptr - 40, "TakeScreenshot");    // Check TakeScreenshot()
            int functionIndex04 = TextFindIndex(ptr - 40, "SaveFileData");      // Check SaveFileData()
            int functionIndex05 = TextFindIndex(ptr - 40, "SaveFileText");      // Check SaveFileText()

            if (!((functionIndex01 != -1) && (functionIndex01 < 40)) &&  // Not found ExportImage() before ""
                !((functionIndex02 != -1) && (functionIndex02 < 10)) &&  // Not found TraceLog() before ""
                !((functionIndex03 != -1) && (functionIndex03 < 40)) &&  // Not found TakeScreenshot() before ""
                !((functionIndex04 != -1) && (functionIndex04 < 40)) &&  // Not found TakeScreenshot() before ""
                !((functionIndex05 != -1) && (functionIndex05 < 40)))    // Not found SaveFileText() before ""
            {
                int len = (int)(end - start);
                if ((len > 0) && (len < REXM_MAX_RESOURCE_PATH_LEN))
                {
                    char buffer[REXM_MAX_RESOURCE_PATH_LEN] = { 0 };
                    strncpy(buffer, start, len);
                    buffer[len] = '\0';

                    // Check for known extensions
                    for (int i = 0; i < extCount; i++)
                    {
                        // NOTE: IsFileExtension() expects a NULL terminated fileName string,
                        // it looks for the last '.' and checks "extension" after that
                        if (IsFileExtension(buffer, exts[i]))
                        {
                            // Avoid duplicates
                            bool found = false;
                            for (int j = 0; j < resCounter; j++)
                            {
                                if (strcmp(paths[j], buffer) == 0) { found = true; break; }
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
            }

            ptr = end + 1;
        }

        UnloadFileText(code);
    }

    *resPathCount = resCounter;
    return paths;
}

// Clear resource paths scanned
static void UnloadExampleResourcePaths(char **resPaths)
{
    for (int i = 0; i < REXM_MAX_RESOURCE_PATHS; i++) RL_FREE(resPaths[i]);

    RL_FREE(resPaths);
}

// Add VS project (.vcxproj) to existing VS solution (.sln)
// WARNING: Adding a .vcxproj to .sln can not be automated with:
//  - "dotnet" tool (C# projects only)
//  - "devenv" tool (no adding support, only building)
// It must be done manually editing the .sln file
static int AddVSProjectToSolution(const char *slnFile, const char *projFile, const char *category)
{
    int result = 0;

    // WARNING: Function uses extensively TextFormat(),
    // *projFile ptr could be overwriten after a while -> Use copied string

    // Generate unique UUID
    const char *uuid = GenerateUUIDv4();

    // Replace default UUID (core_basic_window) on project file by new one
    FileTextReplace(projFile, "0981CA98-E4A5-4DF1-987F-A41D09131EFC", uuid);

    char *slnText = LoadFileText(slnFile);
    char *slnTextUpdated = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1);

    // Add project to solution
    //----------------------------------------------------------------------------------------
    // Format: Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "<project_name>", "examples\<project_name>.vcxproj", "{<project_uuid>}"
    // NOTE: Find a position to insert new project: At the end of the projects list, same strategy as VS2022 "Add Project"
    int prjStartIndex = TextFindIndex(slnText, "Global");

    // Add new project info
    // WARNING: UUID can actually be duplicated and it still works...
    strncpy(slnTextUpdated, slnText, prjStartIndex);
    int offsetIndex = sprintf(slnTextUpdated + prjStartIndex,
        TextFormat("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%s\", \"examples\\%s\", \"{%s}\"\n",
            GetFileNameWithoutExt(projFile), GetFileName(projFile), uuid));
    offsetIndex += prjStartIndex;
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, "EndProject\n");
    //----------------------------------------------------------------------------------------

    // Update project config
    //----------------------------------------------------------------------------------------
    // Find position to add project config: At the end of global section, same strategy as VS2022 "Add Project"
    int projConfStartIndex = TextFindIndex(slnText, "GlobalSection(ProjectConfigurationPlatforms) = postSolution");
    strncpy(slnTextUpdated + offsetIndex, slnText + prjStartIndex, projConfStartIndex - prjStartIndex);
    offsetIndex += (projConfStartIndex - prjStartIndex);

    int projConfEndIndex = TextFindIndex(slnText + projConfStartIndex, "EndGlobalSection");
    projConfEndIndex += projConfStartIndex;

    strncpy(slnTextUpdated + offsetIndex, slnText + projConfStartIndex, projConfEndIndex - projConfStartIndex);
    offsetIndex += (projConfEndIndex - projConfStartIndex);

    // Add project config lines
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t{%s}.Debug.DLL|ARM64.ActiveCfg = Debug.DLL|ARM64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Debug.DLL|ARM64.Build.0 = Debug.DLL|ARM64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Debug.DLL|x64.ActiveCfg = Debug.DLL|x64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Debug.DLL|x64.Build.0 = Debug.DLL|x64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Debug.DLL|x86.ActiveCfg = Debug.DLL|Win32\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Debug.DLL|x86.Build.0 = Debug.DLL|Win32\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Debug|ARM64.ActiveCfg = Debug|ARM64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Debug|ARM64.Build.0 = Debug|ARM64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Debug|x64.ActiveCfg = Debug|x64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Debug|x64.Build.0 = Debug|x64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Debug|x86.ActiveCfg = Debug|Win32\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Debug|x86.Build.0 = Debug|Win32\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release.DLL|ARM64.ActiveCfg = Release.DLL|ARM64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release.DLL|ARM64.Build.0 = Release.DLL|ARM64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release.DLL|x64.ActiveCfg = Release.DLL|x64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release.DLL|x64.Build.0 = Release.DLL|x64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release.DLL|x86.ActiveCfg = Release.DLL|Win32\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release.DLL|x86.Build.0 = Release.DLL|Win32\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release|ARM64.ActiveCfg = Release|ARM64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release|ARM64.Build.0 = Release|ARM64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release|x64.ActiveCfg = Release|x64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release|x64.Build.0 = Release|x64\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release|x86.ActiveCfg = Release|Win32\n", uuid));
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, TextFormat("\t\t{%s}.Release|x86.Build.0 = Release|Win32\n", uuid));
    // Write next section directly to avoid copy logic
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, "\tEndGlobalSection\n");
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, "\tGlobalSection(SolutionProperties) = preSolution\n");
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, "\t\tHideSolutionNode = FALSE\n");
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, "\tEndGlobalSection\n\t");
    //----------------------------------------------------------------------------------------

    // Place project to explorer folder
    //----------------------------------------------------------------------------------------
    // Find position to add project folder: At the end of global section, same strategy as VS2022 "Add Project"
    int projFolderStartIndex = TextFindIndex(slnText, "GlobalSection(NestedProjects) = preSolution");
    int projFolderEndIndex = TextFindIndex(slnText + projFolderStartIndex, "\tEndGlobalSection");
    projFolderEndIndex += projFolderStartIndex;

    strncpy(slnTextUpdated + offsetIndex, slnText + projFolderStartIndex, projFolderEndIndex - projFolderStartIndex);
    offsetIndex += (projFolderEndIndex - projFolderStartIndex);

    // Add project folder line
    // NOTE: Folder uuid depends on category
    if (strcmp(category, "core") == 0) offsetIndex += sprintf(slnTextUpdated + offsetIndex, 
        TextFormat("\t\t{%s} = {6C82BAAE-BDDF-457D-8FA8-7E2490B07035}\n", uuid));
    else if (strcmp(category, "shapes") == 0) offsetIndex += sprintf(slnTextUpdated + offsetIndex, 
        TextFormat("\t\t{%s} = {278D8859-20B1-428F-8448-064F46E1F021}\n", uuid));
    else if (strcmp(category, "textures") == 0) offsetIndex += sprintf(slnTextUpdated + offsetIndex, 
        TextFormat("\t\t{%s} = {DA049009-21FF-4AC0-84E4-830DD1BCD0CE}\n", uuid));
    else if (strcmp(category, "text") == 0) offsetIndex += sprintf(slnTextUpdated + offsetIndex, 
        TextFormat("\t\t{%s} = {8D3C83B7-F1E0-4C2E-9E34-EE5F6AB2502A}\n", uuid));
    else if (strcmp(category, "models") == 0) offsetIndex += sprintf(slnTextUpdated + offsetIndex, 
        TextFormat("\t\t{%s} = {AF5BEC5C-1F2B-4DA8-B12D-D09FE569237C}\n", uuid));
    else if (strcmp(category, "shaders") == 0) offsetIndex += sprintf(slnTextUpdated + offsetIndex, 
        TextFormat("\t\t{%s} = {5317807F-61D4-4E0F-B6DC-2D9F12621ED9}\n", uuid));
    else if (strcmp(category, "audio") == 0) offsetIndex += sprintf(slnTextUpdated + offsetIndex, 
        TextFormat("\t\t{%s} = {CC132A4D-D081-4C26-BFB9-AB11984054F8}\n", uuid));
    else if (strcmp(category, "other") == 0) offsetIndex += sprintf(slnTextUpdated + offsetIndex, 
        TextFormat("\t\t{%s} = {E9D708A5-9C1F-4B84-A795-C5F191801762}\n", uuid));
    else LOG("WARNING: Provided category is not valid: %s\n", category);
    //----------------------------------------------------------------------------------------

    // Write end of file, no need to copy from original file
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, "\tEndGlobalSection\n");
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, "\tGlobalSection(ExtensibilityGlobals) = postSolution\n");
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, "\t\tSolutionGuid = {E926C768-6307-4423-A1EC-57E95B1FAB29}\n");
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, "\tEndGlobalSection\n");
    offsetIndex += sprintf(slnTextUpdated + offsetIndex, "EndGlobal\n");

    SaveFileText(slnFile, slnTextUpdated);
    UnloadFileText(slnText);
    RL_FREE(slnTextUpdated);

    return result;
}

// Remove VS project (.vcxproj) to existing VS solution (.sln)
static int RemoveVSProjectFromSolution(const char *slnFile, const char *exName)
{
    int result = 0;

    // Lines to be removed from solution file:
    //Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "core_random_values", "examples\core_random_values.vcxproj", "{B332DCA8-3599-4A99-917A-82261BDC27AC}"
    //EndProject
    // All lines starting with:
    //"\t\t{B332DCA8-3599-4A99-917A-82261BDC27AC}."

    char *slnText = LoadFileText(slnFile);
    char *slnTextUpdated = (char *)RL_CALLOC(REXM_MAX_BUFFER_SIZE, 1);

    int lineCount = 0;
    char **lines = LoadTextLines(slnText, &lineCount); // WARNING: Max 512 lines, we need +4000!

    char uuid[38] = { 0 };
    strcpy(uuid, "ABCDEF00-0123-4567-89AB-000000000012"); // Temp value
    int textUpdatedOfsset = 0;
    int exNameLen = (int)strlen(exName);

    for (int i = 0, index = 0; i < lineCount; i++)
    {
        index = TextFindIndex(lines[i], exName);
        if (index > 0)
        {
            // Found line with project --> get UUID
            strncpy(uuid, lines[i] + index + exNameLen*2 + 26, 36);

            // Skip copying line and also next one
            i++;
        }
        else
        {
            if (TextFindIndex(lines[i], uuid) == -1)
                textUpdatedOfsset += sprintf(slnTextUpdated + textUpdatedOfsset, "%s\n", lines[i]);
        }
    }

    SaveFileText(slnFile, slnTextUpdated);

    UnloadTextLines(lines, lineCount);
    UnloadFileText(slnText);
    RL_FREE(slnTextUpdated);

    return result;
}

// Generate unique UUID v4 string
// Output format: {9A2F48CC-0DA8-47C0-884E-02E37F9BE6C1}
static const char *GenerateUUIDv4(void)
{
    static char uuid[38] = { 0 };
    memset(uuid, 0, 38);

    unsigned char bytes[16] = { 0 };

    for (int i = 0; i < 16; i++) bytes[i] = (unsigned char)GetRandomValue(0, 255);

    // Set version (4) and variant (RFC 4122)
    bytes[6] = (bytes[6] & 0x0F) | 0x40; // Version
    bytes[8] = (bytes[8] & 0x3F) | 0x80; // Variant

    snprintf(uuid, 38,
        "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        bytes[0], bytes[1], bytes[2], bytes[3],
        bytes[4], bytes[5],
        bytes[6], bytes[7],
        bytes[8], bytes[9],
        bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);

    return uuid;
}

// Update source code header and comments metadata
static void UpdateSourceMetadata(const char *exSrcPath, const rlExampleInfo *info)
{
    if (FileExists(exSrcPath) && IsFileExtension(exSrcPath, ".c") && (!TextIsEqual(info->category, "others")))
    {
        // WARNING: Cache a copy of exSrcPath to avoid modifications by TextFormat()
        char exSourcePath[512] = { 0 };
        strcpy(exSourcePath, exSrcPath);

        char *exText = LoadFileText(exSourcePath);
        char *exTextUpdated[6] = { 0 };     // Pointers to multiple updated text versions
        char *exTextUpdatedPtr = exText;    // Pointer to current valid text version

        char exNameFormated[256] = { 0 };   // Example name without category and using spaces
        int exNameIndex = TextFindIndex(info->name, "_");
        strcpy(exNameFormated, info->name + exNameIndex + 1);
        int exNameLen = (int)strlen(exNameFormated);
        for (int i = 0; i < exNameLen; i++) { if (exNameFormated[i] == '_') exNameFormated[i] = ' '; }

        // Update example header title (line #3 - ALWAYS)
        // String: "*   raylib [shaders] example - texture drawing"
        exTextUpdated[0] = TextReplaceBetween(exTextUpdatedPtr, "*   raylib [", "\n",
            TextFormat("%s] example - %s", info->category, exNameFormated));
        if (exTextUpdated[0] != NULL) exTextUpdatedPtr = exTextUpdated[0];

        // Update example complexity rating
        // String: "*   Example complexity rating: [★★☆☆] 2/4"
        // Get example difficulty stars text
        char starsText[16] = { 0 };
        for (int i = 0; i < 4; i++)
        {
            // NOTE: Every UTF-8 star are 3 bytes
            if (i < info->stars) strcpy(starsText + 3*i, "★");
            else strcpy(starsText + 3*i, "☆");
        }
        exTextUpdated[1] = TextReplaceBetween(exTextUpdatedPtr, "*   Example complexity rating: [", "/4\n",
            TextFormat("%s] %i", starsText, info->stars));
        if (exTextUpdated[1] != NULL) exTextUpdatedPtr = exTextUpdated[1];

        // Update example creation/update raylib versions
        // String: "*   Example originally created with raylib 2.0, last time updated with raylib 3.7
        exTextUpdated[2] = TextReplaceBetween(exTextUpdatedPtr, "*   Example originally created with raylib ", "\n",
            TextFormat("%s, last time updated with raylib %s", info->verCreated, info->verUpdated));
        if (exTextUpdated[2] != NULL) exTextUpdatedPtr = exTextUpdated[2];

        // Update copyright message
        // String: "*   Copyright (c) 2019-2026 Contributor Name (@github_user) and Ramon Santamaria (@raysan5)"
        if (info->yearCreated == info->yearReviewed)
        {
            exTextUpdated[3] = TextReplaceBetween(exTextUpdatedPtr, "Copyright (c) ", ")",
                TextFormat("%i %s (@%s", info->yearCreated, info->author, info->authorGitHub));
            if (exTextUpdated[3] != NULL) exTextUpdatedPtr = exTextUpdated[3];
        }
        else
        {
            exTextUpdated[3] = TextReplaceBetween(exTextUpdatedPtr, "Copyright (c) ", ")",
                TextFormat("%i-%i %s (@%s", info->yearCreated, info->yearReviewed, info->author, info->authorGitHub));
            if (exTextUpdated[3] != NULL) exTextUpdatedPtr = exTextUpdated[3];
        }

        // Update window title
        // String: "InitWindow(screenWidth, screenHeight, "raylib [shaders] example - texture drawing");"
        exTextUpdated[4] = TextReplaceBetween(exTextUpdated[3], "InitWindow(screenWidth, screenHeight, \"", "\");",
            TextFormat("raylib [%s] example - %s", info->category, exNameFormated));
        if (exTextUpdated[4] != NULL) exTextUpdatedPtr = exTextUpdated[4];

        // Update contributors names
        // String: "*   Example contributed by Contributor Name (@github_user) and reviewed by Ramon Santamaria (@raysan5)"
        // WARNING: Not all examples are contributed by someone, so the result of this replace can be NULL (string not found)
        exTextUpdated[5] = TextReplaceBetween(exTextUpdatedPtr, "*   Example contributed by ", ")",
            TextFormat("%s (@%s", info->author, info->authorGitHub));
        if (exTextUpdated[5] != NULL) exTextUpdatedPtr = exTextUpdated[5];

        if (exTextUpdatedPtr != NULL) SaveFileText(exSourcePath, exTextUpdatedPtr);

        for (int i = 0; i < 6; i++) { MemFree(exTextUpdated[i]); exTextUpdated[i] = NULL; }

        UnloadFileText(exText);
    }
}

// Update generated Web example .html file metadata
static void UpdateWebMetadata(const char *exHtmlPath, const char *exFilePath)
{
    if (FileExists(exHtmlPath) && IsFileExtension(exHtmlPath, ".html"))
    {
        // WARNING: Cache a copy of exHtmlPath to avoid modifications by TextFormat()
        char exHtmlPathCopy[512] = { 0 };
        strcpy(exHtmlPathCopy, exHtmlPath);

        char *exHtmlText = LoadFileText(exHtmlPathCopy);
        char *exHtmlTextUpdated[6] = { 0 }; // Pointers to multiple updated text versions

        char exName[64] = { 0 };            // Example name: fileName without extension
        char exCategory[16] = { 0 };        // Example category: core, shapes, text, textures, models, audio, shaders
        char exDescription[256] = { 0 };    // Example description: example text line #3
        char exTitle[64] = { 0 };           // Example title: fileName without extension, replacing underscores by spaces

        // Get example name: replace underscore by spaces
        strncpy(exName, GetFileNameWithoutExt(exHtmlPathCopy), 64 - 1);
        strcpy(exTitle, exName);
        for (int i = 0; (i < 64) && (exTitle[i] != '\0'); i++) { if (exTitle[i] == '_') exTitle[i] = ' '; }

        // Get example category from exName: copy until first underscore
        for (int i = 0; (exName[i] != '_'); i++) exCategory[i] = exName[i];

        // Get example description: copy line #3 from example file
        char *exText = LoadFileText(exFilePath);
        int lineCount = 0;
        char **lines = LoadTextLines(exText, &lineCount);
        int lineLength = (int)strlen(lines[2]);
        strncpy(exDescription, lines[2] + 4, lineLength - 4);
        UnloadTextLines(lines, lineCount);
        UnloadFileText(exText);

        // Update example.html required text
        exHtmlTextUpdated[0] = TextReplace(exHtmlText, "raylib web game", exTitle);
        exHtmlTextUpdated[1] = TextReplace(exHtmlTextUpdated[0], "New raylib web videogame, developed using raylib videogames library", exDescription);
        exHtmlTextUpdated[2] = TextReplace(exHtmlTextUpdated[1], "https://www.raylib.com/common/raylib_logo.png",
            TextFormat("https://raw.githubusercontent.com/raysan5/raylib/master/examples/%s/%s.png", exCategory, exName));
        exHtmlTextUpdated[3] = TextReplace(exHtmlTextUpdated[2], "https://www.raylib.com/games.html",
            TextFormat("https://www.raylib.com/examples/%s/%s.html", exCategory, exName));
        exHtmlTextUpdated[4] = TextReplace(exHtmlTextUpdated[3], "raylib - example", TextFormat("raylib - %s", exName)); // og:site_name
        exHtmlTextUpdated[5] = TextReplace(exHtmlTextUpdated[4], "https://github.com/raysan5/raylib",
            TextFormat("https://github.com/raysan5/raylib/blob/master/examples/%s/%s.c", exCategory, exName));

        SaveFileText(exHtmlPathCopy, exHtmlTextUpdated[5]);

        //LOG("INFO: [%s] Updated successfully\n",files.paths[i]);
        //LOG("      - Name / Title: %s / %s\n", exName, exTitle);
        //LOG("      - Description:  %s\n", exDescription);
        //LOG("      - URL:          %s\n", TextFormat("https://www.raylib.com/examples/%s/%s.html", exCategory, exName));
        //LOG("      - URL Source:   %s\n", TextFormat("https://github.com/raysan5/raylib/blob/master/examples/%s/%s.c", exCategory, exName));

        for (int i = 0; i < 6; i++) { MemFree(exHtmlTextUpdated[i]); exHtmlTextUpdated[i] = NULL; }

        UnloadFileText(exHtmlText);
    }
}

// Check if text string is a list of strings
static int GetTextListIndex(const char *text, const char **list, int listCount)
{
    int result = -1;

    for (int i = 0; i < listCount; i++)
    {
        if (TextIsEqual(text, list[i])) { result = i; break; }
    }

    return result;
}

#if defined(__GNUC__) // GCC and Clang
    #pragma GCC diagnostic pop
#endif
