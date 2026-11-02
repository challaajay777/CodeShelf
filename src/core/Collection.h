#pragma once

#include <QString>
#include <QDateTime>
#include "Validation.h"

namespace CodeShelf {

class Collection {
public:
    Collection();
    explicit Collection(const QString& name, const QString& description = QString(), int sortOrder = 0);

    QString id() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    QString description() const { return m_description; }
    void setDescription(const QString& description) { m_description = description; }

    int sortOrder() const { return m_sortOrder; }
    void setSortOrder(int order) { m_sortOrder = order; }

    QDateTime createdAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }

    QDateTime updatedAt() const { return m_updatedAt; }
    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }

    ValidationResult validate() const;

private:
    QString m_id;
    QString m_name;
    QString m_description;
    int m_sortOrder{0};
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
};

} // namespace CodeShelf
