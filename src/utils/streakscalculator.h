#ifndef STREAKSCALCULATOR_H
#define STREAKSCALCULATOR_H

#include <QDate>
#include <QList>

class Task;

/**
 * @brief Utilitaire pour calculer les séries (streaks) de jours consécutifs avec tâches complétées
 */
class StreaksCalculator
{
public:
    struct StreakInfo {
        int currentStreak;      ///< Nombre de jours consécutifs actuels
        int longestStreak;      ///< Record de jours consécutifs
        QDate currentStreakStart; ///< Date de début du streak actuel
        QDate longestStreakStart; ///< Date de début du record
        QDate longestStreakEnd;   ///< Date de fin du record
    };
    
    /**
     * @brief Calcule les informations de streaks à partir d'une liste de tâches
     * @param tasks Liste de toutes les tâches
     * @return Structure avec les informations de streaks
     */
    static StreakInfo calculateStreaks(const QList<Task*> &tasks);
    
private:
    /**
     * @brief Extrait et trie les dates de complétion uniques
     * @param tasks Liste de tâches
     * @return Dates de complétion triées (ordre croissant)
     */
    static QList<QDate> extractCompletionDates(const QList<Task*> &tasks);
    
    /**
     * @brief Calcule le streak actuel et le record à partir de dates triées
     * @param sortedDates Dates de complétion triées
     * @return Structure StreakInfo
     */
    static StreakInfo analyzeStreaks(const QList<QDate> &sortedDates);
};

#endif // STREAKSCALCULATOR_H
