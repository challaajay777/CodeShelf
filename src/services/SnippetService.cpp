#include "SnippetService.h"
#include <algorithm>

namespace CodeShelf {

SnippetService::SnippetService(SnippetRepository& snippetRepo, CollectionRepository& colRepo,
                               TagRepository& tagRepo, RevisionRepository& revRepo)
    : m_snippetRepo(snippetRepo)
    , m_colRepo(colRepo)
    , m_tagRepo(tagRepo)
    , m_revRepo(revRepo)
{
}

bool SnippetService::saveSnippet(Snippet& snippet, bool createRevision, QString& errorOut, const QString& changeNote) {
    auto existing = m_snippetRepo.getById(snippet.id());
    if (existing.has_value()) {
        return m_snippetRepo.update(snippet, createRevision, errorOut, changeNote);
    } else {
        return m_snippetRepo.create(snippet, errorOut);
    }
}

bool SnippetService::duplicateSnippet(const QString& snippetId, Snippet& outSnippet, QString& errorOut) {
    return m_snippetRepo.duplicate(snippetId, outSnippet, errorOut);
}

bool SnippetService::moveToTrash(const QString& snippetId, QString& errorOut) {
    return m_snippetRepo.softDelete(snippetId, errorOut);
}

bool SnippetService::restoreFromTrash(const QString& snippetId, QString& errorOut) {
    return m_snippetRepo.restore(snippetId, errorOut);
}

bool SnippetService::permanentlyDelete(const QString& snippetId, QString& errorOut) {
    return m_snippetRepo.permanentDelete(snippetId, errorOut);
}

bool SnippetService::restoreRevision(const QString& snippetId, const QString& revisionId, QString& errorOut) {
    auto optRev = m_revRepo.getById(revisionId);
    if (!optRev.has_value()) {
        errorOut = QStringLiteral("Revision not found.");
        return false;
    }

    auto optSnip = m_snippetRepo.getById(snippetId);
    if (!optSnip.has_value()) {
        errorOut = QStringLiteral("Snippet not found.");
        return false;
    }

    Snippet restored = *optSnip;
    restored.setTitle(optRev->title());
    restored.setBody(optRev->body());
    restored.setDescription(optRev->description());
    restored.setLanguage(optRev->language());

    const QString note = QStringLiteral("Restored from revision #%1").arg(optRev->revisionNumber());
    return m_snippetRepo.update(restored, true, errorOut, note);
}

QVector<RelatedSnippetMatch> SnippetService::findRelatedSnippets(const Snippet& target, int limit) {
    QVector<RelatedSnippetMatch> matches;
    const auto all = m_snippetRepo.getAllActive();

    const QSet<QString> targetTags(target.tags().cbegin(), target.tags().cend());

    for (const Snippet& snip : all) {
        if (snip.id() == target.id()) continue;

        int score = 0;
        QStringList reasons;

        // Shared tags
        int sharedTags = 0;
        for (const QString& t : snip.tags()) {
            if (targetTags.contains(t)) {
                ++sharedTags;
            }
        }
        if (sharedTags > 0) {
            score += sharedTags * 25;
            reasons.append(QStringLiteral("Shares %1 tag%2").arg(sharedTags).arg(sharedTags > 1 ? "s" : ""));
        }

        // Shared collection
        if (!target.collectionId().isEmpty() && target.collectionId() == snip.collectionId()) {
            score += 20;
            reasons.append(QStringLiteral("In same collection"));
        }

        // Shared language
        if (target.language() != QStringLiteral("Plain Text") && target.language() == snip.language()) {
            score += 15;
            reasons.append(QStringLiteral("Same language (%1)").arg(target.language()));
        }

        if (score > 0) {
            RelatedSnippetMatch match;
            match.snippet = snip;
            match.score = score;
            match.reason = reasons.join(QStringLiteral(", "));
            matches.append(match);
        }
    }

    std::stable_sort(matches.begin(), matches.end(), [](const RelatedSnippetMatch& a, const RelatedSnippetMatch& b) {
        return a.score > b.score;
    });

    if (matches.size() > limit) {
        matches.resize(limit);
    }

    return matches;
}

} // namespace CodeShelf
