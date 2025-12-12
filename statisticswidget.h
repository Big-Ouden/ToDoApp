#ifndef STATISTICSWIDGET_H
#define STATISTICSWIDGET_H

#include <QWidget>
#include "taskmodel.h"

namespace Ui {
class StatisticsWidget;
}

/**
 * @brief Widget affichant les statistiques des tâches.
 */
class StatisticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsWidget(QWidget *parent = nullptr);
    ~StatisticsWidget();
    
    /**
     * @brief Met à jour les statistiques à partir du modèle.
     */
    void updateStatistics(TaskModel *model);

private:
    Ui::StatisticsWidget *ui;
    
    struct TaskStats {
        int total = 0;
        int completed = 0;
        int inProgress = 0;
        int notStarted = 0;
        int cancelled = 0;
        int overdue = 0;
        
        int priorityLow = 0;
        int priorityMedium = 0;
        int priorityHigh = 0;
        int priorityCritical = 0;
    };
    
    TaskStats calculateStats(TaskModel *model);
    void countTask(Task *task, TaskStats &stats);
};

#endif // STATISTICSWIDGET_H
