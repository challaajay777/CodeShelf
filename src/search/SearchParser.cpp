#include "SearchParser.h"
#include <QRegularExpression>

namespace CodeShelf {

SearchQuery SearchParser::parse(const QString& input) {
    SearchQuery query;
    query.rawQuery = input;

    const QString text = input.trimmed();
    if (text.isEmpty()) {
        return query;
    }

    // Tokenize while respecting quotes: "exact phrase" or bare-word or key:value
    // Regex matches either:
    // 1) "([^"]+)"
    // 2) ([a-zA-Z0-9_-]+):"([^"]+)"
    // 3) ([a-zA-Z0-9_-]+):([^\s]+)
    // 4) ([^\s]+)
    static const QRegularExpression tokenRegex(QStringLiteral(R"raw((?:([a-zA-Z0-9_-]+):(?:"([^"]+)"|([^\s]+)))|(?:"([^"]+)")|([^\s]+))raw"));

    auto it = tokenRegex.globalMatch(text);
    while (it.hasNext()) {
        auto match = it.next();
        const QString key = match.captured(1).toLower();
        const QString quotedVal = match.captured(2);
        const QString bareVal = match.captured(3);
        const QString exactPhrase = match.captured(4);
        const QString bareWord = match.captured(5);

        if (!key.isEmpty()) {
            const QString val = (!quotedVal.isEmpty()) ? quotedVal : bareVal;
            if (key == QStringLiteral("lang") || key == QStringLiteral("language")) {
                query.languageFilter = val;
            } else if (key == QStringLiteral("tag")) {
                query.tagFilter = val;
            } else if (key == QStringLiteral("collection") || key == QStringLiteral("col")) {
                query.collectionFilter = val;
            } else if (key == QStringLiteral("type")) {
                query.typeFilter = stringToSnippetType(val);
            } else if (key == QStringLiteral("is")) {
                const QString v = val.toLower();
                if (v == QStringLiteral("fav") || v == QStringLiteral("favorite")) {
                    query.favoriteFilter = true;
                } else if (v == QStringLiteral("pin") || v == QStringLiteral("pinned")) {
                    query.pinnedFilter = true;
                } else if (v == QStringLiteral("trash") || v == QStringLiteral("deleted")) {
                    query.inTrash = true;
                }
            } else if (key == QStringLiteral("in")) {
                if (val.toLower() == QStringLiteral("trash")) {
                    query.inTrash = true;
                }
            } else {
                // Unknown operator - treat entire token as free term
                query.freeTerms.append(match.captured(0));
            }
        } else if (!exactPhrase.isEmpty()) {
            query.exactPhrases.append(exactPhrase);
        } else if (!bareWord.isEmpty()) {
            query.freeTerms.append(bareWord);
        }
    }

    return query;
}

} // namespace CodeShelf
