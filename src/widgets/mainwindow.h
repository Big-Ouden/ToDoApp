#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QTranslator>
#include <QSettings>
#include <QActionGroup>
#include <QTimer>
#include <QUndoStack>
#include <QStackedWidget>
#include <QToolBar>
#include <QButtonGroup>

class QComboBox;
class QLineEdit;

#include "taskmodel.h"
#include "taskfilterproxymodel.h"
#include "taskdetailwidget.h"
#include "pdfexporttemplate.h"
#include "themesmanager.h"
#include "statisticswidget.h"
#include "pomodorotimer.h"
#include "chartswidget.h"
#include "timelinewidget.h"
#include "burndownwidget.h"
#include "kanbanview.h"
#include "heatmapwidget.h"
#include "focusmodedialog.h"
#include "repositorymanager.h"
#include "gitprojectwidget.h"
#include <QDockWidget>

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
    // Mode view (Personal ou Git)
    enum ViewMode { PersonalMode, GitMode };
    
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    QUndoStack* undoStack() { return m_undoStack; }

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
    void onAutoSave();
    void onExportSelected();
    void onImportTasks();
    void onExportCsv();
    void onExportMarkdown();
    void onFocusMode();
    
    // Git integration
    void onSwitchToPersonalMode();
    void onSwitchToGitMode();
    void onTaskCreatedFromIssue(GitIssueTask *issue);
    void onGitSyncRequested(GitRepository *repo);
    void switchViewMode(ViewMode mode);
    void saveRepositories();
    void loadRepositories();
    
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
    StatisticsWidget *m_statisticsWidget;
    PomodoroTimer *m_pomodoroWidget;
    ChartsWidget *m_chartsWidget;
    TimelineWidget *m_timelineWidget;
    BurndownWidget *m_burndownWidget;
    KanbanView *m_kanbanView;
    HeatmapWidget *m_heatmapWidget;
    RepositoryManager *m_repositoryManager;
    GitProjectWidget *m_gitProjectWidget;
    bool m_showCompleted;
    bool m_askDeleteConfirmation;
    QTimer *m_autoSaveTimer;
    QUndoStack *m_undoStack;

    QTranslator m_translator;
    QString m_currentFilePath;
    QString m_currentLanguage;
    QActionGroup *m_languageGroup;
    bool m_isDarkMode;
    
    // Widgets ajoutés dynamiquement dans la toolbar
    QLineEdit *m_searchLineEdit;
    QComboBox *m_priorityFilterCombo;
    QComboBox *m_statusFilterCombo;
    
    // Barre verticale de sélection des vues
    QWidget *m_rightPanel;
    QStackedWidget *m_viewStack;
    QToolBar *m_viewToolBar;
    QButtonGroup *m_viewButtonGroup;
    QList<int> m_savedSplitterSizes;  // Pour sauvegarder les tailles du splitter
    
    // Boutons des vues (pour retranslation)
    QToolButton *m_btnDetails;
    QToolButton *m_btnStats;
    QToolButton *m_btnPomodoro;
    QToolButton *m_btnCharts;
    QToolButton *m_btnTimeline;
    QToolButton *m_btnBurndown;
    QToolButton *m_btnKanban;
    QToolButton *m_btnHeatmap;
    QToolButton *m_btnHide;
    
    ViewMode m_viewMode;
    QWidget *m_personalView;  // Vue actuelle (liste de tâches)
    QAction *m_personalModeAction;
    QAction *m_gitModeAction;
    
    void setupRightPanel();
    void showView(int index);
    void retranslateViewButtons();
};

#endif // MAINWINDOW_H
