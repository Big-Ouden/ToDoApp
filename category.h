#ifndef CATEGORY_H
#define CATEGORY_H

#pragma once

#include <QObject>
#include <QString>
#include <QColor>
#include <QList>
#include <QUuid>

class Task;

class Category : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit Category(const QString &name = "", QObject *parent = nullptr);

    const QString &name() const { return m_name; }
    const QColor &color() const { return m_color; }
    const QList<Task*>& tasks() const { return m_tasks; }

    void setName(const QString &n);
    void setColor(const QColor &c);

    void addTask(Task *t);
    void removeTask(Task *t);

signals:
    void nameChanged();
    void colorChanged();

private:
    QUuid m_id;
    QString m_name;
    QColor m_color;
    QList<Task*> m_tasks;   // Agrégation faible
};


#endif // CATEGORY_H
