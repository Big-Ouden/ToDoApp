#include "streakscalculator.h"
#include "task.h"
#include <QSet>
#include <algorithm>

/**
 * @brief Calcule les séries de complétion de tâches
 * @param tasks Liste des tâches à analyser
 * @return Informations sur la série actuelle et la plus longue
 */
StreaksCalculator::StreakInfo StreaksCalculator::calculateStreaks(const QList<Task*> &tasks)
{
    QList<QDate> completionDates = extractCompletionDates(tasks);
    return analyzeStreaks(completionDates);
}

/**
 * @brief Extrait les dates de complétion uniques des tâches
 * @param tasks Liste des tâches
 * @return Liste triée des dates de complétion
 * @note Inclut récursivement les sous-tâches
 */
QList<QDate> StreaksCalculator::extractCompletionDates(const QList<Task*> &tasks)
{
    QSet<QDate> uniqueDates;
    
    // Récupérer toutes les dates de complétion uniques
    for (Task *task : tasks) {
        if (task->isCompleted() && task->completionDate().isValid()) {
            uniqueDates.insert(task->completionDate());
        }
        
        // Inclure les sous-tâches récursivement
        for (Task *subtask : task->subtasks()) {
            if (subtask->isCompleted() && subtask->completionDate().isValid()) {
                uniqueDates.insert(subtask->completionDate());
            }
        }
    }
    
    // Convertir en liste et trier
    QList<QDate> sortedDates = uniqueDates.values();
    std::sort(sortedDates.begin(), sortedDates.end());
    
    return sortedDates;
}

/**
 * @brief Analyse les dates pour détecter les séries consécutives
 * @param sortedDates Liste triée de dates
 * @return Statistiques de séries (actuelle et record)
 */
StreaksCalculator::StreakInfo StreaksCalculator::analyzeStreaks(const QList<QDate> &sortedDates)
{
    StreakInfo info;
    info.currentStreak = 0;
    info.longestStreak = 0;
    
    if (sortedDates.isEmpty()) {
        return info;
    }
    
    QDate today = QDate::currentDate();
    int tempStreak = 1;
    QDate tempStart = sortedDates.first();
    QDate tempEnd = sortedDates.first();
    
    // Calculer les streaks
    for (int i = 1; i < sortedDates.size(); ++i) {
        QDate prevDate = sortedDates[i - 1];
        QDate currDate = sortedDates[i];
        
        // Vérifier si les dates sont consécutives
        if (prevDate.daysTo(currDate) == 1) {
            tempStreak++;
            tempEnd = currDate;
        } else {
            // Fin du streak, vérifier si c'est un record
            if (tempStreak > info.longestStreak) {
                info.longestStreak = tempStreak;
                info.longestStreakStart = tempStart;
                info.longestStreakEnd = tempEnd;
            }
            
            // Recommencer un nouveau streak
            tempStreak = 1;
            tempStart = currDate;
            tempEnd = currDate;
        }
    }
    
    // Vérifier le dernier streak
    if (tempStreak > info.longestStreak) {
        info.longestStreak = tempStreak;
        info.longestStreakStart = tempStart;
        info.longestStreakEnd = tempEnd;
    }
    
    // Calculer le streak actuel
    // Le streak actuel doit inclure aujourd'hui ou hier
    QDate lastDate = sortedDates.last();
    int daysFromLast = lastDate.daysTo(today);
    
    if (daysFromLast == 0 || daysFromLast == 1) {
        // Le streak est actif
        info.currentStreak = 1;
        info.currentStreakStart = lastDate;
        
        // Compter en arrière les jours consécutifs
        for (int i = sortedDates.size() - 2; i >= 0; --i) {
            if (sortedDates[i].daysTo(sortedDates[i + 1]) == 1) {
                info.currentStreak++;
                info.currentStreakStart = sortedDates[i];
            } else {
                break;
            }
        }
    } else {
        // Le streak est cassé
        info.currentStreak = 0;
    }
    
    return info;
}
