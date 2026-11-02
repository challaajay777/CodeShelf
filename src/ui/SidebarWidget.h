#pragma once

#include <QWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QPushButton>
#include "../models/SidebarTreeModel.h"
#include "../database/Database.h"
#include "../repositories/CollectionRepository.h"
#include "../repositories/TagRepository.h"
#include "../repositories/SmartViewRepository.h"
#include "../repositories/SnippetRepository.h"

namespace CodeShelf {

class SidebarWidget : public QWidget {
    Q_OBJECT

public:
    explicit SidebarWidget(SnippetRepository& snippetRepo, CollectionRepository& colRepo,
                           TagRepository& tagRepo, SmartViewRepository& svRepo, QWidget* parent = nullptr);

    void refresh();

signals:
    void filterChanged(SidebarItemType type, const QString& entityId);
    void newCollectionRequested();
    void deleteCollectionRequested(const QString& colId);

private slots:
    void onItemActivated(const QModelIndex& index);
    void showContextMenu(const QPoint& pos);

private:
    SnippetRepository& m_snippetRepo;
    CollectionRepository& m_colRepo;
    TagRepository& m_tagRepo;
    SmartViewRepository& m_svRepo;

    QTreeView* m_treeView{nullptr};
    SidebarTreeModel* m_model{nullptr};
};

} // namespace CodeShelf
