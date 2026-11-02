#pragma once

#include <QString>
#include <QVector>
#include "../core/Snippet.h"
#include "../repositories/SnippetRepository.h"

namespace CodeShelf {

enum class DuplicateMatchKind {
    ExactHash,
    NormalizedWhitespace
};

struct DuplicateMatch {
    Snippet snippet;
    DuplicateMatchKind kind;
    QString explanation;
};

class DuplicateService {
public:
    explicit DuplicateService(SnippetRepository& repo);

    QVector<DuplicateMatch> findDuplicatesForSnippet(const Snippet& target, bool checkNormalized = true);
    QVector<QPair<Snippet, Snippet>> scanEntireLibraryForDuplicates(bool checkNormalized = true);

private:
    SnippetRepository& m_repo;
};

} // namespace CodeShelf
