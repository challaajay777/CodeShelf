#include "AppSettings.h"
#include "../platform/PlatformUtils.h"

namespace CodeShelf {

AppSettings& AppSettings::instance() {
    static AppSettings settings;
    return settings;
}

AppSettings::AppSettings()
    : m_settings(QStringLiteral("CodeShelf"), QStringLiteral("CodeShelf"))
{
}

AppTheme AppSettings::theme() const {
    const QString val = m_settings.value(QStringLiteral("ui/theme"), QStringLiteral("System")).toString();
    if (val == QStringLiteral("Light")) return AppTheme::Light;
    if (val == QStringLiteral("Dark")) return AppTheme::Dark;
    return AppTheme::System;
}

void AppSettings::setTheme(AppTheme theme) {
    QString val = QStringLiteral("System");
    if (theme == AppTheme::Light) val = QStringLiteral("Light");
    if (theme == AppTheme::Dark) val = QStringLiteral("Dark");
    m_settings.setValue(QStringLiteral("ui/theme"), val);
}

QString AppSettings::codeFontFamily() const {
#ifdef Q_OS_WIN
    return m_settings.value(QStringLiteral("editor/fontFamily"), QStringLiteral("Consolas")).toString();
#else
    return m_settings.value(QStringLiteral("editor/fontFamily"), QStringLiteral("Monospace")).toString();
#endif
}

void AppSettings::setCodeFontFamily(const QString& family) {
    m_settings.setValue(QStringLiteral("editor/fontFamily"), family);
}

int AppSettings::codeFontSize() const {
    return m_settings.value(QStringLiteral("editor/fontSize"), 10).toInt();
}

void AppSettings::setCodeFontSize(int size) {
    m_settings.setValue(QStringLiteral("editor/fontSize"), size);
}

int AppSettings::tabSize() const {
    return m_settings.value(QStringLiteral("editor/tabSize"), 4).toInt();
}

void AppSettings::setTabSize(int size) {
    m_settings.setValue(QStringLiteral("editor/tabSize"), size);
}

bool AppSettings::showLineNumbers() const {
    return m_settings.value(QStringLiteral("editor/showLineNumbers"), true).toBool();
}

void AppSettings::setShowLineNumbers(bool show) {
    m_settings.setValue(QStringLiteral("editor/showLineNumbers"), show);
}

QString AppSettings::backupDirectory() const {
    return m_settings.value(QStringLiteral("backup/directory"), PlatformUtils::defaultBackupDirectory()).toString();
}

void AppSettings::setBackupDirectory(const QString& dir) {
    m_settings.setValue(QStringLiteral("backup/directory"), dir);
}

void AppSettings::sync() {
    m_settings.sync();
}

} // namespace CodeShelf
