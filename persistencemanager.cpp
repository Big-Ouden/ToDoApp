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
    
    // Tags
    QJsonArray tagsArray;
    for (const QString &tag : t->tags())
        tagsArray.append(tag);
    obj["tags"] = tagsArray;
    
    // Attachments
    QJsonArray attachmentsArray;
    for (const QUrl &url : t->attachments())
        attachmentsArray.append(url.toString());
    obj["attachments"] = attachmentsArray;

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
        t->setDescription(o.value("description").toString());
        QString due = o.value("dueDate").toString();
        if (!due.isEmpty()) t->setDueDate(QDate::fromString(due, Qt::ISODate));
        
        // Charger priority et status
        QString priorityStr = o.value("priority").toString();
        t->setPriority(stringToPriority(priorityStr));
        QString statusStr = o.value("status").toString();
        t->setStatus(stringToStatus(statusStr));
        
        // Charger les tags
        QJsonArray tagsArray = o.value("tags").toArray();
        QStringList tags;
        for (const QJsonValue &tagVal : tagsArray)
            tags.append(tagVal.toString());
        t->setTags(tags);
        
        // Charger les attachments
        QJsonArray attachmentsArray = o.value("attachments").toArray();
        QList<QUrl> attachments;
        for (const QJsonValue &attachVal : attachmentsArray)
            attachments.append(QUrl(attachVal.toString()));
        t->setAttachments(attachments);
        
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

bool PersistenceManager::exportSelectedToJson(const QString &filePath, const QList<Task*> &tasks)
{
    // Réutiliser la même logique que saveToJson
    return saveToJson(filePath, tasks);
}

QList<Task*> PersistenceManager::importFromJson(const QString &filePath)
{
    // Réutiliser la même logique que loadFromJson
    return loadFromJson(filePath);
}

bool PersistenceManager::exportToCsv(const QString &filePath, const QList<Task*> &tasks)
{
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&f);
    
    // En-têtes CSV
    out << "Titre,Description,Date d'échéance,Priorité,Statut,Tags\n";
    
    // Fonction récursive pour exporter les tâches et sous-tâches
    std::function<void(Task*, int)> exportTask = [&](Task *t, int level) {
        QString indent = QString(level * 2, ' ');
        
        // Échapper les virgules et guillemets
        auto escape = [](const QString &str) -> QString {
            QString escaped = str;
            escaped.replace("\"", "\"\"");
            if (escaped.contains(',') || escaped.contains('\n') || escaped.contains('"')) {
                return "\"" + escaped + "\"";
            }
            return escaped;
        };
        
        out << escape(indent + t->title()) << ","
            << escape(t->description()) << ","
            << (t->dueDate().isValid() ? t->dueDate().toString(Qt::ISODate) : "") << ","
            << escape(priorityToString(t->priority())) << ","
            << escape(statusToString(t->status())) << ","
            << escape(t->tags().join("; ")) << "\n";
        
        // Exporter les sous-tâches
        for (Task *st : t->subtasks()) {
            exportTask(st, level + 1);
        }
    };
    
    // Exporter toutes les tâches
    for (Task *task : tasks) {
        exportTask(task, 0);
    }
    
    f.close();
    return true;
}

bool PersistenceManager::exportToMarkdown(const QString &filePath, const QList<Task*> &tasks)
{
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&f);
    
    // En-tête Markdown
    out << "# Liste de tâches\n\n";
    
    // Fonction récursive pour exporter les tâches et sous-tâches
    std::function<void(Task*, int)> exportTask = [&](Task *t, int level) {
        QString indent = QString(level * 2, ' ');
        
        // Checkbox selon le statut
        QString checkbox = (t->status() == Status::COMPLETED) ? "[x]" : "[ ]";
        
        // Titre avec indentation
        out << indent << "- " << checkbox << " **" << t->title() << "**";
        
        // Priorité et statut
        if (t->priority() != Priority::MEDIUM || t->status() != Status::NOTSTARTED) {
            out << " (";
            if (t->priority() != Priority::MEDIUM) {
                out << priorityToString(t->priority());
            }
            if (t->status() != Status::NOTSTARTED) {
                if (t->priority() != Priority::MEDIUM) out << ", ";
                out << statusToString(t->status());
            }
            out << ")";
        }
        out << "\n";
        
        // Description
        if (!t->description().isEmpty()) {
            QStringList descLines = t->description().split('\n');
            for (const QString &line : descLines) {
                out << indent << "  > " << line << "\n";
            }
        }
        
        // Date d'échéance
        if (t->dueDate().isValid()) {
            out << indent << "  📅 Échéance: " << t->dueDate().toString("dd/MM/yyyy") << "\n";
        }
        
        // Tags
        if (!t->tags().isEmpty()) {
            out << indent << "  🏷️ Tags: " << t->tags().join(", ") << "\n";
        }
        
        // Ligne vide après chaque tâche principale
        if (level == 0) {
            out << "\n";
        }
        
        // Exporter les sous-tâches
        for (Task *st : t->subtasks()) {
            exportTask(st, level + 1);
        }
    };
    
    // Exporter toutes les tâches
    for (Task *task : tasks) {
        exportTask(task, 0);
    }
    
    f.close();
    return true;
}
