#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QTranslator>

#include "taskmodel.h"
#include "taskfilterproxymodel.h"
#include "taskdetailwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
    void onSaveFile();
    void onSaveFileAs();
    void onOpenFile();
    void onNewFile();
    void onSearchTextChanged(const QString &text);
    void onAbout();
    void updateStatusBar();

public:
    void setLanguage(const QString &lang);

protected:
    void changeEvent(QEvent *event) override;

private:
    Ui::MainWindow *ui;

    TaskModel *m_taskModel;
    TaskFilterProxyModel *m_proxyModel;
    TaskDetailWidget *m_detailWidget;

    QTranslator m_translator;
    QString m_currentFilePath;
};

#endif // MAINWINDOW_H
