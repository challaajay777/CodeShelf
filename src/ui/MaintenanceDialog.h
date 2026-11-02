#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include "../services/MaintenanceService.h"

namespace CodeShelf {

class MaintenanceDialog : public QDialog {
    Q_OBJECT

public:
    explicit MaintenanceDialog(MaintenanceService& maintService, QWidget* parent = nullptr);

signals:
    void databaseCompacted();

private slots:
    void refreshStats();
    void runIntegrityCheck();
    void rebuildFts();
    void runVacuum();

private:
    void setupUi();

    MaintenanceService& m_maintService;

    QLabel* m_statsLabel{nullptr};
    QPushButton* m_integrityBtn{nullptr};
    QPushButton* m_rebuildFtsBtn{nullptr};
    QPushButton* m_vacuumBtn{nullptr};
    QTextEdit* m_logOutput{nullptr};
};

} // namespace CodeShelf
