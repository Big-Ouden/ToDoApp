#ifndef TASK_H
#define TASK_H

#pragma once

#include <QObject>
#include <QString>
#include <QDate>
#include <QList>
#include <QUuid>
#include <QUrl>

#include "priority.h"
#include "status.h"

class Category;

/**
 * @brief Représente une tâche avec titre, description et sous-tâches.
 */
class Task : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QDate dueDate READ dueDate WRITE setDueDate NOTIFY dueDateChanged)
    Q_PROPERTY(Priority priority READ priority WRITE setPriority NOTIFY priorityChanged)
    Q_PROPERTY(Status status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QStringList tags READ tags WRITE setTags NOTIFY tagsChanged)
    Q_PROPERTY(QList<QUrl> attachments READ attachments WRITE setAttachments NOTIFY attachmentsChanged)
    Q_PROPERTY(int estimatedMinutes READ estimatedMinutes WRITE setEstimatedMinutes NOTIFY estimatedMinutesChanged)
    Q_PROPERTY(int actualMinutes READ actualMinutes WRITE setActualMinutes NOTIFY actualMinutesChanged)
    Q_PROPERTY(QDate completionDate READ completionDate WRITE setCompletionDate NOTIFY completionDateChanged)
    Q_PROPERTY(QString linkedIssueId READ linkedIssueId WRITE setLinkedIssueId NOTIFY linkedIssueIdChanged)

public:
    /**
     * @brief Constructeur d'une tâche.
     * @param title Titre de la tâche
     * @param parent Objet parent Qt
     */
    explicit Task(const QString &title = "", QObject *parent = nullptr);

    /** @return Titre de la tâche */
    const QString &title() const;
    
    /** @return Description de la tâche */
    const QString &description() const;
    
    /** @return Date d'échéance */
    const QDate &dueDate() const;
    
    /** @return Niveau de priorité */
    Priority priority() const;
    
    /** @return Statut actuel */
    Status status() const;
    
    /** @return Identifiant unique de la tâche */
    QUuid id() const { return m_id; }

    /**
     * @brief Définit le titre de la tâche.
     * @param t Nouveau titre
     */
    void setTitle(const QString &t);
    
    /**
     * @brief Définit la description de la tâche.
     * @param d Nouvelle description
     */
    void setDescription(const QString &d);
    
    /**
     * @brief Définit la date d'échéance.
     * @param date Nouvelle date d'échéance
     */
    void setDueDate(const QDate &date);
    
    /**
     * @brief Définit la priorité.
     * @param p Nouveau niveau de priorité
     */
    void setPriority(Priority p);
    
    /**
     * @brief Définit le statut.
     * @param s Nouveau statut
     */
    void setStatus(Status s);
    
    /// Gestion des étiquettes
    
    /** @return Liste des étiquettes */
    const QStringList &tags() const;
    
    /**
     * @brief Définit la liste complète des étiquettes.
     * @param tags Nouvelle liste d'étiquettes
     */
    void setTags(const QStringList &tags);
    
    /**
     * @brief Ajoute une étiquette.
     * @param tag Étiquette à ajouter
     */
    void addTag(const QString &tag);
    
    /**
     * @brief Supprime une étiquette.
     * @param tag Étiquette à supprimer
     */
    void removeTag(const QString &tag);
    
    /**
     * @brief Vérifie si la tâche possède une étiquette.
     * @param tag Étiquette à rechercher
     * @return true si l'étiquette existe
     */
    bool hasTag(const QString &tag) const;
    
    /// Gestion des pièces jointes
    
    /** @return Liste des pièces jointes */
    const QList<QUrl> &attachments() const;
    
    /**
     * @brief Définit la liste complète des pièces jointes.
     * @param attachments Nouvelle liste de pièces jointes
     */
    void setAttachments(const QList<QUrl> &attachments);
    
    /**
     * @brief Ajoute une pièce jointe.
     * @param url URL du fichier à attacher
     */
    void addAttachment(const QUrl &url);
    
    /**
     * @brief Supprime une pièce jointe.
     * @param url URL du fichier à détacher
     */
    void removeAttachment(const QUrl &url);

    /** @return true si la tâche est terminée */
    bool isCompleted() const;
    
    /** @return true si la tâche est en retard */
    bool isOverdue() const;
    
    /// Gestion du temps
    
    /** @return Temps estimé en minutes */
    int estimatedMinutes() const;
    
    /** @return Temps réel passé en minutes */
    int actualMinutes() const;
    
    /**
     * @brief Définit le temps estimé.
     * @param minutes Temps estimé en minutes
     */
    void setEstimatedMinutes(int minutes);
    
    /**
     * @brief Définit le temps réel passé.
     * @param minutes Temps réel en minutes
     */
    void setActualMinutes(int minutes);
    
    /**
     * @brief Ajoute du temps au temps réel passé.
     * @param minutes Minutes à ajouter
     */
    void addActualMinutes(int minutes);
    
    /** @return Date de complétion de la tâche */
    const QDate &completionDate() const;
    
    /**
     * @brief Définit la date de complétion.
     * @param date Date de complétion
     */
    void setCompletionDate(const QDate &date);
    
    /// Liaison avec issues Git
    
    /** @return ID de l'issue Git liée (format: "owner/repo#123") */
    const QString &linkedIssueId() const;
    
    /**
     * @brief Définit l'ID de l'issue Git liée.
     * @param issueId ID au format "owner/repo#123" ou vide pour aucune liaison
     */
    void setLinkedIssueId(const QString &issueId);
    
    /**
     * @brief Vérifie si la tâche est liée à une issue Git.
     * @return true si linkedIssueId n'est pas vide
     */
    bool hasLinkedIssue() const;
    
    /** @return Pourcentage de complétion (0-100) */
    int completionPercentage() const;

    /** @return Tâche parente ou nullptr si tâche racine */
    Task *parentTask() const;
    
    /** @return Liste des sous-tâches */
    const QList<Task*>& subtasks() const;
    
    /**
     * @brief Définit la tâche parente.
     * @param parent Nouvelle tâche parente
     */
    void setParentTask(Task *parent);

    /**
     * @brief Ajoute une sous-tâche à la fin.
     * @param t Sous-tâche à ajouter
     */
    void addSubtask(Task *t);
    
    /**
     * @brief Insère une sous-tâche à une position donnée.
     * @param index Position d'insertion
     * @param t Sous-tâche à insérer
     */
    void insertSubtask(int index, Task *t);
    
    /**
     * @brief Supprime une sous-tâche.
     * @param t Sous-tâche à supprimer
     */
    void removeSubtask(Task *t);

signals:
    void titleChanged();
    void descriptionChanged();
    void dueDateChanged();
    void priorityChanged();
    void statusChanged();
    void tagsChanged();
    void attachmentsChanged();
    void estimatedMinutesChanged();
    void actualMinutesChanged();
    void completionDateChanged();
    void linkedIssueIdChanged();
    void taskModified();

    void subtaskAdded(Task*);
    void subtaskRemoved(Task*);

private:
    QUuid m_id;
    QString m_title;
    QString m_description;
    QDate m_dueDate;
    Priority m_priority;
    Status m_status;
    QStringList m_tags;
    QList<QUrl> m_attachments;
    int m_estimatedMinutes;
    int m_actualMinutes;
    QDate m_completionDate;
    QString m_linkedIssueId;

    Task *m_parentTask;
    QList<Task*> m_subtasks;
};


#endif // TASK_H
