#include "task.h"
#include <QDebug>

/**
 * @file task.cpp
 * @brief Implémentation de la classe Task (modèle d'une tâche).
 */

Task::Task(const QString &title, QObject *parent)
    : QObject(parent),
    m_id(QUuid::createUuid()),
    m_title(title),
    m_dueDate(),
    m_priority(Priority::LOW),
    m_status(Status::NOTSTARTED),
    m_parentTask(nullptr)
{
}

/**
 * @brief Retourne le titre de la tâche.
 */
const QString &Task::title() const { return m_title; }

/**
 * @brief Définit le titre de la tâche et émet titleChanged() et taskModified() si modifié.
 */
void Task::setTitle(const QString &t)
{
    if (m_title == t) return;
    m_title = t;
    emit titleChanged();
    emit taskModified();
}

/**
 * @brief Retourne la description.
 */
const QString &Task::description() const { return m_description; }

/**
 * @brief Définit la description et notifie.
 */
void Task::setDescription(const QString &d)
{
    if (m_description == d) return;
    m_description = d;
    emit descriptionChanged();
    emit taskModified();
}

/**
 * @brief Retourne la date d'échéance.
 */
const QDate &Task::dueDate() const { return m_dueDate; }

/**
 * @brief Définit la date d'échéance et notifie.
 */
void Task::setDueDate(const QDate &date)
{
    if (m_dueDate == date) return;
    m_dueDate = date;
    emit dueDateChanged();
    emit taskModified();
}

/**
 * @brief Retourne la priorité.
 */
Priority Task::priority() const { return m_priority; }

/**
 * @brief Définit la priorité et notifie.
 */
void Task::setPriority(Priority p)
{
    if (m_priority == p) return;
    m_priority = p;
    emit priorityChanged();
    emit taskModified();
}

/**
 * @brief Retourne le statut.
 */
Status Task::status() const { return m_status; }

/**
 * @brief Définit le statut et notifie.
 */
void Task::setStatus(Status s)
{
    if (m_status == s) return;
    m_status = s;
    emit statusChanged();
    emit taskModified();
}

/**
 * @brief Indique si la tâche est complétée.
 */
bool Task::isCompleted() const { return m_status == Status::COMPLETED; }

/**
 * @brief Indique si la tâche est en retard.
 */
bool Task::isOverdue() const
{
    if (!m_dueDate.isValid()) return false;
    return QDate::currentDate() > m_dueDate && !isCompleted();
}

/**
 * @brief Calcule le pourcentage de complétion basé sur les sous-tâches.
 * @return entier entre 0 et 100.
 */
int Task::completionPercentage() const
{
    if (m_subtasks.isEmpty()) return isCompleted() ? 100 : 0;
    int done = 0;
    for (const Task* t : m_subtasks)
        if (t->isCompleted()) ++done;
    return (done * 100) / m_subtasks.size();
}

/**
 * @brief Retourne le parent logique de la tâche (nullptr si racine).
 */
Task *Task::parentTask() const { return m_parentTask; }

/**
 * @brief Accès à la liste des sous-tâches.
 */
const QList<Task*>& Task::subtasks() const { return m_subtasks; }

/**
 * @brief Ajoute une sous-tâche. Met à jour le parent de la sous-tâche et émet subtaskAdded.
 */
void Task::addSubtask(Task *t)
{
    if (!t) return;
    if (m_subtasks.contains(t)) return;
    m_subtasks.append(t);
    t->setParent(this); // QObject parent ownership optional
    t->m_parentTask = this;
    emit subtaskAdded(t);
    emit taskModified();
}

/**
 * @brief Insère une sous-tâche à une position donnée.
 */
void Task::insertSubtask(int index, Task *t)
{
    if (!t) return;
    if (m_subtasks.contains(t)) return;
    m_subtasks.insert(index, t);
    t->setParent(this);
    t->m_parentTask = this;
    emit subtaskAdded(t);
    emit taskModified();
}

/**
 * @brief Supprime une sous-tâche (ne delete pas l'objet, laisse la stratégie mémoire au modèle).
 * @return true si trouvée et retirée.
 */
void Task::removeSubtask(Task *t)
{
    if (!t) return;
    if (!m_subtasks.removeOne(t)) return;
    t->m_parentTask = nullptr;
    emit subtaskRemoved(t);
    emit taskModified();
}

/**
 * @brief Définit le parent de la tâche.
 */
void Task::setParentTask(Task *parent)
{
    m_parentTask = parent;
}
