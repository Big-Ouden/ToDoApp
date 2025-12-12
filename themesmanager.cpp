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

void ThemesManager::applyTheme(Theme theme)
{
    QString stylesheet;
    
    if (theme == Light) {
        stylesheet = lightTheme();
    } else {
        stylesheet = darkTheme();
    }
    
    qApp->setStyleSheet(stylesheet);
}
