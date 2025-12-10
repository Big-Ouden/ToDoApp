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

    // === Recherche avancée avec préfixes ===
    bool matchesText = true;
    
    if (!m_searchText.isEmpty()) {
        QString searchLower = m_searchText.toLower().trimmed();
        
        // Recherche par tag: "tag:urgent" ou "tag:work"
        if (searchLower.startsWith("tag:")) {
            QString tagQuery = searchLower.mid(4).trimmed();
            matchesText = false;
            for (const QString &tag : t->tags()) {
                if (tag.toLower().contains(tagQuery)) {
                    matchesText = true;
                    break;
                }
            }
        }
        // Recherche par priorité: "priority:high" ou "priority:critical"
        else if (searchLower.startsWith("priority:") || searchLower.startsWith("priorité:")) {
            QString priorityQuery = searchLower.mid(searchLower.indexOf(':') + 1).trimmed();
            QString taskPriority = priorityToString(t->priority()).toLower();
            matchesText = taskPriority.contains(priorityQuery);
        }
        // Recherche par statut: "status:completed" ou "statut:terminé"
        else if (searchLower.startsWith("status:") || searchLower.startsWith("statut:")) {
            QString statusQuery = searchLower.mid(searchLower.indexOf(':') + 1).trimmed();
            QString taskStatus = statusToString(t->status()).toLower();
            matchesText = taskStatus.contains(statusQuery);
        }
        // Recherche par date: "date:2024" ou "due:2024-12"
        else if (searchLower.startsWith("date:") || searchLower.startsWith("due:") || searchLower.startsWith("échéance:")) {
            QString dateQuery = searchLower.mid(searchLower.indexOf(':') + 1).trimmed();
            if (t->dueDate().isValid()) {
                matchesText = t->dueDate().toString(Qt::ISODate).contains(dateQuery);
            } else {
                matchesText = false;
            }
        }
        // Recherche classique dans titre et description
        else {
            matchesText = t->title().contains(m_searchText, Qt::CaseInsensitive) || 
                         t->description().contains(m_searchText, Qt::CaseInsensitive);
        }
    }
    
    // Filtrer par status complété
    bool matchesCompleted = m_showCompleted || (t->status() != Status::COMPLETED);
    
    // Filtrer par priorité si activé
    bool matchesPriority = !m_priorityFilterEnabled || (t->priority() == m_priorityFilter);
    
    // Filtrer par status si activé
    bool matchesStatus = !m_statusFilterEnabled || (t->status() == m_statusFilter);

    return matchesText && matchesCompleted && matchesPriority && matchesStatus;
}
