#ifndef TASKMODEL_H
#define TASKMODEL_H

#pragma once

#include <QAbstractItemModel>
#include <QList>

#include "task.h"
#include "category.h"

class TaskModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TaskModel(QObject *parent = nullptr);

    // Index navigation
    QModelIndex index(int row, int col, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    // Structure
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    // Data
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Modifications
    void insertTask(Task *task, Task *parentTask = nullptr);
    void removeTask(const QModelIndex &index);

    // Helpers
    Task *getTask(const QModelIndex &index) const;
    const QList<Task*>& rootTasks() const { return m_rootTasks; }
    void clear();

signals:
    void taskAdded(Task*);
    void taskRemoved(Task*);
    void taskUpdated(Task*);

private:
    QList<Task*> m_rootTasks;
    QList<Category*> m_categories;
};


#endif // TASKMODEL_H
