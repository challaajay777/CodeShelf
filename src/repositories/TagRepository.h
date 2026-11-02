#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <optional>
#include "../core/Tag.h"
#include "../database/Database.h"

namespace CodeShelf {

class TagRepository {
public:
    explicit TagRepository(Database& db = Database::instance());

    Tag getOrCreateTag(const QString& displayName, QString& errorOut);
    std::optional<Tag> getById(const QString& id);
    std::optional<Tag> getByNormalizedName(const QString& normalizedName);
    QVector<Tag> getAllTagsWithCounts();

    bool renameTag(const QString& id, const QString& newDisplayName, QString& errorOut);
    bool mergeTags(const QString& sourceTagId, const QString& targetTagId, QString& errorOut);
    bool deleteTag(const QString& id, QString& errorOut);

    QStringList getTagsForSnippet(const QString& snippetId);
    bool setTagsForSnippet(const QString& snippetId, const QStringList& tags, QString& errorOut);

private:
    Database& m_db;
};

} // namespace CodeShelf
