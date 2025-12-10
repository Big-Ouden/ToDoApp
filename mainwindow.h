#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QTranslator>
#include <QSettings>
#include <QActionGroup>

#include "taskmodel.h"
#include "taskfilterproxymodel.h"
#include "taskdetailwidget.h"
#include "pdfexporttemplate.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Fenêtre principale de l'application.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTaskSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onAddTask();
    void onAddSubtask();
    void onDeleteTask();
    void onMarkCompleted();
    void onPromoteTask();
    void onSaveFile();
    void onSaveFileAs();
    void onOpenFile();
    void onNewFile();
    void onSearchTextChanged(const QString &text);
    void onAbout();
    void updateStatusBar();
    void onShowCompletedToggled(bool checked);
    void onPriorityFilterChanged(int index);
    void onStatusFilterChanged(int index);
    void onExportPdf();
    void onPrintTasks();
    
private:
    void loadPreferences();
    void savePreferences();
    void setupConnections();
    QString generateTasksHtml(const PdfExportTemplate &tmpl);

public:
    void setLanguage(const QString &lang);

protected:
    void changeEvent(QEvent *event) override;

private:
    Ui::MainWindow *ui;

    TaskModel *m_taskModel;
    TaskFilterProxyModel *m_proxyModel;
    TaskDetailWidget *m_detailWidget;
    bool m_showCompleted;
    bool m_askDeleteConfirmation;

    QTranslator m_translator;
    QString m_currentFilePath;
    QString m_currentLanguage;
    QActionGroup *m_languageGroup;
};

#endif // MAINWINDOW_H
