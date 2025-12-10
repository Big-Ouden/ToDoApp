#include "taskmodel.h"
#include <QBrush>
#include <QIcon>

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
    return 6; // Title, Description, DueDate, Priority, Status, Category
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return {};

    Task *t = static_cast<Task*>(index.internalPointer());
    if (!t) return {};

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0: return t->title();
        case 1: return t->description();
        case 2: return t->dueDate().isValid() ? t->dueDate().toString(Qt::ISODate) : QString();
        case 3: return priorityToString(t->priority());
        case 4: return statusToString(t->status());
        case 5: return QString(); // category name resolution omitted for brevity
        default: return {};
        }
    }

    if (role == Qt::ForegroundRole && t->isOverdue()) {
        return QBrush(Qt::red);
    }

    if (role == Qt::DecorationRole && index.column() == 3) {
        // optional: return an icon for priority
        switch (t->priority()) {
        case Priority::Low: return QIcon();
        case Priority::Medium: return QIcon();
        case Priority::High: return QIcon();
        case Priority::Critical: return QIcon();
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
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
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
        emit dataChanged(index(0,0,QModelIndex()), index(rowCount(QModelIndex())-1, columnCount(QModelIndex())-1));
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

Task *TaskModel::getTask(const QModelIndex &index) const
{
    if (!index.isValid()) return nullptr;
    return static_cast<Task*>(index.internalPointer());
}

void TaskModel::clear()
{
    beginResetModel();
    qDeleteAll(m_rootTasks);
    m_rootTasks.clear();
    endResetModel();
}
