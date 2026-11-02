#pragma once

#include <QString>
#include <QDateTime>
#include "Validation.h"

namespace CodeShelf {

class Tag {
public:
    Tag();
    explicit Tag(const QString& displayName);

    QString id() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    QString normalizedName() const { return m_normalizedName; }
    void setNormalizedName(const QString& norm) { m_normalizedName = norm; }

    QString displayName() const { return m_displayName; }
    void setDisplayName(const QString& name);

    int usageCount() const { return m_usageCount; }
    void setUsageCount(int count) { m_usageCount = count; }

    QDateTime createdAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }

    ValidationResult validate() const;

private:
    QString m_id;
    QString m_normalizedName;
    QString m_displayName;
    int m_usageCount{0};
    QDateTime m_createdAt;
};

} // namespace CodeShelf
