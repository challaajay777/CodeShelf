#include "test_content_hash.h"

namespace CodeShelf {

void TestContentHash::testExactHashConsistency() {
    const QString code = QStringLiteral("int main() {\n    return 0;\n}\n");
    const QString hash1 = ContentHash::computeExactHash(code);
    const QString hash2 = ContentHash::computeExactHash(code);

    QCOMPARE(hash1, hash2);
    QCOMPARE(hash1.length(), 64); // SHA-256 hex string length
}

void TestContentHash::testNormalizedHashDifferentLineEndings() {
    const QString lf = QStringLiteral("line1\nline2\nline3");
    const QString crlf = QStringLiteral("line1\r\nline2\r\nline3");

    // Exact hashes will differ because of \r\n vs \n
    QVERIFY(ContentHash::computeExactHash(lf) != ContentHash::computeExactHash(crlf));

    // Normalized hashes MUST match
    QCOMPARE(ContentHash::computeNormalizedHash(lf), ContentHash::computeNormalizedHash(crlf));
}

void TestContentHash::testNormalizedHashTrailingSpaces() {
    const QString clean = QStringLiteral("int a = 1;\nint b = 2;");
    const QString trailing = QStringLiteral("int a = 1;   \nint b = 2;\t\t\n\n");

    QCOMPARE(ContentHash::computeNormalizedHash(clean), ContentHash::computeNormalizedHash(trailing));
}

} // namespace CodeShelf
