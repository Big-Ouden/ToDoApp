#ifndef CATEGORY_H
#define CATEGORY_H

#pragma once

#include <QObject>
#include <QString>
#include <QColor>
#include <QList>
#include <QUuid>

class Task;

/**
 * @brief Catégorie pour regrouper des tâches.
 */
class Category : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    /**
     * @brief Constructeur d'une catégorie.
     * @param name Nom de la catégorie
     * @param parent Objet parent Qt
     */
    explicit Category(const QString &name = "", QObject *parent = nullptr);

    /** @return Nom de la catégorie */
    const QString &name() const { return m_name; }
    
    /** @return Couleur de la catégorie */
    const QColor &color() const { return m_color; }
    
    /** @return Liste des tâches de cette catégorie */
    const QList<Task*>& tasks() const { return m_tasks; }

    /**
     * @brief Définit le nom de la catégorie.
     * @param n Nouveau nom
     */
    void setName(const QString &n);
    
    /**
     * @brief Définit la couleur de la catégorie.
     * @param c Nouvelle couleur
     */
    void setColor(const QColor &c);

    /**
     * @brief Ajoute une tâche à la catégorie.
     * @param t Tâche à ajouter
     */
    void addTask(Task *t);
    
    /**
     * @brief Retire une tâche de la catégorie.
     * @param t Tâche à retirer
     */
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
