#pragma once

#include <QString>
#include <QSettings>

namespace CodeShelf {

enum class AppTheme {
    System,
    Light,
    Dark
};

class AppSettings {
public:
    static AppSettings& instance();

    AppTheme theme() const;
    void setTheme(AppTheme theme);

    QString codeFontFamily() const;
    void setCodeFontFamily(const QString& family);

    int codeFontSize() const;
    void setCodeFontSize(int size);

    int tabSize() const;
    void setTabSize(int size);

    bool showLineNumbers() const;
    void setShowLineNumbers(bool show);

    QString backupDirectory() const;
    void setBackupDirectory(const QString& dir);

    void sync();

private:
    AppSettings();
    QSettings m_settings;
};

} // namespace CodeShelf
