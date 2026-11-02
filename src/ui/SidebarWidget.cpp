#include "SidebarWidget.h"
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

namespace CodeShelf {

SidebarWidget::SidebarWidget(SnippetRepository& snippetRepo, CollectionRepository& colRepo,
                             TagRepository& tagRepo, SmartViewRepository& svRepo, QWidget* parent)
    : QWidget(parent)
    , m_snippetRepo(snippetRepo)
    , m_colRepo(colRepo)
    , m_tagRepo(tagRepo)
    , m_svRepo(svRepo)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    m_treeView = new QTreeView(this);
    m_model = new SidebarTreeModel(this);
    m_treeView->setModel(m_model);
    m_treeView->setHeaderHidden(true);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->setUniformRowHeights(true);

    layout->addWidget(m_treeView);

    connect(m_treeView, &QTreeView::clicked, this, &SidebarWidget::onItemActivated);
    connect(m_treeView, &QTreeView::customContextMenuRequested, this, &SidebarWidget::showContextMenu);

    refresh();
}

void SidebarWidget::refresh() {
    int allCount = m_snippetRepo.countActive();
    int favCount = m_snippetRepo.getFavorites().size();
    int pinCount = m_snippetRepo.getPinned().size();
    int unfiledCount = m_snippetRepo.getUnfiled().size();
    int trashCount = m_snippetRepo.countTrash();

    const auto collections = m_colRepo.getAll();
    const auto tags = m_tagRepo.getAllTagsWithCounts();
    const auto smartViews = m_svRepo.getAll();

    m_model->refresh(allCount, favCount, pinCount, unfiledCount, trashCount, collections, tags, smartViews);
    m_treeView->expandAll();
}

void SidebarWidget::onItemActivated(const QModelIndex& index) {
    if (!index.isValid()) return;

    int typeInt = index.data(SidebarTreeModel::ItemTypeRole).toInt();
    auto type = static_cast<SidebarItemType>(typeInt);
    QString entityId = index.data(SidebarTreeModel::EntityIdRole).toString();

    emit filterChanged(type, entityId);
}

void SidebarWidget::showContextMenu(const QPoint& pos) {
    QModelIndex index = m_treeView->indexAt(pos);
    QMenu menu(this);

    if (index.isValid()) {
        int typeInt = index.data(SidebarTreeModel::ItemTypeRole).toInt();
        auto type = static_cast<SidebarItemType>(typeInt);
        QString entityId = index.data(SidebarTreeModel::EntityIdRole).toString();

        if (type == SidebarItemType::CollectionItem) {
            menu.addAction(QStringLiteral("Rename Collection..."), [this, entityId]() {
                auto col = m_colRepo.getById(entityId);
                if (!col.has_value()) return;
                bool ok = false;
                QString newName = QInputDialog::getText(this, QStringLiteral("Rename Collection"),
                                                       QStringLiteral("Collection Name:"), QLineEdit::Normal,
                                                       col->name(), &ok);
                if (ok && !newName.trimmed().isEmpty()) {
                    col->setName(newName.trimmed());
                    QString err;
                    if (m_colRepo.update(*col, err)) {
                        refresh();
                    } else {
                        QMessageBox::warning(this, QStringLiteral("Error"), err);
                    }
                }
            });

            menu.addAction(QStringLiteral("Delete Collection..."), [this, entityId]() {
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Question);
                msg.setWindowTitle(QStringLiteral("Delete Collection"));
                msg.setText(QStringLiteral("How would you like to handle snippets in this collection?"));
                auto unfileBtn = msg.addButton(QStringLiteral("Keep Snippets (Unfile)"), QMessageBox::AcceptRole);
                auto deleteBtn = msg.addButton(QStringLiteral("Move Snippets to Trash"), QMessageBox::DestructiveRole);
                msg.addButton(QMessageBox::Cancel);
                msg.exec();

                if (msg.clickedButton() == unfileBtn) {
                    QString err;
                    m_colRepo.deleteCollection(entityId, false, err);
                    refresh();
                } else if (msg.clickedButton() == deleteBtn) {
                    QString err;
                    m_colRepo.deleteCollection(entityId, true, err);
                    refresh();
                }
            });
        } else if (type == SidebarItemType::Trash) {
            menu.addAction(QStringLiteral("Empty Trash..."), [this]() {
                if (QMessageBox::question(this, QStringLiteral("Empty Trash"),
                                          QStringLiteral("Are you sure you want to permanently delete all snippets in Trash? This cannot be undone."),
                                          QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                    QString err;
                    m_snippetRepo.emptyTrash(err);
                    refresh();
                }
            });
        }
    }

    menu.addAction(QStringLiteral("New Collection..."), [this]() {
        bool ok = false;
        QString name = QInputDialog::getText(this, QStringLiteral("New Collection"),
                                             QStringLiteral("Collection Name:"), QLineEdit::Normal, QString(), &ok);
        if (ok && !name.trimmed().isEmpty()) {
            Collection col(name.trimmed());
            QString err;
            if (m_colRepo.create(col, err)) {
                refresh();
            } else {
                QMessageBox::warning(this, QStringLiteral("Error"), err);
            }
        }
    });

    menu.exec(m_treeView->viewport()->mapToGlobal(pos));
}

} // namespace CodeShelf
