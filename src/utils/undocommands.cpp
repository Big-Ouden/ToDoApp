#include "undocommands.h"

// ========================================
// AddTaskCommand
// ========================================

AddTaskCommand::AddTaskCommand(TaskModel *model, Task *task, Task *parent, QUndoCommand *parentCmd)
    : QUndoCommand(parentCmd),
      m_model(model),
      m_task(task),
      m_parentTask(parent),
      m_ownsTask(true) // On possède la tâche avant le premier redo
{
    if (parent) {
        setText(QObject::tr("Ajouter une sous-tâche '%1'").arg(task->title()));
    } else {
        setText(QObject::tr("Ajouter une tâche '%1'").arg(task->title()));
    }
}

AddTaskCommand::~AddTaskCommand()
{
    // Nettoyer la tâche si on en est propriétaire
    if (m_ownsTask) {
        delete m_task;
    }
}

void AddTaskCommand::undo()
{
    // Détacher la tâche du modèle (sans la supprimer)
    m_model->detachTaskByPointer(m_task);
    m_ownsTask = true; // On reprend possession
}

void AddTaskCommand::redo()
{
    // Ajouter la tâche au modèle
    m_model->insertTask(m_task, m_parentTask);
    m_ownsTask = false; // Le modèle possède maintenant la tâche
}

// ========================================
// RemoveTaskCommand
// ========================================

RemoveTaskCommand::RemoveTaskCommand(TaskModel *model, Task *task, QUndoCommand *parentCmd)
    : QUndoCommand(parentCmd),
      m_model(model),
      m_task(task),
      m_parentTask(task->parentTask()),
      m_ownsTask(false) // Le modèle possède la tâche avant le premier redo
{
    setText(QObject::tr("Supprimer la tâche '%1'").arg(task->title()));
    
    // Sauvegarder la position
    if (m_parentTask) {
        m_row = m_parentTask->subtasks().indexOf(task);
    } else {
        m_row = m_model->rootTasks().indexOf(task);
    }
}

RemoveTaskCommand::~RemoveTaskCommand()
{
    // Nettoyer la tâche si on en est propriétaire
    if (m_ownsTask) {
        delete m_task;
    }
}

void RemoveTaskCommand::undo()
{
    // Réinsérer la tâche au modèle à sa position d'origine
    m_model->insertTask(m_task, m_parentTask);
    m_ownsTask = false; // Le modèle reprend possession
}

void RemoveTaskCommand::redo()
{
    // Détacher la tâche du modèle (sans la supprimer)
    m_model->detachTaskByPointer(m_task);
    m_ownsTask = true; // On prend possession
}

// ========================================
// ModifyTaskCommand
// ========================================

ModifyTaskCommand::ModifyTaskCommand(Task *task, const QString &property, const QVariant &oldValue, const QVariant &newValue, QUndoCommand *parentCmd)
    : QUndoCommand(parentCmd),
      m_task(task),
      m_property(property),
      m_oldValue(oldValue),
      m_newValue(newValue)
{
    setText(QObject::tr("Modifier '%1' de '%2'").arg(property).arg(task->title()));
}

void ModifyTaskCommand::undo()
{
    // Restaurer l'ancienne valeur
    m_task->setProperty(m_property.toUtf8().constData(), m_oldValue);
}

void ModifyTaskCommand::redo()
{
    // Appliquer la nouvelle valeur
    m_task->setProperty(m_property.toUtf8().constData(), m_newValue);
}
