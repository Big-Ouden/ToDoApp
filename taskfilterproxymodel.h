#ifndef TASKFILTERPROXYMODEL_H
#define TASKFILTERPROXYMODEL_H

#pragma once

#include <QSortFilterProxyModel>
#include <QString>

#include "priority.h"
#include "status.h"

class TaskFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit TaskFilterProxyModel(QObject *parent = nullptr);

    void setSearchText(const QString &text);
    void setPriorityFilter(Priority p, bool enabled = true);
    void setStatusFilter(Status s, bool enabled = true);
    void setShowCompleted(bool show);

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

private:
    QString m_searchText;
    Priority m_priorityFilter;
    Status m_statusFilter;
    bool m_showCompleted;
    bool m_priorityFilterEnabled;
    bool m_statusFilterEnabled;
};


#endif // TASKFILTERPROXYMODEL_H
