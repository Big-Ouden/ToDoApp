#include "taskfilterproxymodel.h"
#include "taskmodel.h"
#include "task.h"

/**
 * @file taskfilterproxymodel.cpp
 * @brief Filtrage personnalisé pour les tâches.
 */

TaskFilterProxyModel::TaskFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
    m_priorityFilter(Priority::Low),
    m_statusFilter(Status::NotStarted)
{
    setRecursiveFilteringEnabled(true);
}

void TaskFilterProxyModel::setSearchText(const QString &text)
{
    m_searchText = text;
    invalidateFilter();
}

void TaskFilterProxyModel::setPriorityFilter(Priority p)
{
    m_priorityFilter = p;
    invalidateFilter();
}

void TaskFilterProxyModel::setStatusFilter(Status s)
{
    m_statusFilter = s;
    invalidateFilter();
}

bool TaskFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    QModelIndex srcIndex = sourceModel()->index(row, 0, parent);
    if (!srcIndex.isValid()) return false;

    Task *t = static_cast<Task*>(srcIndex.internalPointer());
    if (!t) return false;

    bool matchesText = m_searchText.isEmpty() || t->title().contains(m_searchText, Qt::CaseInsensitive) || t->description().contains(m_searchText, Qt::CaseInsensitive);
    bool matchesPriority = true; // if filter selects "Any" we would handle it; simplified here
    bool matchesStatus = true;

    Q_UNUSED(matchesPriority)
    Q_UNUSED(matchesStatus)

    return matchesText;
}
