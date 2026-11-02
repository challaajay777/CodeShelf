#pragma once

#include <QString>
#include <QDateTime>
#include <sqlite3.h>
#include <memory>
#include <functional>

namespace CodeShelf {

class SqliteStatement {
public:
    SqliteStatement(sqlite3* db, const QString& sql);
    ~SqliteStatement();

    // Non-copyable
    SqliteStatement(const SqliteStatement&) = delete;
    SqliteStatement& operator=(const SqliteStatement&) = delete;

    // Movable
    SqliteStatement(SqliteStatement&& other) noexcept;
    SqliteStatement& operator=(SqliteStatement&& other) noexcept;

    bool isValid() const { return m_stmt != nullptr; }
    QString lastError() const;

    bool bindText(int index, const QString& text);
    bool bindInt(int index, int value);
    bool bindInt64(int index, qint64 value);
    bool bindDouble(int index, double value);
    bool bindNull(int index);
    bool bindDateTime(int index, const QDateTime& dt);

    // Execution
    int step(); // Returns SQLITE_ROW, SQLITE_DONE, or error code
    bool execute(); // Steps once, expects SQLITE_DONE
    bool reset();

    // Value reading
    QString getText(int col) const;
    int getInt(int col) const;
    qint64 getInt64(int col) const;
    double getDouble(int col) const;
    bool isNull(int col) const;
    QDateTime getDateTime(int col) const;

    sqlite3_stmt* raw() const { return m_stmt; }

private:
    sqlite3* m_db{nullptr};
    sqlite3_stmt* m_stmt{nullptr};
};

class SqliteTransaction {
public:
    explicit SqliteTransaction(sqlite3* db);
    ~SqliteTransaction();

    SqliteTransaction(const SqliteTransaction&) = delete;
    SqliteTransaction& operator=(const SqliteTransaction&) = delete;

    bool commit();
    void rollback();
    bool isActive() const { return m_active; }

private:
    sqlite3* m_db{nullptr};
    bool m_active{false};
    bool m_owner{false};
};

} // namespace CodeShelf
