#ifndef DEBUG_H
#define DEBUG_H

#include <QDebug>
#include <QApplication>

/**
 * @file debug.h
 * @brief Macros pour le debug conditionnel basé sur les options CLI.
 */

/**
 * @brief Macro pour afficher un message de debug seulement si --debug est activé.
 * 
 * Utilisation: DEBUG_LOG() << "Message" << variable;
 */
#define DEBUG_LOG() \
    if (qApp->property("debugEnabled").toBool()) qDebug()

/**
 * @brief Macro pour afficher un message verbeux seulement si --verbose est activé.
 * 
 * Utilisation: VERBOSE_LOG() << "Message détaillé" << variable;
 */
#define VERBOSE_LOG() \
    if (qApp->property("verboseEnabled").toBool()) qDebug()

/**
 * @brief Macro pour afficher un avertissement conditionnel.
 * 
 * Utilisation: DEBUG_WARNING() << "Avertissement" << variable;
 */
#define DEBUG_WARNING() \
    if (qApp->property("debugEnabled").toBool()) qWarning()

#endif // DEBUG_H
