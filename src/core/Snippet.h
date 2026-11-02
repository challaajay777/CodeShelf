#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include "Enums.h"
#include "Validation.h"
#include "ContentHash.h"

namespace CodeShelf {

class Snippet {
public:
    Snippet();
    explicit Snippet(const QString& title, const QString& body, const QString& language = QStringLiteral("Plain Text"));

    QString id() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    QString title() const { return m_title; }
    void setTitle(const QString& title) { m_title = title; }

    QString body() const { return m_body; }
    void setBody(const QString& body);

    QString description() const { return m_description; }
    void setDescription(const QString& description) { m_description = description; }

    QString language() const { return m_language; }
    void setLanguage(const QString& language) { m_language = normalizeLanguage(language); }

    SnippetType snippetType() const { return m_snippetType; }
    void setSnippetType(SnippetType type) { m_snippetType = type; }

    QString collectionId() const { return m_collectionId; }
    void setCollectionId(const QString& collectionId) { m_collectionId = collectionId; }

    QStringList tags() const { return m_tags; }
    void setTags(const QStringList& tags);
    void addTag(const QString& tag);
    void removeTag(const QString& tag);

    bool isFavorite() const { return m_favorite; }
    void setFavorite(bool fav) { m_favorite = fav; }

    bool isPinned() const { return m_pinned; }
    void setPinned(bool pin) { m_pinned = pin; }

    QDateTime createdAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }

    QDateTime updatedAt() const { return m_updatedAt; }
    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }

    QDateTime deletedAt() const { return m_deletedAt; }
    void setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }
    bool isDeleted() const { return m_deletedAt.isValid(); }

    QString contentHash() const { return m_contentHash; }
    void setContentHash(const QString& hash) { m_contentHash = hash; }

    QString sourceUrl() const { return m_sourceUrl; }
    void setSourceUrl(const QString& url) { m_sourceUrl = url; }

    void refreshContentHash();

    ValidationResult validate() const;

private:
    QString m_id;
    QString m_title;
    QString m_body;
    QString m_description;
    QString m_language{QStringLiteral("Plain Text")};
    SnippetType m_snippetType{SnippetType::Code};
    QString m_collectionId;
    QStringList m_tags;
    bool m_favorite{false};
    bool m_pinned{false};
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QDateTime m_deletedAt; // Invalid if active
    QString m_contentHash;
    QString m_sourceUrl;
};

} // namespace CodeShelf
