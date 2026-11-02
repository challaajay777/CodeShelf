#include "SmartView.h"

namespace CodeShelf {

SmartView::SmartView()
    : m_id(Validation::generateUuid())
    , m_createdAt(QDateTime::currentDateTimeUtc())
{
}

SmartView::SmartView(const QString& name, const QString& queryString, int sortOrder)
    : m_id(Validation::generateUuid())
    , m_name(name)
    , m_queryString(queryString)
    , m_sortOrder(sortOrder)
    , m_createdAt(QDateTime::currentDateTimeUtc())
{
}

ValidationResult SmartView::validate() const {
    if (m_name.trimmed().isEmpty()) {
        return ValidationResult::failure(QStringLiteral("Smart view name cannot be empty."));
    }
    if (m_name.length() > 128) {
        return ValidationResult::failure(QStringLiteral("Smart view name exceeds 128 characters."));
    }
    return ValidationResult::success();
}

} // namespace CodeShelf
