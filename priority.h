#ifndef PRIORITY_H
#define PRIORITY_H

#include <QString>
#include <QCoreApplication>

/// Niveaux de priorité d'une tâche
enum class Priority
{
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

inline QString priorityToString(Priority p) {
    switch(p) {
    case Priority::LOW: return QCoreApplication::translate("MainWindow", "Low");
    case Priority::MEDIUM: return QCoreApplication::translate("MainWindow", "Medium");
    case Priority::HIGH: return QCoreApplication::translate("MainWindow", "High");
    case Priority::CRITICAL: return QCoreApplication::translate("MainWindow", "Critical");
    }
    return QCoreApplication::translate("MainWindow", "Unknown");
}

inline Priority stringToPriority(const QString &s) {
    if (s == "Low") return Priority::LOW;
    if (s == "Medium") return Priority::MEDIUM;
    if (s == "High") return Priority::HIGH;
    if (s == "Critical") return Priority::CRITICAL;
    return Priority::LOW; // default
}

#endif // PRIORITY_H
