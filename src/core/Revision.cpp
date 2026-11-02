#include "Revision.h"

namespace CodeShelf {

Revision::Revision()
    : m_id(Validation::generateUuid())
    , m_changedAt(QDateTime::currentDateTimeUtc())
{
}

Revision::Revision(const QString& snippetId, int revisionNumber, const QString& title, const QString& body,
                   const QString& description, const QString& language, const QString& changeNote)
    : m_id(Validation::generateUuid())
    , m_snippetId(snippetId)
    , m_revisionNumber(revisionNumber)
    , m_title(title)
    , m_body(body)
    , m_description(description)
    , m_language(language)
    , m_changedAt(QDateTime::currentDateTimeUtc())
    , m_changeNote(changeNote)
{
}

} // namespace CodeShelf
