#pragma once

#include <QString>
#include <QVector>
#include "SearchQuery.h"
#include "../core/Snippet.h"
#include "../database/Database.h"
#include "../repositories/SnippetRepository.h"

namespace CodeShelf {

struct SearchResult {
    Snippet snippet;
    double score{0.0};
    QStringList matchedTerms;
};

class SearchEngine {
public:
    explicit SearchEngine(Database& db = Database::instance());

    QVector<SearchResult> search(const SearchQuery& query);

    // Highlighting helper for UI (does not alter code)
    static QString highlightSnippetTitle(const QString& title, const QStringList& terms);

private:
    QVector<SearchResult> searchFts(const SearchQuery& query);
    QVector<SearchResult> searchFallback(const SearchQuery& query);

    Database& m_db;
    SnippetRepository m_repo;
};

} // namespace CodeShelf
