#include "Collection.h"

namespace CodeShelf {

Collection::Collection()
    : m_id(Validation::generateUuid())
    , m_createdAt(QDateTime::currentDateTimeUtc())
    , m_updatedAt(m_createdAt)
{
}

Collection::Collection(const QString& name, const QString& description, int sortOrder)
    : m_id(Validation::generateUuid())
    , m_name(name)
    , m_description(description)
    , m_sortOrder(sortOrder)
    , m_createdAt(QDateTime::currentDateTimeUtc())
    , m_updatedAt(m_createdAt)
{
}

ValidationResult Collection::validate() const {
    return Validation::validateCollectionName(m_name);
}

} // namespace CodeShelf
