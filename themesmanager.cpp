#include "themesmanager.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>

QString ThemesManager::lightTheme()
{
    QFile file(":/themes/themes/light.qss");
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Failed to load light theme:" << file.errorString();
        return "";
    }
    
    QTextStream stream(&file);
    QString stylesheet = stream.readAll();
    file.close();
    
    return stylesheet;
}

QString ThemesManager::darkTheme()
{
    QFile file(":/themes/themes/dark.qss");
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Failed to load dark theme:" << file.errorString();
        return "";
    }
    
    QTextStream stream(&file);
    QString stylesheet = stream.readAll();
    file.close();
    
    return stylesheet;
}

QString ThemesManager::warmTheme()
{
    QFile file(":/themes/themes/warm.qss");
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Impossible de charger le thème warm:" << file.errorString();
        return QString();
    }
    
    QTextStream stream(&file);
    return stream.readAll();
}

void ThemesManager::applyTheme(Theme theme)
{
    QString stylesheet;
    
    switch (theme) {
        case Light:
            stylesheet = lightTheme();
            break;
        case Dark:
            stylesheet = darkTheme();
            break;
        case Warm:
            stylesheet = warmTheme();
            break;
    }
    
    qApp->setStyleSheet(stylesheet);
}
