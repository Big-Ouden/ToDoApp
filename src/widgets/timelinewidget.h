#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QWidget>
#include <QCalendarWidget>
#include <QListWidget>
#include <QLabel>
#include <QDate>
#include "taskmodel.h"

/**
 * @brief Widget affichant un calendrier des tâches avec leurs dates d'échéance.
 * 
 * Utilise QCalendarWidget pour montrer visuellement les dates importantes
 * et une liste pour afficher les détails des tâches du jour sélectionné.
 */
class TimelineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineWidget(TaskModel *model, QWidget *parent = nullptr);
    
public slots:
    /**
     * @brief Met à jour le calendrier et la liste des tâches.
     */
    void updateCalendar();
    
private slots:
    /**
     * @brief Appelé quand l'utilisateur sélectionne une date dans le calendrier.
     */
    void onDateSelected(const QDate &date);

private:
    TaskModel *m_model;
    QCalendarWidget *m_calendar;
    QListWidget *m_taskList;
    QLabel *m_selectedDateLabel;
    
    /**
     * @brief Collecte récursivement toutes les tâches avec leur date.
     */
    void collectTasksWithDates(const QList<Task*> &tasks, QMap<QDate, QList<Task*>> &dateMap);
    
    /**
     * @brief Retourne le format de date pour une cellule du calendrier.
     */
    QTextCharFormat formatForDate(const QDate &date, const QMap<QDate, QList<Task*>> &dateMap);
};

#endif // TIMELINEWIDGET_H
