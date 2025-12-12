#include "burndownwidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDateTime>

BurndownWidget::BurndownWidget(TaskModel *model, QWidget *parent)
    : QWidget(parent),
      m_model(model)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Titre
    QLabel *titleLabel = new QLabel(tr("Graphique d'avancement"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Configuration du graphique
    setupChart();
    mainLayout->addWidget(m_chartView);
    
    // Légende explicative
    QLabel *legendLabel = new QLabel(
        tr("🔵 Total des tâches | 🟢 Tâches terminées | 🔴 Tâches restantes"),
        this
    );
    legendLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(legendLabel);
    
    // Connexions pour mise à jour automatique
    connect(m_model, &TaskModel::taskAdded, this, &BurndownWidget::updateChart);
    connect(m_model, &TaskModel::taskRemoved, this, &BurndownWidget::updateChart);
    connect(m_model, &TaskModel::taskUpdated, this, &BurndownWidget::updateChart);
    
    // Initialiser les données
    updateChart();
}

void BurndownWidget::setupChart()
{
    // Créer les séries
    m_totalSeries = new QLineSeries(this);
    m_totalSeries->setName(tr("Total"));
    m_totalSeries->setColor(QColor("#2196F3")); // Bleu
    
    m_completedSeries = new QLineSeries(this);
    m_completedSeries->setName(tr("Terminées"));
    m_completedSeries->setColor(QColor("#4CAF50")); // Vert
    
    m_remainingSeries = new QLineSeries(this);
    m_remainingSeries->setName(tr("Restantes"));
    m_remainingSeries->setColor(QColor("#F44336")); // Rouge
    
    // Créer le graphique
    m_chart = new QChart();
    m_chart->addSeries(m_totalSeries);
    m_chart->addSeries(m_completedSeries);
    m_chart->addSeries(m_remainingSeries);
    
    // Axes
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("dd/MM");
    axisX->setTitleText(tr("Date"));
    m_chart->addAxis(axisX, Qt::AlignBottom);
    
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText(tr("Nombre de tâches"));
    axisY->setLabelFormat("%i");
    axisY->setMinorTickCount(0);
    m_chart->addAxis(axisY, Qt::AlignLeft);
    
    m_totalSeries->attachAxis(axisX);
    m_totalSeries->attachAxis(axisY);
    m_completedSeries->attachAxis(axisX);
    m_completedSeries->attachAxis(axisY);
    m_remainingSeries->attachAxis(axisX);
    m_remainingSeries->attachAxis(axisY);
    
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    
    // Créer la vue
    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(300);
}

int BurndownWidget::countAllTasks(const QList<Task*> &tasks)
{
    int count = 0;
    for (Task *task : tasks) {
        count++;
        if (!task->subtasks().isEmpty()) {
            count += countAllTasks(task->subtasks());
        }
    }
    return count;
}

int BurndownWidget::countCompletedTasks(const QList<Task*> &tasks)
{
    int count = 0;
    for (Task *task : tasks) {
        if (task->status() == Status::COMPLETED) {
            count++;
        }
        if (!task->subtasks().isEmpty()) {
            count += countCompletedTasks(task->subtasks());
        }
    }
    return count;
}

void BurndownWidget::updateChart()
{
    // Compter les tâches actuelles
    int totalTasks = countAllTasks(m_model->rootTasks());
    int completedTasks = countCompletedTasks(m_model->rootTasks());
    int remainingTasks = totalTasks - completedTasks;
    
    // Effacer les séries existantes
    m_totalSeries->clear();
    m_completedSeries->clear();
    m_remainingSeries->clear();
    
    if (totalTasks == 0) {
        m_chart->setTitle(tr("Aucune tâche"));
        return;
    }
    
    // Générer des points de données pour les 7 derniers jours
    QDateTime now = QDateTime::currentDateTime();
    
    for (int i = 6; i >= 0; --i) {
        QDateTime pointDate = now.addDays(-i);
        qint64 timestamp = pointDate.toMSecsSinceEpoch();
        
        // Pour une simulation simple, on suppose que les tâches progressent linéairement
        // Dans une vraie application, on chargerait l'historique depuis une base de données
        double progress = (7 - i) / 7.0;
        int completedAtPoint = qMin(static_cast<int>(completedTasks * progress), completedTasks);
        int remainingAtPoint = totalTasks - completedAtPoint;
        
        m_totalSeries->append(timestamp, totalTasks);
        m_completedSeries->append(timestamp, completedAtPoint);
        m_remainingSeries->append(timestamp, remainingAtPoint);
    }
    
    // Ajouter le point actuel
    qint64 nowTimestamp = now.toMSecsSinceEpoch();
    m_totalSeries->append(nowTimestamp, totalTasks);
    m_completedSeries->append(nowTimestamp, completedTasks);
    m_remainingSeries->append(nowTimestamp, remainingTasks);
    
    // Calculer le taux de complétion
    double completionRate = totalTasks > 0 ? (completedTasks * 100.0 / totalTasks) : 0;
    m_chart->setTitle(tr("Avancement: %1% (%2/%3 tâches)")
                      .arg(static_cast<int>(completionRate))
                      .arg(completedTasks)
                      .arg(totalTasks));
    
    // Ajuster les axes
    QDateTimeAxis *axisX = qobject_cast<QDateTimeAxis*>(m_chart->axes(Qt::Horizontal).first());
    if (axisX) {
        axisX->setRange(now.addDays(-7), now);
    }
    
    QValueAxis *axisY = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Vertical).first());
    if (axisY) {
        axisY->setRange(0, totalTasks + 2);
    }
}
