#ifndef PERSISTENCEMANAGER_H
#define PERSISTENCEMANAGER_H

#pragma once

#include <QString>
#include <QJsonObject>
#include <QList>

#include "task.h"

/**
 * @brief Gère la sauvegarde et le chargement des tâches en JSON.
 */
class PersistenceManager
{
public:
    /**
     * @brief Sauvegarde une liste de tâches dans un fichier JSON.
     * @param filePath Chemin du fichier de destination
     * @param tasks Liste des tâches à sauvegarder
     * @return true si la sauvegarde a réussi
     */
    static bool saveToJson(const QString &filePath, const QList<Task*> &tasks);
    
    /**
     * @brief Charge une liste de tâches depuis un fichier JSON.
     * @param filePath Chemin du fichier source
     * @return Liste des tâches chargées
     */
    static QList<Task*> loadFromJson(const QString &filePath);

private:
    /**
     * @brief Convertit une tâche en objet JSON.
     * @param t Tâche à convertir
     * @return Objet JSON représentant la tâche
     */
    static QJsonObject taskToJson(Task *t);
    
    /**
     * @brief Convertit un objet JSON en tâche.
     * @param obj Objet JSON à convertir
     * @return Pointeur vers la tâche créée
     */
    static Task* jsonToTask(const QJsonObject &obj);
};

#endif // PERSISTENCEMANAGER_H
