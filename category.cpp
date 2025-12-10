#include "category.h"
#include "task.h"

/**
 * @file category.cpp
 * @brief Implémentation de Category (agrégation faible vers Task).
 */

Category::Category(const QString &name, QObject *parent)
    : QObject(parent),
    m_id(QUuid::createUuid()),
    m_name(name),
    m_color(Qt::transparent)
{
}

void Category::setName(const QString &n)
{
    if (m_name == n) return;
    m_name = n;
    emit nameChanged();
}

void Category::setColor(const QColor &c)
{
    if (m_color == c) return;
    m_color = c;
    emit colorChanged();
}

void Category::addTask(Task *t)
{
    if (!t) return;
    if (m_tasks.contains(t)) return;
    m_tasks.append(t);
    // Agrégation : ne pas prendre ownership, juste référencer
}

void Category::removeTask(Task *t)
{
    if (!t) return;
    m_tasks.removeOne(t);
}
