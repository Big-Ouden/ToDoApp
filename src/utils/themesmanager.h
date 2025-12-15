#ifndef THEMESMANAGER_H
#define THEMESMANAGER_H

#include <QString>

/**
 * @brief Gestionnaire de thèmes pour l'application.
 */
class ThemesManager
{
public:
    enum Theme {
        Light,
        Dark,
        Warm
    };
    
    /**
     * @brief Retourne le stylesheet pour le thème clair.
     */
    static QString lightTheme();
    
    /**
     * @brief Retourne le stylesheet pour le thème sombre.
     */
    static QString darkTheme();
    
    /**
     * @brief Retourne le stylesheet pour le thème chaud (crème/orange).
     */
    static QString warmTheme();
    
    /**
     * @brief Applique un thème à l'application.
     */
    static void applyTheme(Theme theme);
};

#endif // THEMESMANAGER_H
