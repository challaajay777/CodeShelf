#pragma once

#include <QString>
#include <QDateTime>
#include "Validation.h"

namespace CodeShelf {

class Revision {
public:
    Revision();
    explicit Revision(const QString& snippetId, int revisionNumber, const QString& title, const QString& body,
                     const QString& description, const QString& language, const QString& changeNote = QString());

    QString id() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    QString snippetId() const { return m_snippetId; }
    void setSnippetId(const QString& id) { m_snippetId = id; }

    int revisionNumber() const { return m_revisionNumber; }
    void setRevisionNumber(int rev) { m_revisionNumber = rev; }

    QString title() const { return m_title; }
    void setTitle(const QString& title) { m_title = title; }

    QString body() const { return m_body; }
    void setBody(const QString& body) { m_body = body; }

    QString description() const { return m_description; }
    void setDescription(const QString& desc) { m_description = desc; }

    QString language() const { return m_language; }
    void setLanguage(const QString& lang) { m_language = lang; }

    QDateTime changedAt() const { return m_changedAt; }
    void setChangedAt(const QDateTime& dt) { m_changedAt = dt; }

    QString changeNote() const { return m_changeNote; }
    void setChangeNote(const QString& note) { m_changeNote = note; }

private:
    QString m_id;
    QString m_snippetId;
    int m_revisionNumber{1};
    QString m_title;
    QString m_body;
    QString m_description;
    QString m_language;
    QDateTime m_changedAt;
    QString m_changeNote;
};

} // namespace CodeShelf
