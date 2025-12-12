#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <QUndoCommand>
#include "task.h"
#include "taskmodel.h"

/**
 * @brief Commande pour ajouter une tâche (Undo/Redo).
 */
class AddTaskCommand : public QUndoCommand
{
public:
    AddTaskCommand(TaskModel *model, Task *task, Task *parent = nullptr, QUndoCommand *parentCmd = nullptr);
    ~AddTaskCommand();
    
    void undo() override;
    void redo() override;
    
private:
    TaskModel *m_model;
    Task *m_task;
    Task *m_parentTask;
    bool m_ownsTask; // True si la commande possède la tâche (après undo)
};

/**
 * @brief Commande pour supprimer une tâche (Undo/Redo).
 */
class RemoveTaskCommand : public QUndoCommand
{
public:
    RemoveTaskCommand(TaskModel *model, Task *task, QUndoCommand *parentCmd = nullptr);
    ~RemoveTaskCommand();
    
    void undo() override;
    void redo() override;
    
private:
    TaskModel *m_model;
    Task *m_task;
    Task *m_parentTask;
    int m_row; // Position dans la liste parent
    bool m_ownsTask; // True si la commande possède la tâche (après redo)
};

/**
 * @brief Commande pour modifier une tâche (Undo/Redo).
 */
class ModifyTaskCommand : public QUndoCommand
{
public:
    ModifyTaskCommand(Task *task, const QString &property, const QVariant &oldValue, const QVariant &newValue, QUndoCommand *parentCmd = nullptr);
    
    void undo() override;
    void redo() override;
    
private:
    Task *m_task;
    QString m_property;
    QVariant m_oldValue;
    QVariant m_newValue;
};

#endif // UNDOCOMMANDS_H
