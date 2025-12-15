#ifndef TASKFILTERPROXYMODEL_H
#define TASKFILTERPROXYMODEL_H

#pragma once

#include <QSortFilterProxyModel>
#include <QString>

#include "priority.h"
#include "status.h"

/**
 * @brief Filtre et recherche avancée pour les tâches.
 */
class TaskFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    /** @brief Constructeur du proxy de filtrage */
    explicit TaskFilterProxyModel(QObject *parent = nullptr);

    /**
     * @brief Définit le texte de recherche.
     * @param text Texte à rechercher (supporte les préfixes tag:, priority:, status:, date:)
     */
    void setSearchText(const QString &text);
    
    /**
     * @brief Active/désactive le filtre par priorité.
     * @param p Niveau de priorité à filtrer
     * @param enabled true pour activer le filtre
     */
    void setPriorityFilter(Priority p, bool enabled = true);
    
    /**
     * @brief Active/désactive le filtre par statut.
     * @param s Statut à filtrer
     * @param enabled true pour activer le filtre
     */
    void setStatusFilter(Status s, bool enabled = true);
    
    /**
     * @brief Affiche ou masque les tâches terminées.
     * @param show true pour afficher les tâches terminées
     */
    void setShowCompleted(bool show);

protected:
    /**
     * @brief Détermine si une ligne doit être affichée.
     * @param row Numéro de ligne
     * @param parent Index parent
     * @return true si la ligne doit être affichée
     */
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

private:
    QString m_searchText;
    Priority m_priorityFilter;
    Status m_statusFilter;
    bool m_showCompleted;
    bool m_priorityFilterEnabled;
    bool m_statusFilterEnabled;
};


#endif // TASKFILTERPROXYMODEL_H
