#include "statisticswidget.h"
#include "ui_statisticswidget.h"
#include <functional>

StatisticsWidget::StatisticsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StatisticsWidget)
{
    ui->setupUi(this);
}

StatisticsWidget::~StatisticsWidget()
{
    delete ui;
}

void StatisticsWidget::updateStatistics(TaskModel *model)
{
    if (!model) return;
    
    TaskStats stats = calculateStats(model);
    
    // Vue d'ensemble
    ui->labelTotal->setText(QString::number(stats.total));
    ui->labelCompleted->setText(QString::number(stats.completed));
    ui->labelInProgress->setText(QString::number(stats.inProgress));
    ui->labelNotStarted->setText(QString::number(stats.notStarted));
    ui->labelOverdue->setText(QString::number(stats.overdue));
    
    // Par priorité
    ui->labelCritical->setText(QString::number(stats.priorityCritical));
    ui->labelHigh->setText(QString::number(stats.priorityHigh));
    ui->labelMedium->setText(QString::number(stats.priorityMedium));
    ui->labelLow->setText(QString::number(stats.priorityLow));
    
    // Progression
    int percentage = stats.total > 0 ? (stats.completed * 100 / stats.total) : 0;
    ui->progressBar->setValue(percentage);
    ui->labelProgressText->setText(tr("%1 / %2 tâches complétées").arg(stats.completed).arg(stats.total));
}

StatisticsWidget::TaskStats StatisticsWidget::calculateStats(TaskModel *model)
{
    TaskStats stats;
    
    // Parcourir toutes les tâches récursivement
    for (Task *task : model->rootTasks()) {
        countTask(task, stats);
    }
    
    return stats;
}

void StatisticsWidget::countTask(Task *task, TaskStats &stats)
{
    if (!task) return;
    
    stats.total++;
    
    // Compter par statut
    switch (task->status()) {
        case Status::COMPLETED:
            stats.completed++;
            break;
        case Status::INPROGRESS:
            stats.inProgress++;
            break;
        case Status::NOTSTARTED:
            stats.notStarted++;
            break;
        case Status::CANCELLED:
            stats.cancelled++;
            break;
    }
    
    // Compter les tâches en retard (non complétées avec date dépassée)
    if (task->isOverdue() && task->status() != Status::COMPLETED) {
        stats.overdue++;
    }
    
    // Compter par priorité
    switch (task->priority()) {
        case Priority::LOW:
            stats.priorityLow++;
            break;
        case Priority::MEDIUM:
            stats.priorityMedium++;
            break;
        case Priority::HIGH:
            stats.priorityHigh++;
            break;
        case Priority::CRITICAL:
            stats.priorityCritical++;
            break;
    }
    
    // Compter récursivement les sous-tâches
    for (Task *subtask : task->subtasks()) {
        countTask(subtask, stats);
    }
}
