#ifndef STATUS_H
#define STATUS_H

#include <QString>

enum class Status
{
    NOTSTARTED,
    INPROGRESS,
    COMPLETED,
    CANCELLED
};

inline QString statusToString(Status s) {
    switch(s) {
    case Status::NOTSTARTED: return "Not started";
    case Status::INPROGRESS: return "In progress";
    case Status::COMPLETED: return "Completed";
    case Status::CANCELLED: return "Cancelled";
    }
    return "Unknown";
}

inline Status stringToStatus(const QString &s) {
    if (s == "Not started") return Status::NOTSTARTED;
    if (s == "In progress") return Status::INPROGRESS;
    if (s == "Completed") return Status::COMPLETED;
    if (s == "Cancelled") return Status::CANCELLED;
    return Status::NOTSTARTED; // default
}



#endif // STATUS_H
