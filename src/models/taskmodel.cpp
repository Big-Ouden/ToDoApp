#include "taskmodel.h"
#include <QBrush>
#include <QIcon>
#include <QMimeData>
#include <QDataStream>
#include <QIODevice>
#include <functional>

/**
 * @file taskmodel.cpp
 * @brief Implémentation d'un modèle hiérarchique de tâches (QAbstractItemModel).
 *
 * Le modèle possède les tasks racines dans m_rootTasks. Les sous-tâches
 * sont accessibles via Task::subtasks() et le pointeur internalPointer() des QModelIndex
 * pointe vers l'objet Task correspondant.
 */

TaskModel::TaskModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex TaskModel::index(int row, int col, const QModelIndex &parent) const
{
    if (row < 0 || col < 0) return QModelIndex();

    Task *parentTask = parent.isValid() ? static_cast<Task*>(parent.internalPointer()) : nullptr;
    Task *child = nullptr;

    if (parentTask) {
        if (row >= 0 && row < parentTask->subtasks().size())
            child = parentTask->subtasks().at(row);
    } else {
        if (row >= 0 && row < m_rootTasks.size())
            child = m_rootTasks.at(row);
    }

    if (!child) return QModelIndex();
    return createIndex(row, col, child);
}

QModelIndex TaskModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) return QModelIndex();
    Task *childTask = static_cast<Task*>(child.internalPointer());
    Task *p = childTask->parentTask();
    if (!p) return QModelIndex();

    // find row of p in its parent
    Task *gp = p->parentTask();
    int row = 0;
    if (gp) {
        row = gp->subtasks().indexOf(p);
    } else {
        row = m_rootTasks.indexOf(p);
    }
    return createIndex(row, 0, p);
}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_rootTasks.size();
    Task *p = static_cast<Task*>(parent.internalPointer());
    return p ? p->subtasks().size() : 0;
}

int TaskModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 5; // Title, DueDate, Priority, Status, Tags
}

QVariant TaskModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return tr("Titre");
        case 1: return tr("Date d'échéance");
        case 2: return tr("Priorité");
        case 3: return tr("Statut");
        case 4: return tr("Étiquettes");
        default: return {};
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return {};

    Task *t = static_cast<Task*>(index.internalPointer());
    if (!t) return {};

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0: return t->title();
        case 1: return t->dueDate().isValid() ? t->dueDate().toString(Qt::ISODate) : QString();
        case 2: return tr(priorityToString(t->priority()).toUtf8().constData());
        case 3: return tr(statusToString(t->status()).toUtf8().constData());
        case 4: return t->tags().join(", ");
        default: return {};
        }
    }

    if (role == Qt::BackgroundRole && index.column() == 3) {
        // Colorer le status selon le thème
        if (m_isDarkMode) {
            switch (t->status()) {
            case Status::NOTSTARTED: return QBrush(QColor(70, 70, 70)); // Gris foncé
            case Status::INPROGRESS: return QBrush(QColor(37, 99, 235)); // Bleu foncé
            case Status::COMPLETED: return QBrush(QColor(34, 139, 34)); // Vert foncé
            case Status::CANCELLED: return QBrush(QColor(153, 27, 27)); // Rouge foncé
            }
        } else {
            switch (t->status()) {
            case Status::NOTSTARTED: return QBrush(QColor(220, 220, 220)); // Gris clair
            case Status::INPROGRESS: return QBrush(QColor(173, 216, 230)); // Bleu clair
            case Status::COMPLETED: return QBrush(QColor(144, 238, 144)); // Vert clair
            case Status::CANCELLED: return QBrush(QColor(255, 200, 200)); // Rouge clair
            }
        }
    }
    
    if (role == Qt::BackgroundRole && index.column() == 2) {
        // Colorer la priorité selon le thème
        if (m_isDarkMode) {
            switch (t->priority()) {
            case Priority::LOW: return QBrush(QColor(34, 139, 34)); // Vert foncé
            case Priority::MEDIUM: return QBrush(QColor(184, 134, 11)); // Jaune/or foncé
            case Priority::HIGH: return QBrush(QColor(255, 140, 0)); // Orange
            case Priority::CRITICAL: return QBrush(QColor(220, 38, 38)); // Rouge vif
            }
        } else {
            switch (t->priority()) {
            case Priority::LOW: return QBrush(QColor(200, 255, 200)); // Vert pâle
            case Priority::MEDIUM: return QBrush(QColor(255, 255, 200)); // Jaune pâle
            case Priority::HIGH: return QBrush(QColor(255, 220, 180)); // Orange pâle
            case Priority::CRITICAL: return QBrush(QColor(255, 180, 180)); // Rouge pâle
            }
        }
    }
    
    if (role == Qt::ForegroundRole && t->isOverdue() && index.column() == 1) {
        // Seulement la date en rouge si en retard
        return QBrush(Qt::red);
    }

    if (role == Qt::DecorationRole && index.column() == 2) {
        // optional: return an icon for priority
        switch (t->priority()) {
        case Priority::LOW: return QIcon();
        case Priority::MEDIUM: return QIcon();
        case Priority::HIGH: return QIcon();
        case Priority::CRITICAL: return QIcon();
        }
    }

    return {};
}

bool TaskModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    Task *t = static_cast<Task*>(index.internalPointer());
    if (!t) return false;

    if (role == Qt::EditRole) {
        switch (index.column()) {
        case 0: t->setTitle(value.toString()); break;
        case 1: t->setDescription(value.toString()); break;
        case 2: t->setDueDate(QDate::fromString(value.toString(), Qt::ISODate)); break;
        default: break;
        }
        emit dataChanged(index, index);
        emit taskUpdated(t);
        return true;
    }
    return false;
}

Qt::ItemFlags TaskModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    
    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

void TaskModel::insertTask(Task *task, Task *parentTask)
{
    if (!task) return;
    if (!parentTask) {
        beginInsertRows(QModelIndex(), m_rootTasks.size(), m_rootTasks.size());
        m_rootTasks.append(task);
        endInsertRows();
    } else {
        // insert as subtask of parentTask
        int pos = parentTask->subtasks().size();
        QModelIndex pidx = createIndex( (parentTask->parentTask() ? parentTask->parentTask()->subtasks().indexOf(parentTask) : m_rootTasks.indexOf(parentTask)) , 0, parentTask);
        beginInsertRows(pidx, pos, pos);
        parentTask->addSubtask(task);
        endInsertRows();
    }

    // connect signals
    connect(task, &Task::taskModified, this, [this, task]() {
        // find index and emit dataChanged for the row
        // naive approach: emit dataChanged for whole model root
        emit dataChanged(index(0,0,QModelIndex()), index(rowCount(QModelIndex())-1, columnCount(QModelIndex())-1, QModelIndex()));
        emit taskUpdated(task);
    });

    emit taskAdded(task);
}

void TaskModel::removeTask(const QModelIndex &index)
{
    if (!index.isValid()) return;
    Task *t = static_cast<Task*>(index.internalPointer());
    Task *parentTask = t->parentTask();

    if (!parentTask) {
        int row = m_rootTasks.indexOf(t);
        if (row >= 0) {
            beginRemoveRows(QModelIndex(), row, row);
            m_rootTasks.removeAt(row);
            endRemoveRows();
            emit taskRemoved(t);
            delete t;
        }
    } else {
        int row = parentTask->subtasks().indexOf(t);
        QModelIndex pidx = createIndex( (parentTask->parentTask() ? parentTask->parentTask()->subtasks().indexOf(parentTask) : m_rootTasks.indexOf(parentTask)), 0, parentTask);
        if (row >= 0) {
            beginRemoveRows(pidx, row, row);
            parentTask->removeSubtask(t);
            endRemoveRows();
            emit taskRemoved(t);
            delete t;
        }
    }
}

bool TaskModel::removeTaskByPointer(Task *task)
{
    if (!task) return false;
    
    Task *parentTask = task->parentTask();
    
    if (!parentTask) {
        // Tâche racine
        int row = m_rootTasks.indexOf(task);
        if (row >= 0) {
            beginRemoveRows(QModelIndex(), row, row);
            m_rootTasks.removeAt(row);
            endRemoveRows();
            emit taskRemoved(task);
            delete task;
            return true;
        }
    } else {
        // Sous-tâche
        int row = parentTask->subtasks().indexOf(task);
        if (row >= 0) {
            QModelIndex pidx = createIndex(
                (parentTask->parentTask() ? parentTask->parentTask()->subtasks().indexOf(parentTask) : m_rootTasks.indexOf(parentTask)), 
                0, 
                parentTask
            );
            beginRemoveRows(pidx, row, row);
            parentTask->removeSubtask(task);
            endRemoveRows();
            emit taskRemoved(task);
            delete task;
            return true;
        }
    }
    
    return false;
}

bool TaskModel::detachTaskByPointer(Task *task)
{
    if (!task) return false;
    
    Task *parentTask = task->parentTask();
    
    if (!parentTask) {
        // Tâche racine
        int row = m_rootTasks.indexOf(task);
        if (row >= 0) {
            beginRemoveRows(QModelIndex(), row, row);
            m_rootTasks.removeAt(row);
            endRemoveRows();
            // Ne PAS delete la tâche - elle est détachée pour Undo/Redo
            return true;
        }
    } else {
        // Sous-tâche
        int row = parentTask->subtasks().indexOf(task);
        if (row >= 0) {
            QModelIndex pidx = createIndex(
                (parentTask->parentTask() ? parentTask->parentTask()->subtasks().indexOf(parentTask) : m_rootTasks.indexOf(parentTask)), 
                0, 
                parentTask
            );
            beginRemoveRows(pidx, row, row);
            parentTask->removeSubtask(task);
            endRemoveRows();
            // Ne PAS delete la tâche - elle est détachée pour Undo/Redo
            return true;
        }
    }
    
    return false;
}

Task *TaskModel::getTask(const QModelIndex &index) const
{
    if (!index.isValid()) return nullptr;
    return static_cast<Task*>(index.internalPointer());
}

QModelIndex TaskModel::getIndexForTask(Task *task) const
{
    if (!task) return QModelIndex();
    
    // Fonction récursive pour chercher la tâche
    std::function<QModelIndex(const QList<Task*>&, Task*, const QModelIndex&)> findTask;
    findTask = [&](const QList<Task*>& tasks, Task* target, const QModelIndex& parent) -> QModelIndex {
        for (int row = 0; row < tasks.size(); ++row) {
            Task *t = tasks.at(row);
            if (t == target) {
                return index(row, 0, parent);
            }
            // Chercher dans les sous-tâches
            if (!t->subtasks().isEmpty()) {
                QModelIndex parentIdx = index(row, 0, parent);
                QModelIndex result = findTask(t->subtasks(), target, parentIdx);
                if (result.isValid()) {
                    return result;
                }
            }
        }
        return QModelIndex();
    };
    
    return findTask(m_rootTasks, task, QModelIndex());
}

void TaskModel::clear()
{
    beginResetModel();
    qDeleteAll(m_rootTasks);
    m_rootTasks.clear();
    endResetModel();
}

void TaskModel::promoteTask(const QModelIndex &index)
{
    if (!index.isValid()) return;
    
    Task *task = getTask(index);
    if (!task) return;
    
    Task *parent = task->parentTask();
    if (!parent) return; // Déjà au niveau racine
    
    Task *grandParent = parent->parentTask();
    
    // Retirer de l'ancien parent
    int oldRow = parent->subtasks().indexOf(task);
    QModelIndex parentIdx = createIndex(
        grandParent ? grandParent->subtasks().indexOf(parent) : m_rootTasks.indexOf(parent),
        0, parent);
    
    beginRemoveRows(parentIdx, oldRow, oldRow);
    parent->removeSubtask(task);
    endRemoveRows();
    
    // Ajouter au même niveau que l'ancien parent
    if (grandParent) {
        int newRow = grandParent->subtasks().indexOf(parent) + 1;
        QModelIndex grandParentIdx = createIndex(
            grandParent->parentTask() ? grandParent->parentTask()->subtasks().indexOf(grandParent) : m_rootTasks.indexOf(grandParent),
            0, grandParent);
        
        beginInsertRows(grandParentIdx, newRow, newRow);
        grandParent->insertSubtask(newRow, task);
        endInsertRows();
    } else {
        int newRow = m_rootTasks.indexOf(parent) + 1;
        beginInsertRows(QModelIndex(), newRow, newRow);
        m_rootTasks.insert(newRow, task);
        task->setParentTask(nullptr);
        endInsertRows();
    }
    
    emit taskUpdated(task);
}

Qt::DropActions TaskModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList TaskModel::mimeTypes() const
{
    return QStringList() << "application/x-task-pointer";
}

QMimeData *TaskModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty()) return nullptr;
    
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    
    // Encoder le pointeur de la première tâche sélectionnée
    QModelIndex index = indexes.first();
    if (index.isValid()) {
        Task *task = getTask(index);
        stream << reinterpret_cast<quintptr>(task);
    }
    
    mimeData->setData("application/x-task-pointer", encodedData);
    return mimeData;
}

bool TaskModel::canDropMimeData(const QMimeData *data, Qt::DropAction action,
                                 int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    
    if (!data->hasFormat("application/x-task-pointer"))
        return false;
    
    if (action != Qt::MoveAction)
        return false;
    
    return true;
}

bool TaskModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                              int row, int column, const QModelIndex &parent)
{
    if (!canDropMimeData(data, action, row, column, parent))
        return false;
    
    if (action == Qt::IgnoreAction)
        return true;
    
    // Décoder la tâche
    QByteArray encodedData = data->data("application/x-task-pointer");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    
    quintptr taskPtr;
    stream >> taskPtr;
    Task *draggedTask = reinterpret_cast<Task*>(taskPtr);
    
    if (!draggedTask) return false;
    
    // Vérifier qu'on ne déplace pas dans ses propres sous-tâches
    Task *newParent = parent.isValid() ? getTask(parent) : nullptr;
    Task *checkParent = newParent;
    while (checkParent) {
        if (checkParent == draggedTask) {
            return false; // Impossible de déplacer dans ses propres sous-tâches
        }
        checkParent = checkParent->parentTask();
    }
    
    // Retirer de l'ancien emplacement
    Task *oldParent = draggedTask->parentTask();
    int oldRow = -1;
    QModelIndex oldParentIdx;
    
    if (oldParent) {
        oldRow = oldParent->subtasks().indexOf(draggedTask);
        oldParentIdx = createIndex(
            oldParent->parentTask() ? oldParent->parentTask()->subtasks().indexOf(oldParent) : m_rootTasks.indexOf(oldParent),
            0, oldParent);
        beginRemoveRows(oldParentIdx, oldRow, oldRow);
        oldParent->removeSubtask(draggedTask);
        endRemoveRows();
    } else {
        oldRow = m_rootTasks.indexOf(draggedTask);
        beginRemoveRows(QModelIndex(), oldRow, oldRow);
        m_rootTasks.removeAt(oldRow);
        endRemoveRows();
    }
    
    // Insérer au nouvel emplacement
    int newRow = (row >= 0) ? row : (newParent ? newParent->subtasks().size() : m_rootTasks.size());
    
    if (newParent) {
        beginInsertRows(parent, newRow, newRow);
        newParent->insertSubtask(newRow, draggedTask);
        endInsertRows();
    } else {
        beginInsertRows(QModelIndex(), newRow, newRow);
        m_rootTasks.insert(newRow, draggedTask);
        draggedTask->setParentTask(nullptr);
        endInsertRows();
    }
    
    emit taskUpdated(draggedTask);
    return true;
}

void TaskModel::setDarkMode(bool dark)
{
    m_isDarkMode = dark;
    // Forcer la mise à jour de toutes les cellules colorées
    if (!m_rootTasks.isEmpty()) {
        QModelIndex topLeft = index(0, 2, QModelIndex());
        QModelIndex bottomRight = index(rowCount(QModelIndex())-1, 3, QModelIndex());
        emit dataChanged(topLeft, bottomRight);
    }
}

void TaskModel::refreshAllData()
{
    // Fonction récursive pour mettre à jour toutes les lignes incluant les sous-tâches
    std::function<void(const QModelIndex&)> refreshRecursive;
    refreshRecursive = [&](const QModelIndex& parent) {
        int rows = rowCount(parent);
        if (rows > 0) {
            QModelIndex topLeft = index(0, 0, parent);
            QModelIndex bottomRight = index(rows - 1, columnCount(QModelIndex()) - 1, parent);
            emit dataChanged(topLeft, bottomRight);
            
            // Mettre à jour récursivement les enfants
            for (int i = 0; i < rows; ++i) {
                QModelIndex child = index(i, 0, parent);
                if (rowCount(child) > 0) {
                    refreshRecursive(child);
                }
            }
        }
    };
    
    // Commencer par la racine
    refreshRecursive(QModelIndex());
}
