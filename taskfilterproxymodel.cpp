#include "taskfilterproxymodel.h"
#include "taskmodel.h"
#include "task.h"

/**
 * @file taskfilterproxymodel.cpp
 * @brief Filtrage personnalisé pour les tâches.
 */

TaskFilterProxyModel::TaskFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
    m_priorityFilter(Priority::LOW),
    m_statusFilter(Status::NOTSTARTED),
    m_showCompleted(true),
    m_priorityFilterEnabled(false),
    m_statusFilterEnabled(false)
{
    setRecursiveFilteringEnabled(true);
}

void TaskFilterProxyModel::setSearchText(const QString &text)
{
    m_searchText = text;
    beginResetModel();
    endResetModel();
}

void TaskFilterProxyModel::setPriorityFilter(Priority p, bool enabled)
{
    m_priorityFilter = p;
    m_priorityFilterEnabled = enabled;
    beginResetModel();
    endResetModel();
}

void TaskFilterProxyModel::setStatusFilter(Status s, bool enabled)
{
    m_statusFilter = s;
    m_statusFilterEnabled = enabled;
    beginResetModel();
    endResetModel();
}

void TaskFilterProxyModel::setShowCompleted(bool show)
{
    m_showCompleted = show;
    beginResetModel();
    endResetModel();
}

bool TaskFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    QModelIndex srcIndex = sourceModel()->index(row, 0, parent);
    if (!srcIndex.isValid()) return false;

    Task *t = static_cast<Task*>(srcIndex.internalPointer());
    if (!t) return false;

    // Filtrer par texte
    bool matchesText = m_searchText.isEmpty() || 
                       t->title().contains(m_searchText, Qt::CaseInsensitive) || 
                       t->description().contains(m_searchText, Qt::CaseInsensitive);
    
    // Filtrer par status complété
    bool matchesCompleted = m_showCompleted || (t->status() != Status::COMPLETED);
    
    // Filtrer par priorité si activé
    bool matchesPriority = !m_priorityFilterEnabled || (t->priority() == m_priorityFilter);
    
    // Filtrer par status si activé
    bool matchesStatus = !m_statusFilterEnabled || (t->status() == m_statusFilter);

    return matchesText && matchesCompleted && matchesPriority && matchesStatus;
}
