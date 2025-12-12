#ifndef BURNDOWNWIDGET_H
#define BURNDOWNWIDGET_H

#include <QWidget>
#include <QtCharts>
#include "taskmodel.h"

/**
 * @brief Widget affichant un graphique burndown de l'évolution des tâches.
 * 
 * Montre l'évolution du nombre de tâches restantes au fil du temps,
 * organisées par semaine.
 */
class BurndownWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BurndownWidget(TaskModel *model, QWidget *parent = nullptr);
    
public slots:
    /**
     * @brief Met à jour le graphique avec les données actuelles.
     */
    void updateChart();

private:
    TaskModel *m_model;
    
    QChart *m_chart;
    QChartView *m_chartView;
    QLineSeries *m_totalSeries;
    QLineSeries *m_completedSeries;
    QLineSeries *m_remainingSeries;
    
    /**
     * @brief Configure le graphique burndown.
     */
    void setupChart();
    
    /**
     * @brief Compte récursivement toutes les tâches.
     */
    int countAllTasks(const QList<Task*> &tasks);
    
    /**
     * @brief Compte récursivement les tâches terminées.
     */
    int countCompletedTasks(const QList<Task*> &tasks);
};

#endif // BURNDOWNWIDGET_H
