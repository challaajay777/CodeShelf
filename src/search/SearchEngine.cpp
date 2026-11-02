#include "SearchEngine.h"
#include <QMutexLocker>
#include <algorithm>

namespace CodeShelf {

SearchEngine::SearchEngine(Database& db)
    : m_db(db)
    , m_repo(db)
{
}

QVector<SearchResult> SearchEngine::search(const SearchQuery& query) {
    if (m_db.isFts5Available() && (!query.freeTerms.isEmpty() || !query.exactPhrases.isEmpty())) {
        return searchFts(query);
    }
    return searchFallback(query);
}

QVector<SearchResult> SearchEngine::searchFts(const SearchQuery& query) {
    // Construct FTS MATCH expression safely
    QStringList ftsTokens;
    for (const QString& term : query.freeTerms) {
        // Escape internal quotes
        QString sanitized = term;
        sanitized.replace(QLatin1Char('"'), QStringLiteral("\"\""));
        ftsTokens.append(QStringLiteral("\"%1\"*").arg(sanitized));
    }
    for (const QString& phrase : query.exactPhrases) {
        QString sanitized = phrase;
        sanitized.replace(QLatin1Char('"'), QStringLiteral("\"\""));
        ftsTokens.append(QStringLiteral("\"%1\"").arg(sanitized));
    }

    const QString matchExpr = ftsTokens.join(QStringLiteral(" AND "));

    QMutexLocker locker(&m_db.mutex());
    QVector<SearchResult> results;

    const char* ftsSql = "SELECT snippet_id, bm25(snippets_fts) AS rank FROM snippets_fts WHERE snippets_fts MATCH ?1 ORDER BY rank ASC;";
    SqliteStatement stmt(m_db.rawDb(), QString::fromUtf8(ftsSql));
    stmt.bindText(1, matchExpr);

    while (stmt.step() == SQLITE_ROW) {
        const QString snipId = stmt.getText(0);
        double rank = stmt.getDouble(1);

        auto optSnip = m_repo.getById(snipId);
        if (!optSnip.has_value()) continue;
        const Snippet& snip = *optSnip;

        // Apply filters
        if (query.inTrash != snip.isDeleted()) continue;
        if (!query.languageFilter.isEmpty() && snip.language().compare(query.languageFilter, Qt::CaseInsensitive) != 0) continue;
        if (!query.collectionFilter.isEmpty() && snip.collectionId() != query.collectionFilter) continue;
        if (!query.tagFilter.isEmpty() && !snip.tags().contains(query.tagFilter, Qt::CaseInsensitive)) continue;
        if (query.favoriteFilter.has_value() && snip.isFavorite() != *query.favoriteFilter) continue;
        if (query.pinnedFilter.has_value() && snip.isPinned() != *query.pinnedFilter) continue;
        if (query.typeFilter.has_value() && snip.snippetType() != *query.typeFilter) continue;

        SearchResult res;
        res.snippet = snip;
        // bm25 lower is better; invert to positive score
        res.score = (rank < 0) ? -rank : (100.0 / (1.0 + rank));
        res.matchedTerms = query.freeTerms + query.exactPhrases;
        results.append(res);
    }

    return results;
}

QVector<SearchResult> SearchEngine::searchFallback(const SearchQuery& query) {
    QVector<Snippet> baseSnippets;
    if (query.inTrash) {
        baseSnippets = m_repo.getTrash();
    } else {
        baseSnippets = m_repo.getAllActive(query.sortField, query.sortDirection);
    }

    QVector<SearchResult> results;
    const QStringList terms = query.freeTerms + query.exactPhrases;

    for (const Snippet& snip : baseSnippets) {
        // Filters
        if (!query.languageFilter.isEmpty() && snip.language().compare(query.languageFilter, Qt::CaseInsensitive) != 0) continue;
        if (!query.collectionFilter.isEmpty() && snip.collectionId() != query.collectionFilter) continue;
        if (!query.tagFilter.isEmpty() && !snip.tags().contains(query.tagFilter, Qt::CaseInsensitive)) continue;
        if (query.favoriteFilter.has_value() && snip.isFavorite() != *query.favoriteFilter) continue;
        if (query.pinnedFilter.has_value() && snip.isPinned() != *query.pinnedFilter) continue;
        if (query.typeFilter.has_value() && snip.snippetType() != *query.typeFilter) continue;

        double score = 0.0;
        bool allTermsMatch = true;

        if (!terms.isEmpty()) {
            for (const QString& term : terms) {
                bool termFound = false;
                if (snip.title().contains(term, Qt::CaseInsensitive)) {
                    score += 50.0;
                    if (snip.title().compare(term, Qt::CaseInsensitive) == 0) score += 50.0;
                    termFound = true;
                }
                if (snip.tags().contains(term, Qt::CaseInsensitive)) {
                    score += 30.0;
                    termFound = true;
                }
                if (snip.description().contains(term, Qt::CaseInsensitive)) {
                    score += 20.0;
                    termFound = true;
                }
                if (snip.body().contains(term, Qt::CaseInsensitive)) {
                    score += 10.0;
                    termFound = true;
                }
                if (!termFound) {
                    allTermsMatch = false;
                    break;
                }
            }
            if (!allTermsMatch) continue;
        }

        SearchResult res;
        res.snippet = snip;
        res.score = score;
        res.matchedTerms = terms;
        results.append(res);
    }

    if (!terms.isEmpty()) {
        std::stable_sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
            if (a.snippet.isPinned() != b.snippet.isPinned()) {
                return a.snippet.isPinned() > b.snippet.isPinned();
            }
            return a.score > b.score;
        });
    }

    return results;
}

QString SearchEngine::highlightSnippetTitle(const QString& title, const QStringList& terms) {
    if (terms.isEmpty()) return title;
    // For minimal UI: return title directly without cluttering characters
    return title;
}

} // namespace CodeShelf
