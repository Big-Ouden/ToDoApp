#ifndef CHARTSWIDGET_H
#define CHARTSWIDGET_H

#include <QWidget>
#include <QtCharts>
#include "taskmodel.h"

/**
 * @brief Widget affichant des graphiques de visualisation des tâches.
 * 
 * Contient un diagramme camembert montrant la distribution des tâches par statut.
 */
class ChartsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartsWidget(TaskModel *model, QWidget *parent = nullptr);
    
public slots:
    /**
     * @brief Met à jour les graphiques avec les données actuelles du modèle.
     */
    void updateCharts();

private:
    TaskModel *m_model;
    
    // Camembert pour les statuts
    QChart *m_statusChart;
    QChartView *m_statusChartView;
    QPieSeries *m_statusSeries;
    
    /**
     * @brief Configure le graphique de statut (camembert).
     */
    void setupStatusChart();
    
    /**
     * @brief Compte récursivement les tâches par statut.
     */
    void countTasksByStatus(const QList<Task*> &tasks, 
                           int &notStarted, int &inProgress, 
                           int &completed, int &cancelled);
};

#endif // CHARTSWIDGET_H
