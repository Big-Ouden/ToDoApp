#ifndef PRIORITY_H
#define PRIORITY_H

#include <QString>

enum class Priority
{
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

inline QString priorityToString(Priority p) {
    switch(p) {
    case Priority::LOW: return "Low";
    case Priority::MEDIUM: return "Medium";
    case Priority::HIGH: return "High";
    case Priority::CRITICAL: return "Critical";
    }
    return "Unknown";
}

#endif // PRIORITY_H
