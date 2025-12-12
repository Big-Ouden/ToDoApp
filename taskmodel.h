#ifndef TASKMODEL_H
#define TASKMODEL_H

#pragma once

#include <QAbstractItemModel>
#include <QList>

#include "task.h"
#include "category.h"

/**
 * @brief Modèle arborescent pour gérer les tâches et sous-tâches.
 */
class TaskModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /** @brief Constructeur du modèle */
    explicit TaskModel(QObject *parent = nullptr);

    // Index navigation
    
    /**
     * @brief Crée un index pour une cellule donnée.
     * @param row Ligne
     * @param col Colonne
     * @param parent Index parent
     * @return Index créé
     */
    QModelIndex index(int row, int col, const QModelIndex &parent) const override;
    
    /**
     * @brief Retourne l'index parent d'un enfant.
     * @param child Index enfant
     * @return Index parent
     */
    QModelIndex parent(const QModelIndex &child) const override;

    // Structure
    
    /**
     * @brief Retourne le nombre de lignes.
     * @param parent Index parent
     * @return Nombre de lignes
     */
    int rowCount(const QModelIndex &parent) const override;
    
    /**
     * @brief Retourne le nombre de colonnes.
     * @param parent Index parent
     * @return Nombre de colonnes
     */
    int columnCount(const QModelIndex &parent) const override;

    // Data
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // Modifications
    
    /**
     * @brief Insère une nouvelle tâche dans le modèle.
     * @param task Tâche à insérer
     * @param parentTask Tâche parente (nullptr pour une tâche racine)
     */
    void insertTask(Task *task, Task *parentTask = nullptr);
    
    /**
     * @brief Supprime une tâche du modèle.
     * @param index Index de la tâche à supprimer
     */
    void removeTask(const QModelIndex &index);
    
    /**
     * @brief Supprime une tâche du modèle par son pointeur.
     * @param task Pointeur vers la tâche à supprimer
     * @return true si la tâche a été trouvée et supprimée
     */
    bool removeTaskByPointer(Task *task);
    
    /**
     * @brief Détache une tâche du modèle sans la supprimer (pour Undo/Redo).
     * @param task Pointeur vers la tâche à détacher
     * @return true si la tâche a été trouvée et détachée
     */
    bool detachTaskByPointer(Task *task);
    
    /**
     * @brief Promouvoir une sous-tâche au même niveau que son parent.
     * @param index Index de la tâche à promouvoir
     */
    void promoteTask(const QModelIndex &index);
    
    // Drag & Drop
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    // Helpers
    
    /**
     * @brief Récupère la tâche associée à un index.
     * @param index Index du modèle
     * @return Pointeur vers la tâche ou nullptr
     */
    Task *getTask(const QModelIndex &index) const;
    
    /** @return Liste des tâches racines */
    const QList<Task*>& rootTasks() const { return m_rootTasks; }
    
    /** @brief Supprime toutes les tâches du modèle */
    void clear();
    
    /** @brief Active/désactive le mode sombre pour les couleurs */
    void setDarkMode(bool dark);
    
    /** @brief Force la mise à jour de toutes les cellules (pour retraduction) */
    void refreshAllData();

signals:
    void taskAdded(Task*);
    void taskRemoved(Task*);
    void taskUpdated(Task*);

private:
    QList<Task*> m_rootTasks;
    QList<Category*> m_categories;
    bool m_isDarkMode = false;
};


#endif // TASKMODEL_H
