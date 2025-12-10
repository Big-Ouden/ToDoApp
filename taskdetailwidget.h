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
    void updateTranslations();  // Méthode publique pour forcer la mise à jour

signals:
    void taskModified(Task* task);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onUserEdited();
    void onApplyClicked();

private:
    void updateComboTranslations();
    
    Ui::TaskDetailWidget *ui;
    Task *m_task;
};


#endif // TASKDETAILWIDGET_H
