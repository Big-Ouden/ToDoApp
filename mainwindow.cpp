#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

/**
 * @file mainwindow.cpp
 * @brief Fenêtre principale rassemblant vue, modèle et contrôles.
 */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_taskModel(new TaskModel(this)),
    m_proxyModel(new TaskFilterProxyModel(this)),
    m_detailWidget(new TaskDetailWidget(this))
{
    ui->setupUi(this);

    m_proxyModel->setSourceModel(m_taskModel);
    ui->taskTreeView->setModel(m_proxyModel);

    // Install delegate
    ui->taskTreeView->setItemDelegate(new TaskItemDelegate(this));

    // Place detail widget on the right (assumes splitter with placeholder)
    ui->splitter->replaceWidget(1, m_detailWidget);

    // Connections
    connect(ui->taskTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::onTaskSelectionChanged);

    connect(m_detailWidget, &TaskDetailWidget::taskModified, this, [this]() {
        updateStatusBar();
    });

    // Search
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);

    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTaskSelectionChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    QModelIndex src = m_proxyModel->mapToSource(current);
    Task *t = m_taskModel->getTask(src);
    m_detailWidget->setTask(t);
}

void MainWindow::onAddTask()
{
    Task *t = new Task("Nouvelle tâche");
    m_taskModel->insertTask(t, nullptr);
}

void MainWindow::onAddSubtask()
{
    QModelIndex cur = ui->taskTreeView->currentIndex();
    QModelIndex src = m_proxyModel->mapToSource(cur);
    Task *parent = m_taskModel->getTask(src);
    if (!parent) return;
    Task *t = new Task("Nouvelle sous-tâche");
    m_taskModel->insertTask(t, parent);
}

void MainWindow::onDeleteTask()
{
    QModelIndex cur = ui->taskTreeView->currentIndex();
    if (!cur.isValid()) return;
    QModelIndex src = m_proxyModel->mapToSource(cur);
    QMessageBox::StandardButton btn = QMessageBox::question(this, tr("Supprimer"), tr("Confirmer la suppression ?"));
    if (btn == QMessageBox::Yes) {
        m_taskModel->removeTask(src);
    }
}

void MainWindow::onMarkCompleted()
{
    QModelIndex cur = ui->taskTreeView->currentIndex();
    QModelIndex src = m_proxyModel->mapToSource(cur);
    Task *t = m_taskModel->getTask(src);
    if (!t) return;
    t->setStatus(Status::Completed);
    updateStatusBar();
}

void MainWindow::onSaveFile()
{
    if (m_currentFilePath.isEmpty()) {
        onSaveFileAs();
        return;
    }
    if (PersistenceManager::saveToJson(m_currentFilePath, m_taskModel->rootTasks())) {
        statusBar()->showMessage(tr("Sauvegarde réussie"), 2000);
    } else {
        statusBar()->showMessage(tr("Échec sauvegarde"), 2000);
    }
}

void MainWindow::onSaveFileAs()
{
    QString fname = QFileDialog::getSaveFileName(this, tr("Enregistrer"), QString(), tr("JSON Files (*.json)"));
    if (fname.isEmpty()) return;
    m_currentFilePath = fname;
    onSaveFile();
}

void MainWindow::onOpenFile()
{
    QString fname = QFileDialog::getOpenFileName(this, tr("Ouvrir"), QString(), tr("JSON Files (*.json)"));
    if (fname.isEmpty()) return;
    QList<Task*> loaded = PersistenceManager::loadFromJson(fname);
    m_taskModel->clear();
    for (Task *t : loaded) m_taskModel->insertTask(t, nullptr);
    m_currentFilePath = fname;
    statusBar()->showMessage(tr("Fichier chargé"), 2000);
}

void MainWindow::onNewFile()
{
    m_taskModel->clear();
    m_currentFilePath.clear();
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    m_proxyModel->setSearchText(text);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("À propos"), tr("ToDoApp - Exemple pédagogique"));
}

void MainWindow::updateStatusBar()
{
    int total = 0;
    int done = 0;
    for (Task *t : m_taskModel->rootTasks()) {
        ++total;
        if (t->isCompleted()) ++done;
    }
    statusBar()->showMessage(tr("%1 tâches | %2 complétées").arg(total).arg(done));
}

void MainWindow::setLanguage(const QString &lang)
{
    qApp->removeTranslator(&m_translator);
    if (m_translator.load(":/i18n/app_" + lang + ".qm")) {
        qApp->installTranslator(&m_translator);
    }
    ui->retranslateUi(this);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
    QMainWindow::changeEvent(event);
}
