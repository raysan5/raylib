// Headless tests for the hash/encoding helpers in rcore.c
// (ComputeSHA256, ComputeSHA1, ComputeMD5, ComputeCRC32, EncodeDataBase64, DecodeDataBase64)
//
// Expected values come from an INDEPENDENT oracle (.NET System.Security.Cryptography) over the
// message 'a' repeated N times, so tests do not depend on raylib's own implementation.
// Lengths 55/56/57/59/60/63/64/65/119/120/121 are the padding boundaries of 512-bit blocks, where
// ComputeSHA256 was wrong before raylib 6.2 (issue #6143 / PR #6144: "wrong hash when message length mod 64 is 56..59").

#include "raylib.h"
#include "rtest.h"

#include <stdlib.h>

typedef struct { int len; const char *sha256; const char *sha1; const char *md5; } HashVector;

static const HashVector vectors[] = {
    {   0, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", "da39a3ee5e6b4b0d3255bfef95601890afd80709", "d41d8cd98f00b204e9800998ecf8427e" },
    {   1, "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb", "86f7e437faa5a7fce15d1ddcb9eaeaea377667b8", "0cc175b9c0f1b6a831c399e269772661" },
    {   3, "9834876dcfb05cb167a5c24953eba58c4ac89b1adf57f28f2f9d09af107ee8f0", "7e240de74fb1ed08fa08d38063f6a6a91462a815", "47bce5c74f589f4867dbd57e9ca9f808" },
    {  55, "9f4390f8d30c2dd92ec9f095b65e2b9ae9b0a925a5258e241c9f1e910f734318", "c1c8bbdc22796e28c0e15163d20899b65621d65a", "ef1772b6dff9a122358552954ad0df65" },
    {  56, "b35439a4ac6f0948b6d6f9e3c6af0f5f590ce20f1bde7090ef7970686ec6738a", "c2db330f6083854c99d4b5bfb6e8f29f201be699", "3b0c8ac703f828b04c6c197006d17218" },
    {  57, "f13b2d724659eb3bf47f2dd6af1accc87b81f09f59f2b75e5c0bed6589dfe8c6", "f08f24908d682555111be7ff6f004e78283d989a", "652b906d60af96844ebd21b674f35e93" },
    {  59, "111bb261277afd65f0744b247cd3e47d386d71563d0ed995517807d5ebd4fba3", "dbc8b8f59ff85a2b1448ed873484b14bf0507246", "762fc2665994b217c52c3c2eb7d9f406" },
    {  60, "11ee391211c6256460b6ed375957fadd8061cafbb31daf967db875aebd5aaad4", "13d956033d9af449bfe2c4ef78c17c20469c4bf1", "cc7ed669cf88f201c3297c6a91e1d18d" },
    {  63, "7d3e74a05d7db15bce4ad9ec0658ea98e3f06eeecf16b4c6fff2da457ddc2f34", "03f09f5b158a7a8cdad920bddc29b81c18a551f5", "b06521f39153d618550606be297466d5" },
    {  64, "ffe054fe7ae0cb6dc65c3af9b61d5209f439851db43d0ba5997337df154668eb", "0098ba824b5c16427bd7a1122a5a442a25ec644d", "014842d480b571495a4a0363793f7367" },
    {  65, "635361c48bb9eab14198e76ea8ab7f1a41685d6ad62aa9146d301d4f17eb0ae0", "11655326c708d70319be2610e8a57d9a5b959d3b", "c743a45e0d2e6a95cb859adae0248435" },
    { 119, "31eba51c313a5c08226adf18d4a359cfdfd8d2e816b13f4af952f7ea6584dcfb", "ee971065aaa017e0632a8ca6c77bb3bf8b1dfc56", "8a7bd0732ed6a28ce75f6dabc90e1613" },
    { 120, "2f3d335432c70b580af0e8e1b3674a7c020d683aa5f73aaaedfdc55af904c21c", "f34c1488385346a55709ba056ddd08280dd4c6d6", "5f61c0ccad4cac44c75ff505e1f1e537" },
    { 121, "e9615320128cc7a3d6078e9af05603188e5ccbf0d07d8b735d3df5e8e0c1281f", "fa6b5a6f8ac27182f838fe7841ec6d2aef3ade29", "f6acfca2d47c87f2b14ca038234d3614" },
};

// Message used by every vector: 'a' repeated len times
static unsigned char *MakeMessage(int len)
{
    unsigned char *msg = (unsigned char *)malloc(len + 1);
    memset(msg, 'a', len);
    msg[len] = 0;
    return msg;
}

// SHA-1/SHA-256 return 32-bit words holding the digest in big-endian reading order -> print each word as %08x
static void WordsToHex(const unsigned int *words, int count, char *out)
{
    for (int i = 0; i < count; i++) sprintf(out + i*8, "%08x", words[i]);
}

// MD5 returns int[4] whose bytes in memory ARE the digest (little-endian words) -> print memory bytes
static void BytesToHex(const unsigned int *words, int byteCount, char *out)
{
    const unsigned char *bytes = (const unsigned char *)words;
    for (int i = 0; i < byteCount; i++) sprintf(out + i*2, "%02x", bytes[i]);
}

static void test_sha256_boundaries(void)
{
    char hex[65];

    for (size_t i = 0; i < sizeof(vectors)/sizeof(vectors[0]); i++)
    {
        unsigned char *msg = MakeMessage(vectors[i].len);
        WordsToHex(ComputeSHA256(msg, vectors[i].len), 8, hex);
        printf("  sha256 len=%-3d %s\n", vectors[i].len, (strcmp(hex, vectors[i].sha256) == 0)? "ok" : "MISMATCH");
        CHECK_STR(vectors[i].sha256, hex);
        free(msg);
    }
}

static void test_sha1_boundaries(void)
{
    char hex[41];

    for (size_t i = 0; i < sizeof(vectors)/sizeof(vectors[0]); i++)
    {
        unsigned char *msg = MakeMessage(vectors[i].len);
        WordsToHex(ComputeSHA1(msg, vectors[i].len), 5, hex);
        CHECK_STR(vectors[i].sha1, hex);
        free(msg);
    }
}

static void test_md5_boundaries(void)
{
    char hex[33];

    for (size_t i = 0; i < sizeof(vectors)/sizeof(vectors[0]); i++)
    {
        unsigned char *msg = MakeMessage(vectors[i].len);
        BytesToHex(ComputeMD5(msg, vectors[i].len), 16, hex);
        CHECK_STR(vectors[i].md5, hex);
        free(msg);
    }
}

static void test_crc32(void)
{
    // Standard CRC-32 check value (ISO-HDLC): CRC("123456789") == 0xCBF43926
    CHECK_INT(0xCBF43926u, ComputeCRC32((const unsigned char *)"123456789", 9));
    CHECK_INT(0x00000000u, ComputeCRC32((const unsigned char *)"", 0));
}

static void test_base64(void)
{
    // RFC 4648 section 10 test vectors
    static const char *plain[]   = { "", "f", "fo", "foo", "foob", "fooba", "foobar" };
    static const char *encoded[] = { "", "Zg==", "Zm8=", "Zm9v", "Zm9vYg==", "Zm9vYmE=", "Zm9vYmFy" };

    for (int i = 0; i < 7; i++)
    {
        int len = (int)strlen(plain[i]);
        int encLen = 0;
        char *enc = EncodeDataBase64((const unsigned char *)plain[i], len, &encLen);

        CHECK_STR(encoded[i], enc);

        int decLen = 0;
        unsigned char *dec = DecodeDataBase64(encoded[i], &decLen);
        CHECK_INT(len, decLen);
        CHECK(memcmp(dec, plain[i], len) == 0);

        MemFree(enc);
        MemFree(dec);
    }
}

int main(void)
{
    SetTraceLogLevel(LOG_NONE);     // Headless: no window is created, keep output clean

    RUN(test_sha256_boundaries);
    RUN(test_sha1_boundaries);
    RUN(test_md5_boundaries);
    RUN(test_crc32);
    RUN(test_base64);

    return RTEST_SUMMARY();
}
