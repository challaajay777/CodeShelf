#pragma once

#include <QString>
#include <QDateTime>
#include "Validation.h"

namespace CodeShelf {

class SmartView {
public:
    SmartView();
    explicit SmartView(const QString& name, const QString& queryString, int sortOrder = 0);

    QString id() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    QString queryString() const { return m_queryString; }
    void setQueryString(const QString& qs) { m_queryString = qs; }

    int sortOrder() const { return m_sortOrder; }
    void setSortOrder(int order) { m_sortOrder = order; }

    QDateTime createdAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }

    ValidationResult validate() const;

private:
    QString m_id;
    QString m_name;
    QString m_queryString;
    int m_sortOrder{0};
    QDateTime m_createdAt;
};

} // namespace CodeShelf
