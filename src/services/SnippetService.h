#pragma once

#include <QString>
#include <QVector>
#include <optional>
#include "../core/Snippet.h"
#include "../core/Collection.h"
#include "../core/Tag.h"
#include "../core/Revision.h"
#include "../repositories/SnippetRepository.h"
#include "../repositories/CollectionRepository.h"
#include "../repositories/TagRepository.h"
#include "../repositories/RevisionRepository.h"

namespace CodeShelf {

struct RelatedSnippetMatch {
    Snippet snippet;
    int score{0};
    QString reason;
};

class SnippetService {
public:
    SnippetService(SnippetRepository& snippetRepo, CollectionRepository& colRepo,
                   TagRepository& tagRepo, RevisionRepository& revRepo);

    bool saveSnippet(Snippet& snippet, bool createRevision, QString& errorOut, const QString& changeNote = QString());
    bool duplicateSnippet(const QString& snippetId, Snippet& outSnippet, QString& errorOut);
    bool moveToTrash(const QString& snippetId, QString& errorOut);
    bool restoreFromTrash(const QString& snippetId, QString& errorOut);
    bool permanentlyDelete(const QString& snippetId, QString& errorOut);
    bool restoreRevision(const QString& snippetId, const QString& revisionId, QString& errorOut);

    QVector<RelatedSnippetMatch> findRelatedSnippets(const Snippet& target, int limit = 5);

private:
    SnippetRepository& m_snippetRepo;
    CollectionRepository& m_colRepo;
    TagRepository& m_tagRepo;
    RevisionRepository& m_revRepo;
};

} // namespace CodeShelf
