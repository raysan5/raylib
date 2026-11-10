// Headless tests for the file-path helpers in rcore.c
// (GetFileName, GetFileExtension, IsFileExtension, GetFileNameWithoutExt, IsPathAbsolute)
//
// Regression context: raylib 6.2 CHANGELOG lists fixes in this area
//   - GetFileExtension() returning the name of hidden files (PR #6326)
//   - IsPathAbsolute() added in 6.2 and later changed to drop the isalpha() dependency

#include "raylib.h"
#include "rtest.h"

static void test_get_file_name(void)
{
    CHECK_STR("c.txt", GetFileName("a/b/c.txt"));
    CHECK_STR("c.txt", GetFileName("a\\b\\c.txt"));      // Both separators are handled
    CHECK_STR("c.txt", GetFileName("c.txt"));
    CHECK_STR("", GetFileName("a/b/"));                  // Directory path -> empty name
    CHECK_STR(NULL, GetFileName(NULL));
}

static void test_get_file_extension(void)
{
    CHECK_STR(".png", GetFileExtension("sprite.png"));
    CHECK_STR(".png", GetFileExtension("assets/img/sprite.png"));
    CHECK_STR(".gz", GetFileExtension("archive.tar.gz"));        // Only the last extension
    CHECK_STR(NULL, GetFileExtension("README"));                 // No extension
    CHECK_STR(NULL, GetFileExtension(".gitignore"));             // Hidden file is not an extension (#6326)
    CHECK_STR(NULL, GetFileExtension("project/.gitignore"));     // Same, with a directory prefix (#6326)
    CHECK_STR(NULL, GetFileExtension("my.dir/README"));          // Dot in a directory name only
    CHECK_STR(".txt", GetFileExtension("project/.hidden/a.txt"));
}

static void test_is_file_extension(void)
{
    CHECK(IsFileExtension("sprite.png", ".png"));
    CHECK(IsFileExtension("SPRITE.PNG", ".png"));                // Case-insensitive
    CHECK(IsFileExtension("sound.wav", ".png;.wav;.ogg"));       // ';' separated list
    CHECK(!IsFileExtension("sprite.png", ".wav"));
    CHECK(!IsFileExtension("README", ".png"));
}

static void test_file_name_without_ext(void)
{
    CHECK_STR("sprite", GetFileNameWithoutExt("assets/sprite.png"));
    CHECK_STR("archive.tar", GetFileNameWithoutExt("archive.tar.gz"));
}

static void test_is_path_absolute(void)
{
    CHECK(!IsPathAbsolute(NULL));
    CHECK(!IsPathAbsolute(""));
    CHECK(!IsPathAbsolute("relative/path.txt"));
    CHECK(!IsPathAbsolute("./relative.txt"));

#if defined(_WIN32)
    CHECK(IsPathAbsolute("C:\\Users\\raylib"));
    CHECK(IsPathAbsolute("d:/games/raylib"));                    // Lowercase drive + forward slash
    CHECK(IsPathAbsolute("\\\\server\\share\\file.txt"));        // UNC path
    CHECK(!IsPathAbsolute("C:"));                                // Drive without path is not absolute
    CHECK(!IsPathAbsolute("C:file.txt"));                        // Drive-relative path
#else
    CHECK(IsPathAbsolute("/usr/lib"));
    CHECK(IsPathAbsolute("/"));
#endif
}

int main(void)
{
    SetTraceLogLevel(LOG_NONE);

    RUN(test_get_file_name);
    RUN(test_get_file_extension);
    RUN(test_is_file_extension);
    RUN(test_file_name_without_ext);
    RUN(test_is_path_absolute);

    return RTEST_SUMMARY();
}
