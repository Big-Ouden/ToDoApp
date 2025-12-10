#ifndef TASKDETAILWIDGET_H
#define TASKDETAILWIDGET_H

#pragma once

#include <QWidget>
#include "task.h"

namespace Ui { class TaskDetailWidget; }

class TaskDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskDetailWidget(QWidget *parent = nullptr);
    ~TaskDetailWidget();

    void setTask(Task *task);
    void clearTask();

signals:
    void taskModified();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onUserEdited();

private:
    Ui::TaskDetailWidget *ui;
    Task *m_task;
};


#endif // TASKDETAILWIDGET_H
