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



#endif // STATUS_H
