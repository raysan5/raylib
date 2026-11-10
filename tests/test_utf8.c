// Headless tests for the UTF-8 helpers in rtext.c
// (GetCodepoint, GetCodepointCount, CodepointToUTF8)
//
// Regression test for PR #6348 "[rtext] Fix GetCodepoint() size on invalid sequences reading past the string end"
//   Before that fix, on a truncated sequence codepointSize could be larger than the bytes left before the '\0',
//   so a caller that advanced with text += codepointSize skipped the terminator and read out of bounds.

#include "raylib.h"
#include "rtest.h"

static void test_valid_sequences(void)
{
    int size = 0;

    CHECK_INT('A', GetCodepoint("A", &size));            CHECK_INT(1, size);
    CHECK_INT(0xE9, GetCodepoint("\xC3\xA9", &size));    CHECK_INT(2, size);          // U+00E9 (e acute)
    CHECK_INT(0x20AC, GetCodepoint("\xE2\x82\xAC", &size)); CHECK_INT(3, size);       // U+20AC (euro)
    CHECK_INT(0x1F600, GetCodepoint("\xF0\x9F\x98\x80", &size)); CHECK_INT(4, size);  // U+1F600 (emoji)
}

static void test_invalid_sequences_return_question_mark(void)
{
    int size = 0;

    CHECK_INT('?', GetCodepoint("\x80", &size));         // Lone continuation byte
    CHECK_INT('?', GetCodepoint("\xC3(", &size));        // Bad continuation byte
    CHECK_INT('?', GetCodepoint("\xE0\x80\x80", &size)); // Overlong encoding
    CHECK_INT('?', GetCodepoint("\xF5\x80\x80\x80", &size)); // Above U+10FFFF
}

static void test_invalid_size_stays_inside_string(void)
{
    // Truncated sequences: "\xC3", "\xE2\x82", "\xF0\x9F\x98" followed by the terminator.
    // A safe decoder never reports a size that goes beyond the terminating '\0' of the string.
    const char *truncated[] = { "\xC3", "\xE2\x82", "\xF0\x9F\x98" };

    for (int i = 0; i < 3; i++)
    {
        int size = 0;
        GetCodepoint(truncated[i], &size);
        CHECK(size <= (int)strlen(truncated[i]));
    }
}

static void test_codepoint_count(void)
{
    CHECK_INT(0, GetCodepointCount(""));
    CHECK_INT(5, GetCodepointCount("hello"));
    CHECK_INT(4, GetCodepointCount("a\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80")); // a, e acute, euro, emoji
}

static void test_codepoint_to_utf8(void)
{
    int size = 0;

    CHECK_STR("A", CodepointToUTF8('A', &size));                   CHECK_INT(1, size);
    CHECK_STR("\xC3\xA9", CodepointToUTF8(0xE9, &size));           CHECK_INT(2, size);
    CHECK_STR("\xE2\x82\xAC", CodepointToUTF8(0x20AC, &size));     CHECK_INT(3, size);
    CHECK_STR("\xF0\x9F\x98\x80", CodepointToUTF8(0x1F600, &size)); CHECK_INT(4, size);
}

int main(void)
{
    SetTraceLogLevel(LOG_NONE);

    RUN(test_valid_sequences);
    RUN(test_invalid_sequences_return_question_mark);
    RUN(test_invalid_size_stays_inside_string);
    RUN(test_codepoint_count);
    RUN(test_codepoint_to_utf8);

    return RTEST_SUMMARY();
}
