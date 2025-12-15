#include "chartswidget.h"
#include <QVBoxLayout>
#include <QLabel>

ChartsWidget::ChartsWidget(TaskModel *model, QWidget *parent)
    : QWidget(parent),
      m_model(model)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Titre
    QLabel *titleLabel = new QLabel(tr("Distribution des tâches par statut"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Configuration du graphique de statut
    setupStatusChart();
    mainLayout->addWidget(m_statusChartView);
    
    // Connexions pour mise à jour automatique
    connect(m_model, &TaskModel::taskAdded, this, &ChartsWidget::updateCharts);
    connect(m_model, &TaskModel::taskRemoved, this, &ChartsWidget::updateCharts);
    connect(m_model, &TaskModel::taskUpdated, this, &ChartsWidget::updateCharts);
    
    // Initialiser les données
    updateCharts();
}

void ChartsWidget::setupStatusChart()
{
    // Créer la série de données
    m_statusSeries = new QPieSeries(this);
    
    // Créer le graphique
    m_statusChart = new QChart();
    m_statusChart->addSeries(m_statusSeries);
    m_statusChart->setAnimationOptions(QChart::SeriesAnimations);
    m_statusChart->legend()->setAlignment(Qt::AlignRight);
    
    // Créer la vue du graphique
    m_statusChartView = new QChartView(m_statusChart, this);
    m_statusChartView->setRenderHint(QPainter::Antialiasing);
    m_statusChartView->setMinimumHeight(300);
}

void ChartsWidget::countTasksByStatus(const QList<Task*> &tasks, 
                                      int &notStarted, int &inProgress, 
                                      int &completed, int &cancelled)
{
    for (Task *task : tasks) {
        switch (task->status()) {
        case Status::NOTSTARTED:
            notStarted++;
            break;
        case Status::INPROGRESS:
            inProgress++;
            break;
        case Status::COMPLETED:
            completed++;
            break;
        case Status::CANCELLED:
            cancelled++;
            break;
        }
        
        // Compter récursivement les sous-tâches
        if (!task->subtasks().isEmpty()) {
            countTasksByStatus(task->subtasks(), notStarted, inProgress, completed, cancelled);
        }
    }
}

void ChartsWidget::updateCharts()
{
    // Compter les tâches par statut
    int notStarted = 0;
    int inProgress = 0;
    int completed = 0;
    int cancelled = 0;
    
    countTasksByStatus(m_model->rootTasks(), notStarted, inProgress, completed, cancelled);
    
    // Mettre à jour la série du camembert
    m_statusSeries->clear();
    
    int total = notStarted + inProgress + completed + cancelled;
    if (total == 0) {
        // Pas de données à afficher
        m_statusChart->setTitle(tr("Aucune tâche"));
        return;
    }
    
    // Ajouter les tranches avec pourcentages
    if (notStarted > 0) {
        QPieSlice *slice = m_statusSeries->append(tr("Non démarré (%1%)").arg(notStarted * 100 / total), notStarted);
        slice->setColor(QColor("#9E9E9E")); // Gris
        slice->setLabelVisible(true);
    }
    
    if (inProgress > 0) {
        QPieSlice *slice = m_statusSeries->append(tr("En cours (%1%)").arg(inProgress * 100 / total), inProgress);
        slice->setColor(QColor("#2196F3")); // Bleu
        slice->setLabelVisible(true);
    }
    
    if (completed > 0) {
        QPieSlice *slice = m_statusSeries->append(tr("Terminé (%1%)").arg(completed * 100 / total), completed);
        slice->setColor(QColor("#4CAF50")); // Vert
        slice->setLabelVisible(true);
        // Détacher légèrement la tranche terminée
        slice->setExploded(true);
        slice->setExplodeDistanceFactor(0.05);
    }
    
    if (cancelled > 0) {
        QPieSlice *slice = m_statusSeries->append(tr("Annulé (%1%)").arg(cancelled * 100 / total), cancelled);
        slice->setColor(QColor("#F44336")); // Rouge
        slice->setLabelVisible(true);
    }
    
    m_statusChart->setTitle(tr("Total: %1 tâche(s)").arg(total));
}
