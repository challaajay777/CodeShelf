#pragma once

#include <QObject>
#include <QTest>
#include "ui/MainWindow.h"

namespace CodeShelf {

class TestGui : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testMainWindowLaunchAndEmptyState();
    void testCreateEditSaveSnippetFlow();
    void testSearchInstantFilterFlow();
};

} // namespace CodeShelf
