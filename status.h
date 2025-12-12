#ifndef STATUS_H
#define STATUS_H

#include <QString>
#include <QCoreApplication>

/// États possibles d'une tâche
enum class Status
{
    NOTSTARTED,
    INPROGRESS,
    COMPLETED,
    CANCELLED
};

inline QString statusToString(Status s) {
    switch(s) {
    case Status::NOTSTARTED: return QCoreApplication::translate("MainWindow", "Not started");
    case Status::INPROGRESS: return QCoreApplication::translate("MainWindow", "In progress");
    case Status::COMPLETED: return QCoreApplication::translate("MainWindow", "Completed");
    case Status::CANCELLED: return QCoreApplication::translate("MainWindow", "Cancelled");
    }
    return QCoreApplication::translate("MainWindow", "Unknown");
}

inline Status stringToStatus(const QString &s) {
    if (s == "Not started") return Status::NOTSTARTED;
    if (s == "In progress") return Status::INPROGRESS;
    if (s == "Completed") return Status::COMPLETED;
    if (s == "Cancelled") return Status::CANCELLED;
    return Status::NOTSTARTED; // default
}



#endif // STATUS_H
