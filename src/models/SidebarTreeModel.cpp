#include "SidebarTreeModel.h"

namespace CodeShelf {

SidebarTreeModel::SidebarTreeModel(QObject* parent)
    : QStandardItemModel(parent)
{
}

void SidebarTreeModel::refresh(int allCount, int favCount, int pinCount, int unfiledCount, int trashCount,
                               const QVector<Collection>& collections,
                               const QVector<Tag>& tags,
                               const QVector<SmartView>& smartViews) {
    clear();

    // 1. Library items
    m_allItem = new QStandardItem(QStringLiteral("All Snippets (%1)").arg(allCount));
    m_allItem->setData(static_cast<int>(SidebarItemType::AllSnippets), ItemTypeRole);
    m_allItem->setEditable(false);
    appendRow(m_allItem);

    m_favItem = new QStandardItem(QStringLiteral("Favorites (%1)").arg(favCount));
    m_favItem->setData(static_cast<int>(SidebarItemType::Favorites), ItemTypeRole);
    m_favItem->setEditable(false);
    appendRow(m_favItem);

    m_pinItem = new QStandardItem(QStringLiteral("Pinned (%1)").arg(pinCount));
    m_pinItem->setData(static_cast<int>(SidebarItemType::Pinned), ItemTypeRole);
    m_pinItem->setEditable(false);
    appendRow(m_pinItem);

    m_unfiledItem = new QStandardItem(QStringLiteral("Unfiled (%1)").arg(unfiledCount));
    m_unfiledItem->setData(static_cast<int>(SidebarItemType::Unfiled), ItemTypeRole);
    m_unfiledItem->setEditable(false);
    appendRow(m_unfiledItem);

    // 2. Collections header
    m_collectionsHeader = new QStandardItem(QStringLiteral("Collections"));
    m_collectionsHeader->setData(static_cast<int>(SidebarItemType::CollectionsHeader), ItemTypeRole);
    m_collectionsHeader->setEditable(false);
    m_collectionsHeader->setSelectable(false);
    appendRow(m_collectionsHeader);

    for (const auto& col : collections) {
        auto item = new QStandardItem(col.name());
        item->setData(static_cast<int>(SidebarItemType::CollectionItem), ItemTypeRole);
        item->setData(col.id(), EntityIdRole);
        item->setEditable(false);
        m_collectionsHeader->appendRow(item);
    }

    // 3. Tags header
    m_tagsHeader = new QStandardItem(QStringLiteral("Tags"));
    m_tagsHeader->setData(static_cast<int>(SidebarItemType::TagsHeader), ItemTypeRole);
    m_tagsHeader->setEditable(false);
    m_tagsHeader->setSelectable(false);
    appendRow(m_tagsHeader);

    for (const auto& tag : tags) {
        auto item = new QStandardItem(QStringLiteral("%1 (%2)").arg(tag.displayName()).arg(tag.usageCount()));
        item->setData(static_cast<int>(SidebarItemType::TagItem), ItemTypeRole);
        item->setData(tag.normalizedName(), EntityIdRole);
        item->setEditable(false);
        m_tagsHeader->appendRow(item);
    }

    // 4. Smart views header
    m_smartViewsHeader = new QStandardItem(QStringLiteral("Smart Views"));
    m_smartViewsHeader->setData(static_cast<int>(SidebarItemType::SmartViewsHeader), ItemTypeRole);
    m_smartViewsHeader->setEditable(false);
    m_smartViewsHeader->setSelectable(false);
    appendRow(m_smartViewsHeader);

    for (const auto& sv : smartViews) {
        auto item = new QStandardItem(sv.name());
        item->setData(static_cast<int>(SidebarItemType::SmartViewItem), ItemTypeRole);
        item->setData(sv.id(), EntityIdRole);
        item->setEditable(false);
        m_smartViewsHeader->appendRow(item);
    }

    // 5. Trash
    m_trashItem = new QStandardItem(QStringLiteral("Trash (%1)").arg(trashCount));
    m_trashItem->setData(static_cast<int>(SidebarItemType::Trash), ItemTypeRole);
    m_trashItem->setEditable(false);
    appendRow(m_trashItem);
}

} // namespace CodeShelf
