#pragma once

#include <QStandardItemModel>
#include <QString>
#include "../core/Collection.h"
#include "../core/Tag.h"
#include "../core/SmartView.h"

namespace CodeShelf {

enum class SidebarItemType {
    AllSnippets,
    Favorites,
    Pinned,
    Unfiled,
    CollectionsHeader,
    CollectionItem,
    TagsHeader,
    TagItem,
    SmartViewsHeader,
    SmartViewItem,
    Trash
};

class SidebarTreeModel : public QStandardItemModel {
    Q_OBJECT

public:
    enum CustomRoles {
        ItemTypeRole = Qt::UserRole + 100,
        EntityIdRole
    };

    explicit SidebarTreeModel(QObject* parent = nullptr);

    void refresh(int allCount, int favCount, int pinCount, int unfiledCount, int trashCount,
                 const QVector<Collection>& collections,
                 const QVector<Tag>& tags,
                 const QVector<SmartView>& smartViews);

private:
    QStandardItem* m_allItem{nullptr};
    QStandardItem* m_favItem{nullptr};
    QStandardItem* m_pinItem{nullptr};
    QStandardItem* m_unfiledItem{nullptr};
    QStandardItem* m_collectionsHeader{nullptr};
    QStandardItem* m_tagsHeader{nullptr};
    QStandardItem* m_smartViewsHeader{nullptr};
    QStandardItem* m_trashItem{nullptr};
};

} // namespace CodeShelf
