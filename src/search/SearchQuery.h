#pragma once

#include <QString>
#include <QStringList>
#include <optional>
#include "../core/Enums.h"

namespace CodeShelf {

struct SearchQuery {
    QString rawQuery;
    QStringList freeTerms;
    QStringList exactPhrases;
    QString languageFilter;
    QString tagFilter;
    QString collectionFilter;
    std::optional<SnippetType> typeFilter;
    std::optional<bool> favoriteFilter;
    std::optional<bool> pinnedFilter;
    bool inTrash{false};
    SortField sortField{SortField::UpdatedAt};
    SortDirection sortDirection{SortDirection::Descending};

    bool isEmpty() const {
        return freeTerms.isEmpty() && exactPhrases.isEmpty() && languageFilter.isEmpty() &&
               tagFilter.isEmpty() && collectionFilter.isEmpty() && !typeFilter.has_value() &&
               !favoriteFilter.has_value() && !pinnedFilter.has_value();
    }
};

} // namespace CodeShelf
