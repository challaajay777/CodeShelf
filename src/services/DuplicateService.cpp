#include "DuplicateService.h"
#include "../core/ContentHash.h"

namespace CodeShelf {

DuplicateService::DuplicateService(SnippetRepository& repo)
    : m_repo(repo)
{
}

QVector<DuplicateMatch> DuplicateService::findDuplicatesForSnippet(const Snippet& target, bool checkNormalized) {
    QVector<DuplicateMatch> matches;
    const QString exactHash = ContentHash::computeExactHash(target.body());
    const QString normHash = checkNormalized ? ContentHash::computeNormalizedHash(target.body()) : QString();

    const auto all = m_repo.getAllActive();
    for (const Snippet& snip : all) {
        if (snip.id() == target.id()) continue;

        if (snip.contentHash() == exactHash) {
            DuplicateMatch match;
            match.snippet = snip;
            match.kind = DuplicateMatchKind::ExactHash;
            match.explanation = QStringLiteral("Identical code content (exact SHA-256 hash match).");
            matches.append(match);
        } else if (checkNormalized) {
            const QString snipNormHash = ContentHash::computeNormalizedHash(snip.body());
            if (snipNormHash == normHash) {
                DuplicateMatch match;
                match.snippet = snip;
                match.kind = DuplicateMatchKind::NormalizedWhitespace;
                match.explanation = QStringLiteral("Equivalent code content (ignoring line endings and trailing whitespace).");
                matches.append(match);
            }
        }
    }

    return matches;
}

QVector<QPair<Snippet, Snippet>> DuplicateService::scanEntireLibraryForDuplicates(bool checkNormalized) {
    QVector<QPair<Snippet, Snippet>> dupPairs;
    const auto all = m_repo.getAllActive();
    QSet<QString> seenIds;

    for (int i = 0; i < all.size(); ++i) {
        for (int j = i + 1; j < all.size(); ++j) {
            bool isDup = false;
            if (all[i].contentHash() == all[j].contentHash()) {
                isDup = true;
            } else if (checkNormalized) {
                if (ContentHash::computeNormalizedHash(all[i].body()) == ContentHash::computeNormalizedHash(all[j].body())) {
                    isDup = true;
                }
            }
            if (isDup) {
                dupPairs.append(qMakePair(all[i], all[j]));
            }
        }
    }
    return dupPairs;
}

} // namespace CodeShelf
