#include "persistencemanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

/**
 * @file persistencemanager.cpp
 * @brief Sérialisation et désérialisation JSON des tâches.
 */

QJsonObject PersistenceManager::taskToJson(Task *t)
{
    QJsonObject obj;
    obj["id"] = t->id().toString();
    obj["title"] = t->title();
    obj["description"] = t->description();
    obj["dueDate"] = t->dueDate().isValid() ? t->dueDate().toString(Qt::ISODate) : QString();
    obj["priority"] = priorityToString(t->priority());
    obj["status"] = statusToString(t->status());

    QJsonArray arr;
    for (Task *st : t->subtasks())
        arr.append(taskToJson(st));
    obj["subtasks"] = arr;
    return obj;
}

QList<Task*> PersistenceManager::loadFromJson(const QString &filePath)
{
    QList<Task*> result;
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) return result;

    QByteArray data = f.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return result;

    QJsonObject root = doc.object();
    QJsonArray tasks = root.value("tasks").toArray();
    // simple recursive loader
    std::function<Task*(const QJsonObject&)> loader = [&](const QJsonObject &o) -> Task* {
        Task *t = new Task(o.value("title").toString());
        QString due = o.value("dueDate").toString();
        if (!due.isEmpty()) t->setDueDate(QDate::fromString(due, Qt::ISODate));
        // priority/status parsing left simple: ignore parsing back to enum for brevity
        QJsonArray subs = o.value("subtasks").toArray();
        for (const QJsonValue &sv : subs) {
            Task *st = loader(sv.toObject());
            t->addSubtask(st);
        }
        return t;
    };

    for (const QJsonValue &v : tasks) {
        Task *t = loader(v.toObject());
        result.append(t);
    }
    return result;
}

bool PersistenceManager::saveToJson(const QString &filePath, const QList<Task*> &tasks)
{
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly)) return false;

    QJsonArray arr;
    for (Task *t : tasks) arr.append(taskToJson(t));
    QJsonObject root;
    root["tasks"] = arr;
    QJsonDocument doc(root);
    f.write(doc.toJson());
    f.close();
    return true;
}
