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
    void setPriorityFilter(Priority p);
    void setStatusFilter(Status s);

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

private:
    QString m_searchText;
    Priority m_priorityFilter;
    Status m_statusFilter;
};


#endif // TASKFILTERPROXYMODEL_H
