#include "timelinewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextCharFormat>

TimelineWidget::TimelineWidget(TaskModel *model, QWidget *parent)
    : QWidget(parent),
      m_model(model)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Titre
    QLabel *titleLabel = new QLabel(tr("Calendrier des échéances"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Calendrier
    m_calendar = new QCalendarWidget(this);
    m_calendar->setGridVisible(true);
    m_calendar->setMinimumDate(QDate::currentDate().addYears(-1));
    m_calendar->setMaximumDate(QDate::currentDate().addYears(2));
    mainLayout->addWidget(m_calendar);
    
    // Label pour la date sélectionnée
    m_selectedDateLabel = new QLabel(tr("Sélectionnez une date"), this);
    m_selectedDateLabel->setAlignment(Qt::AlignCenter);
    QFont labelFont = m_selectedDateLabel->font();
    labelFont.setPointSize(10);
    labelFont.setBold(true);
    m_selectedDateLabel->setFont(labelFont);
    mainLayout->addWidget(m_selectedDateLabel);
    
    // Liste des tâches pour la date sélectionnée
    m_taskList = new QListWidget(this);
    m_taskList->setMinimumHeight(150);
    mainLayout->addWidget(m_taskList);
    
    // Connexions
    connect(m_calendar, &QCalendarWidget::clicked, this, &TimelineWidget::onDateSelected);
    connect(m_model, &TaskModel::taskAdded, this, &TimelineWidget::updateCalendar);
    connect(m_model, &TaskModel::taskRemoved, this, &TimelineWidget::updateCalendar);
    connect(m_model, &TaskModel::taskUpdated, this, &TimelineWidget::updateCalendar);
    
    // Initialiser
    updateCalendar();
}

void TimelineWidget::collectTasksWithDates(const QList<Task*> &tasks, QMap<QDate, QList<Task*>> &dateMap)
{
    for (Task *task : tasks) {
        if (task->dueDate().isValid()) {
            dateMap[task->dueDate()].append(task);
        }
        
        // Traiter récursivement les sous-tâches
        if (!task->subtasks().isEmpty()) {
            collectTasksWithDates(task->subtasks(), dateMap);
        }
    }
}

QTextCharFormat TimelineWidget::formatForDate(const QDate &date, const QMap<QDate, QList<Task*>> &dateMap)
{
    QTextCharFormat format;
    
    if (!dateMap.contains(date)) {
        return format;
    }
    
    const QList<Task*> &tasks = dateMap[date];
    
    // Déterminer la couleur selon le statut des tâches
    bool hasCompleted = false;
    bool hasInProgress = false;
    bool hasNotStarted = false;
    
    for (Task *task : tasks) {
        switch (task->status()) {
        case Status::COMPLETED:
            hasCompleted = true;
            break;
        case Status::INPROGRESS:
            hasInProgress = true;
            break;
        case Status::NOTSTARTED:
            hasNotStarted = true;
            break;
        default:
            break;
        }
    }
    
    // Choisir la couleur (priorité: en cours > non démarré > terminé)
    if (hasInProgress) {
        format.setBackground(QColor("#BBDEFB")); // Bleu clair
    } else if (hasNotStarted) {
        format.setBackground(QColor("#FFECB3")); // Jaune clair
    } else if (hasCompleted) {
        format.setBackground(QColor("#C8E6C9")); // Vert clair
    }
    
    format.setFontWeight(QFont::Bold);
    
    return format;
}

void TimelineWidget::updateCalendar()
{
    // Collecter toutes les tâches avec leur date
    QMap<QDate, QList<Task*>> dateMap;
    collectTasksWithDates(m_model->rootTasks(), dateMap);
    
    // Réinitialiser le format de toutes les dates
    QTextCharFormat defaultFormat;
    for (int year = m_calendar->minimumDate().year(); year <= m_calendar->maximumDate().year(); ++year) {
        for (int month = 1; month <= 12; ++month) {
            int daysInMonth = QDate(year, month, 1).daysInMonth();
            for (int day = 1; day <= daysInMonth; ++day) {
                QDate date(year, month, day);
                m_calendar->setDateTextFormat(date, defaultFormat);
            }
        }
    }
    
    // Appliquer les formats pour les dates avec des tâches
    for (auto it = dateMap.constBegin(); it != dateMap.constEnd(); ++it) {
        QTextCharFormat format = formatForDate(it.key(), dateMap);
        m_calendar->setDateTextFormat(it.key(), format);
    }
    
    // Mettre à jour la liste si une date est déjà sélectionnée
    if (m_calendar->selectedDate().isValid()) {
        onDateSelected(m_calendar->selectedDate());
    }
}

void TimelineWidget::onDateSelected(const QDate &date)
{
    m_selectedDateLabel->setText(tr("Tâches du %1").arg(date.toString("dd/MM/yyyy")));
    
    // Collecter toutes les tâches
    QMap<QDate, QList<Task*>> dateMap;
    collectTasksWithDates(m_model->rootTasks(), dateMap);
    
    // Afficher les tâches pour cette date
    m_taskList->clear();
    
    if (dateMap.contains(date)) {
        const QList<Task*> &tasks = dateMap[date];
        
        for (Task *task : tasks) {
            QString statusEmoji;
            switch (task->status()) {
            case Status::NOTSTARTED:
                statusEmoji = "⏸️";
                break;
            case Status::INPROGRESS:
                statusEmoji = "▶️";
                break;
            case Status::COMPLETED:
                statusEmoji = "✅";
                break;
            case Status::CANCELLED:
                statusEmoji = "❌";
                break;
            }
            
            QString priorityEmoji;
            switch (task->priority()) {
            case Priority::LOW:
                priorityEmoji = "🟢";
                break;
            case Priority::MEDIUM:
                priorityEmoji = "🟡";
                break;
            case Priority::HIGH:
                priorityEmoji = "🟠";
                break;
            case Priority::CRITICAL:
                priorityEmoji = "🔴";
                break;
            }
            
            QString itemText = QString("%1 %2 %3").arg(statusEmoji, priorityEmoji, task->title());
            m_taskList->addItem(itemText);
        }
    } else {
        m_taskList->addItem(tr("Aucune tâche pour cette date"));
    }
}
