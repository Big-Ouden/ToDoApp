#ifndef PERSISTENCEMANAGER_H
#define PERSISTENCEMANAGER_H

#pragma once

#include <QString>
#include <QJsonObject>
#include <QList>

#include "task.h"

class PersistenceManager
{
public:
    static bool saveToJson(const QString &filePath, const QList<Task*> &tasks);
    static QList<Task*> loadFromJson(const QString &filePath);

private:
    static QJsonObject taskToJson(Task *t);
    static Task* jsonToTask(const QJsonObject &obj);
};

#endif // PERSISTENCEMANAGER_H
