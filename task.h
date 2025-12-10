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

public:
    explicit Task(const QString &title = "", QObject *parent = nullptr);

    const QString &title() const;
    const QString &description() const;
    const QDate &dueDate() const;
    Priority priority() const;
    Status status() const;
    QUuid id() const { return m_id; }

    void setTitle(const QString &t);
    void setDescription(const QString &d);
    void setDueDate(const QDate &date);
    void setPriority(Priority p);
    void setStatus(Status s);
    
    // Tags
    const QStringList &tags() const;
    void setTags(const QStringList &tags);
    void addTag(const QString &tag);
    void removeTag(const QString &tag);
    bool hasTag(const QString &tag) const;
    
    // Attachments
    const QList<QUrl> &attachments() const;
    void setAttachments(const QList<QUrl> &attachments);
    void addAttachment(const QUrl &url);
    void removeAttachment(const QUrl &url);

    bool isCompleted() const;
    bool isOverdue() const;
    int completionPercentage() const;

    Task *parentTask() const;
    const QList<Task*>& subtasks() const;
    void setParentTask(Task *parent);

    void addSubtask(Task *t);
    void insertSubtask(int index, Task *t);
    void removeSubtask(Task *t);

signals:
    void titleChanged();
    void descriptionChanged();
    void dueDateChanged();
    void priorityChanged();
    void statusChanged();
    void tagsChanged();
    void attachmentsChanged();
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

    Task *m_parentTask;
    QList<Task*> m_subtasks;
};


#endif // TASK_H
