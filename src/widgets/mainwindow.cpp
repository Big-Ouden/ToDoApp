#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "taskitemdelegate.h"
#include "persistencemanager.h"
#include "undocommands.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>
#include <functional>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QPrintDialog>
#include <QPageSize>
#include <QPageLayout>
#include <QInputDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QButtonGroup>
#include <QToolBar>
#include <QScrollArea>
#include <QJsonDocument>
#include <QJsonArray>

/**
 * @file mainwindow.cpp
 * @brief Fenêtre principale rassemblant vue, modèle et contrôles.
 */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_taskModel(new TaskModel(this)),
    m_proxyModel(new TaskFilterProxyModel(this)),
    m_detailWidget(new TaskDetailWidget(this)),
    m_languageGroup(nullptr)
{
    ui->setupUi(this);

    // Configure le modèle et le proxy
    m_proxyModel->setSourceModel(m_taskModel);
    ui->taskTreeView->setModel(m_proxyModel);

    // Install delegate pour l'édition in-place
    ui->taskTreeView->setItemDelegate(new TaskItemDelegate(this));

    // Configure la TreeView
    ui->taskTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->taskTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    ui->taskTreeView->setRootIsDecorated(true); // Afficher les icônes +/- pour les branches
    ui->taskTreeView->expandAll();
    
    // Activer drag & drop
    ui->taskTreeView->setDragEnabled(true);
    ui->taskTreeView->setAcceptDrops(true);
    ui->taskTreeView->setDropIndicatorShown(true);
    ui->taskTreeView->setDragDropMode(QAbstractItemView::InternalMove);
    
    // Ajuster la largeur des colonnes
    ui->taskTreeView->setColumnWidth(0, 350); // Titre - plus large
    ui->taskTreeView->setColumnWidth(1, 130); // Date d'échéance
    ui->taskTreeView->setColumnWidth(2, 110); // Priorité
    ui->taskTreeView->setColumnWidth(3, 110); // Statut
    ui->taskTreeView->setColumnWidth(4, 220); // Étiquettes

    // Créer les widgets pour le panneau de droite
    m_statisticsWidget = new StatisticsWidget(this);
    m_pomodoroWidget = new PomodoroTimer(this);
    m_chartsWidget = new ChartsWidget(m_taskModel, this);
    m_timelineWidget = new TimelineWidget(m_taskModel, this);
    m_burndownWidget = new BurndownWidget(m_taskModel, this);
    m_kanbanView = new KanbanView(m_taskModel, this);
    m_heatmapWidget = new HeatmapWidget(m_taskModel, this);
    
    // Configuration du panneau de droite avec barre verticale
    setupRightPanel();
    
    // Connexions du Pomodoro pour notifications dans la barre de statut
    connect(m_pomodoroWidget, &PomodoroTimer::pomodoroCompleted, this, [this]() {
        statusBar()->showMessage(tr("🍅 Pomodoro terminé ! Temps de faire une pause."), 5000);
    });
    connect(m_pomodoroWidget, &PomodoroTimer::breakCompleted, this, [this]() {
        statusBar()->showMessage(tr("✓ Pause terminée ! Prêt pour un nouveau Pomodoro."), 5000);
    });

    // ========================================
    // Initialisation précoce (avant setupConnections)
    // ========================================
    m_showCompleted = true;
    m_currentLanguage = "fr";
    
    // Configurer la sauvegarde automatique (5 minutes par défaut)
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::onAutoSave);
    
    // Configurer l'undo stack AVANT setupConnections qui en a besoin
    m_undoStack = new QUndoStack(this);
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    connect(m_undoStack, &QUndoStack::canUndoChanged, ui->actionUndo, &QAction::setEnabled);
    connect(m_undoStack, &QUndoStack::canRedoChanged, ui->actionRedo, &QAction::setEnabled);

    // ========================================
    // Ajouter les widgets dans la toolbar
    // ========================================
    ui->mainToolBar->addSeparator();
    
    // Ajouter le champ de recherche dans la toolbar
    QWidget *searchWidget = new QWidget(this);
    QHBoxLayout *searchLayout = new QHBoxLayout(searchWidget);
    searchLayout->setContentsMargins(8, 0, 8, 0);
    searchLayout->setSpacing(8);
    
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setObjectName("searchLineEdit");
    m_searchLineEdit->setPlaceholderText(tr("Rechercher une tâche..."));
    m_searchLineEdit->setClearButtonEnabled(true);
    m_searchLineEdit->setMinimumWidth(150);
    m_searchLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    searchLayout->addWidget(m_searchLineEdit);
    
    ui->mainToolBar->addWidget(searchWidget);
    ui->mainToolBar->addSeparator();
    
    // Ajouter les filtres de priorité et statut
    QLabel *labelPriority = new QLabel(tr("Priorité:"), this);
    labelPriority->setObjectName("labelPriorityFilter");
    ui->mainToolBar->addWidget(labelPriority);
    
    m_priorityFilterCombo = new QComboBox(this);
    m_priorityFilterCombo->setObjectName("priorityFilterCombo");
    m_priorityFilterCombo->addItem(tr("Toutes"));
    m_priorityFilterCombo->addItem("Low");
    m_priorityFilterCombo->addItem("Medium");
    m_priorityFilterCombo->addItem("High");
    m_priorityFilterCombo->addItem("Critical");
    ui->mainToolBar->addWidget(m_priorityFilterCombo);
    
    ui->mainToolBar->addSeparator();
    
    QLabel *labelStatus = new QLabel(tr("Statut:"), this);
    labelStatus->setObjectName("labelStatusFilter");
    ui->mainToolBar->addWidget(labelStatus);
    
    m_statusFilterCombo = new QComboBox(this);
    m_statusFilterCombo->setObjectName("statusFilterCombo");
    m_statusFilterCombo->addItem(tr("Tous"));
    m_statusFilterCombo->addItem("Not started");
    m_statusFilterCombo->addItem("In progress");
    m_statusFilterCombo->addItem("Completed");
    m_statusFilterCombo->addItem("Cancelled");
    ui->mainToolBar->addWidget(m_statusFilterCombo);
    
    // Connecter les widgets de recherche et filtres
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_priorityFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onPriorityFilterChanged);
    connect(m_statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onStatusFilterChanged);

    // ========================================
    // Configuration des connexions
    // ========================================
    setupConnections();
    
    // ========================================
    // Initialiser le gestionnaire de dépôts Git
    // ========================================
    m_repositoryManager = new RepositoryManager(this);
    m_gitProjectWidget = new GitProjectWidget(nullptr);  // Pas de parent pour l'instant
    m_gitProjectWidget->setRepositoryManager(m_repositoryManager);
    m_gitProjectWidget->hide();  // Caché par défaut
    
    connect(m_gitProjectWidget, &GitProjectWidget::taskCreatedFromIssue,
            this, &MainWindow::onTaskCreatedFromIssue);
    connect(m_gitProjectWidget, &GitProjectWidget::syncRequested,
            this, &MainWindow::onGitSyncRequested);
    connect(m_gitProjectWidget, &GitProjectWidget::repositoryAdded,
            this, [this](GitRepository*) { saveRepositories(); });
    connect(m_gitProjectWidget, &GitProjectWidget::repositoryModified,
            this, [this](GitRepository*) { saveRepositories(); });
    connect(m_gitProjectWidget, &GitProjectWidget::repositoryDeleted,
            this, [this](GitRepository*) { saveRepositories(); });
    
    // Sauvegarder la vue personnelle actuelle
    m_personalView = centralWidget();  // Récupérer le widget central actuel
    m_viewMode = PersonalMode;
    
    // Charger les préférences avant de définir la locale
    loadPreferences();
    loadRepositories();
    
    // Appliquer la langue chargée depuis les préférences
    setLanguage(m_currentLanguage);
    
    // Initialiser le mode sombre
    m_isDarkMode = false;
    
    updateStatusBar();
    setWindowTitle(tr("ToDoApp - Nouveau fichier"));
    
    // Définir une taille de fenêtre par défaut plus grande
    resize(1400, 900);
}

MainWindow::~MainWindow()
{
    savePreferences();
    delete ui;
}

void MainWindow::setupConnections()
{
    // ========================================
    // Connexions des signaux - Modèle et vue
    // ========================================
    connect(ui->taskTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::onTaskSelectionChanged);

    // Note: Les boutons de la barre d'outils personnalisée ont été supprimés.
    // Les actions sont maintenant uniquement dans la toolbar principale et le menu.

    // ========================================
    // Connexions des signaux - Actions du menu
    // ========================================
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onNewFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::onSaveFileAs);
    
    // Créer les actions d'export PDF et d'impression
    QAction *exportPdfAction = new QAction(tr("Exporter en PDF..."), this);
    exportPdfAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E));
    connect(exportPdfAction, &QAction::triggered, this, &MainWindow::onExportPdf);
    
    QAction *printAction = new QAction(tr("Imprimer..."), this);
    printAction->setShortcut(QKeySequence::Print);
    connect(printAction, &QAction::triggered, this, &MainWindow::onPrintTasks);
    
    // Connecter les actions d'export/import
    connect(ui->actionExportSelected, &QAction::triggered, this, &MainWindow::onExportSelected);
    connect(ui->actionImportTasks, &QAction::triggered, this, &MainWindow::onImportTasks);
    connect(ui->actionExportCsv, &QAction::triggered, this, &MainWindow::onExportCsv);
    connect(ui->actionExportMarkdown, &QAction::triggered, this, &MainWindow::onExportMarkdown);
    
    // Insérer avant l'action de sortie (Quit)
    QAction *beforeAction = nullptr;
    for (QAction *act : ui->menuFile->actions()) {
        if (act->objectName() == "actionQuit") {
            beforeAction = act;
            break;
        }
    }
    
    if (beforeAction) {
        ui->menuFile->insertSeparator(beforeAction);
        ui->menuFile->insertAction(beforeAction, printAction);
        ui->menuFile->insertAction(beforeAction, exportPdfAction);
    } else {
        ui->menuFile->addSeparator();
        ui->menuFile->addAction(exportPdfAction);
        ui->menuFile->addAction(printAction);
    }
    
    connect(ui->actionAddTask, &QAction::triggered, this, &MainWindow::onAddTask);
    connect(ui->actionAddSubtask, &QAction::triggered, this, &MainWindow::onAddSubtask);
    connect(ui->actionDeleteTask, &QAction::triggered, this, &MainWindow::onDeleteTask);
    connect(ui->actionMarkCompleted, &QAction::triggered, this, &MainWindow::onMarkCompleted);
    connect(ui->actionPromoteTask, &QAction::triggered, this, &MainWindow::onPromoteTask);
    
    connect(ui->actionUndo, &QAction::triggered, m_undoStack, &QUndoStack::undo);
    connect(ui->actionRedo, &QAction::triggered, m_undoStack, &QUndoStack::redo);
    
    connect(ui->actionExpandAll, &QAction::triggered, ui->taskTreeView, &QTreeView::expandAll);
    connect(ui->actionCollapseAll, &QAction::triggered, ui->taskTreeView, &QTreeView::collapseAll);
    
    // Dark mode toggle
    connect(ui->actionDarkMode, &QAction::toggled, this, [this](bool checked) {
        m_isDarkMode = checked;
        ThemesManager::applyTheme(checked ? ThemesManager::Dark : ThemesManager::Light);
        m_taskModel->setDarkMode(checked);
    });
    
    // Action pour th\u00e8me Warm (Ctrl+W)
    QAction *warmThemeAction = new QAction(tr("Th\u00e8me Chaud"), this);
    warmThemeAction->setShortcut(QKeySequence(tr("Ctrl+W")));
    connect(warmThemeAction, &QAction::triggered, this, [this]() {
        ThemesManager::applyTheme(ThemesManager::Warm);
        m_isDarkMode = false;
        ui->actionDarkMode->setChecked(false);
        m_taskModel->setDarkMode(false);
    });
    addAction(warmThemeAction);  // Ajoute au context menu de la fenêtre
    
    // Groupe pour langues mutuellement exclusives
    if (!m_languageGroup) {
        m_languageGroup = new QActionGroup(this);
    }
    m_languageGroup->addAction(ui->actionFrench);
    m_languageGroup->addAction(ui->actionEnglish);
    
    connect(ui->actionFrench, &QAction::triggered, this, [this]() { setLanguage("fr"); });
    connect(ui->actionEnglish, &QAction::triggered, this, [this]() { setLanguage("en"); });
    
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actionFocusMode, &QAction::triggered, this, &MainWindow::onFocusMode);
    
    // Note: Les widgets de recherche et filtres sont maintenant ajoutés directement dans la toolbar
    // et connectés dans le constructeur, pas ici.

    // ========================================
    // Connexions des signaux - TaskDetailWidget
    // ========================================
    connect(m_detailWidget, &TaskDetailWidget::taskModified, this, [this](Task* task) {
        // Trouver l'index de la tâche modifiée
        std::function<QModelIndex(Task*, const QList<Task*>&, const QModelIndex&)> findTaskIndex;
        findTaskIndex = [&](Task* target, const QList<Task*>& tasks, const QModelIndex& parent) -> QModelIndex {
            for (int i = 0; i < tasks.size(); ++i) {
                if (tasks[i] == target) {
                    return m_taskModel->index(i, 0, parent);
                }
                QModelIndex childIdx = m_taskModel->index(i, 0, parent);
                QModelIndex found = findTaskIndex(target, tasks[i]->subtasks(), childIdx);
                if (found.isValid()) return found;
            }
            return QModelIndex();
        };
        
        QModelIndex idx = findTaskIndex(task, m_taskModel->rootTasks(), QModelIndex());
        if (idx.isValid()) {
            // Émettre dataChanged seulement pour cette tâche (toutes les colonnes)
            QModelIndex topLeft = m_taskModel->index(idx.row(), 0, idx.parent());
            QModelIndex bottomRight = m_taskModel->index(idx.row(), m_taskModel->columnCount(QModelIndex()) - 1, idx.parent());
            emit m_taskModel->dataChanged(topLeft, bottomRight);
        }
        updateStatusBar();
    });

    // ========================================
    // Connexions des signaux - TaskModel
    // ========================================
    connect(m_taskModel, &TaskModel::taskAdded, this, [this](Task *task) {
        updateStatusBar();
        statusBar()->showMessage(tr("Tâche ajoutée : %1").arg(task->title()), 2000);
    });

    connect(m_taskModel, &TaskModel::taskRemoved, this, [this]() {
        updateStatusBar();
        statusBar()->showMessage(tr("Tâche supprimée"), 2000);
    });

    connect(m_taskModel, &TaskModel::taskUpdated, this, [this]() {
        updateStatusBar();
    });
    
    // ========================================
    // Connexion pour le filtrage des tâches complétées
    // ========================================
    connect(ui->actionShowCompleted, &QAction::toggled, this, &MainWindow::onShowCompletedToggled);
    
    // ========================================
    // Actions pour le mode Git
    // ========================================
    m_personalModeAction = new QAction(tr("Mode Personnel"), this);
    m_personalModeAction->setCheckable(true);
    m_personalModeAction->setChecked(true);
    m_personalModeAction->setShortcut(QKeySequence(tr("Ctrl+T")));
    connect(m_personalModeAction, &QAction::triggered, this, &MainWindow::onSwitchToPersonalMode);
    
    m_gitModeAction = new QAction(tr("Mode Git/Issues"), this);
    m_gitModeAction->setCheckable(true);
    m_gitModeAction->setShortcut(QKeySequence(tr("Ctrl+G")));
    connect(m_gitModeAction, &QAction::triggered, this, &MainWindow::onSwitchToGitMode);
    
    QActionGroup *modeGroup = new QActionGroup(this);
    modeGroup->addAction(m_personalModeAction);
    modeGroup->addAction(m_gitModeAction);
    
    // Ajouter au menu Vue
    ui->menuView->addSeparator();
    ui->menuView->addAction(m_personalModeAction);
    ui->menuView->addAction(m_gitModeAction);
}

void MainWindow::onTaskSelectionChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    QModelIndex src = m_proxyModel->mapToSource(current);
    Task *t = m_taskModel->getTask(src);
    m_detailWidget->setTask(t);
    updateStatusBar();
}

void MainWindow::onAddTask()
{
    Task *t = new Task(tr("Nouvelle tâche"));
    AddTaskCommand *cmd = new AddTaskCommand(m_taskModel, t, nullptr);
    m_undoStack->push(cmd);
    
    // Sélectionne la nouvelle tâche
    QModelIndex idx = m_taskModel->index(m_taskModel->rootTasks().size() - 1, 0, QModelIndex());
    QModelIndex proxyIdx = m_proxyModel->mapFromSource(idx);
    ui->taskTreeView->setCurrentIndex(proxyIdx);
    ui->taskTreeView->edit(proxyIdx);
}

void MainWindow::onAddSubtask()
{
    QModelIndex cur = ui->taskTreeView->currentIndex();
    if (!cur.isValid()) {
        QMessageBox::information(this, tr("Information"), 
                                tr("Veuillez d'abord sélectionner une tâche parente."));
        return;
    }
    
    QModelIndex src = m_proxyModel->mapToSource(cur);
    Task *parent = m_taskModel->getTask(src);
    if (!parent) return;
    
    Task *t = new Task(tr("Nouvelle sous-tâche"));
    AddTaskCommand *cmd = new AddTaskCommand(m_taskModel, t, parent);
    m_undoStack->push(cmd);
    
    // Développe le parent et sélectionne la nouvelle sous-tâche
    ui->taskTreeView->expand(cur);
}

void MainWindow::onDeleteTask()
{
    // Récupérer tous les index sélectionnés
    QModelIndexList selectedIndexes = ui->taskTreeView->selectionModel()->selectedRows();
    
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, tr("Information"), 
                                tr("Veuillez d'abord sélectionner une tâche à supprimer."));
        return;
    }
    
    // Convertir les index proxy en index source et récupérer les tâches
    QList<Task*> tasksToDelete;
    int totalSubtasks = 0;
    
    for (const QModelIndex &proxyIdx : selectedIndexes) {
        QModelIndex srcIdx = m_proxyModel->mapToSource(proxyIdx);
        Task *t = m_taskModel->getTask(srcIdx);
        if (t) {
            tasksToDelete.append(t);
            totalSubtasks += t->subtasks().size();
        }
    }
    
    if (tasksToDelete.isEmpty()) return;
    
    // Si l'utilisateur a désactivé la confirmation, supprimer directement
    if (!m_askDeleteConfirmation) {
        for (Task *t : tasksToDelete) {
            RemoveTaskCommand *cmd = new RemoveTaskCommand(m_taskModel, t);
            m_undoStack->push(cmd);
        }
        m_detailWidget->setTask(nullptr);
        return;
    }
    
    // Préparer le message de confirmation
    QString message;
    if (tasksToDelete.size() == 1) {
        message = tr("Êtes-vous sûr de vouloir supprimer la tâche \"%1\" ?").arg(tasksToDelete.first()->title());
        if (totalSubtasks > 0) {
            message += tr("\n\nCette tâche contient %1 sous-tâche(s) qui seront également supprimée(s).").arg(totalSubtasks);
        }
    } else {
        message = tr("Êtes-vous sûr de vouloir supprimer les %1 tâches sélectionnées ?").arg(tasksToDelete.size());
        if (totalSubtasks > 0) {
            message += tr("\n\nCes tâches contiennent au total %1 sous-tâche(s) qui seront également supprimée(s).").arg(totalSubtasks);
        }
    }
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Confirmer la suppression"));
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    QCheckBox *dontAskAgain = new QCheckBox(tr("Ne plus demander confirmation"));
    msgBox.setCheckBox(dontAskAgain);
    
    int ret = msgBox.exec();
    
    if (dontAskAgain->isChecked()) {
        m_askDeleteConfirmation = false;
        savePreferences();
    }
    
    if (ret == QMessageBox::Yes) {
        for (Task *t : tasksToDelete) {
            RemoveTaskCommand *cmd = new RemoveTaskCommand(m_taskModel, t);
            m_undoStack->push(cmd);
        }
        m_detailWidget->setTask(nullptr);
    }
}

void MainWindow::onMarkCompleted()
{
    QModelIndex cur = ui->taskTreeView->currentIndex();
    if (!cur.isValid()) {
        QMessageBox::information(this, tr("Information"), 
                                tr("Veuillez d'abord sélectionner une tâche."));
        return;
    }
    
    QModelIndex src = m_proxyModel->mapToSource(cur);
    Task *t = m_taskModel->getTask(src);
    if (!t) return;
    
    // Fonction récursive pour marquer toutes les sous-tâches
    std::function<void(Task*, Status)> markRecursively = [&](Task *task, Status status) {
        task->setStatus(status);
        for (Task *subtask : task->subtasks()) {
            markRecursively(subtask, status);
        }
    };
    
    if (t->status() == Status::COMPLETED) {
        // Si déjà complétée, on repasse en "En cours"
        markRecursively(t, Status::INPROGRESS);
        statusBar()->showMessage(tr("Tâche et sous-tâches marquées comme en cours"), 2000);
    } else {
        markRecursively(t, Status::COMPLETED);
        statusBar()->showMessage(tr("Tâche et sous-tâches marquées comme complétées"), 2000);
    }
    
    updateStatusBar();
}

void MainWindow::onPromoteTask()
{
    QModelIndex cur = ui->taskTreeView->currentIndex();
    if (!cur.isValid()) {
        QMessageBox::information(this, tr("Information"), 
                                tr("Veuillez d'abord sélectionner une sous-tâche à promouvoir."));
        return;
    }
    
    QModelIndex src = m_proxyModel->mapToSource(cur);
    Task *t = m_taskModel->getTask(src);
    
    if (!t || !t->parentTask()) {
        QMessageBox::information(this, tr("Information"), 
                                tr("Cette tâche est déjà au niveau racine."));
        return;
    }
    
    m_taskModel->promoteTask(src);
    statusBar()->showMessage(tr("Sous-tâche promue au niveau parent"), 2000);
    updateStatusBar();
}

void MainWindow::onSaveFile()
{
    if (m_currentFilePath.isEmpty()) {
        onSaveFileAs();
        return;
    }
    
    if (PersistenceManager::saveToJson(m_currentFilePath, m_taskModel->rootTasks())) {
        saveRepositories();  // Sauvegarder aussi les dépôts Git
        statusBar()->showMessage(tr("Fichier enregistré : %1").arg(m_currentFilePath), 3000);
        setWindowTitle(tr("ToDoApp - %1").arg(QFileInfo(m_currentFilePath).fileName()));
    } else {
        QMessageBox::critical(this, tr("Erreur"), 
                            tr("Impossible d'enregistrer le fichier."));
        statusBar()->showMessage(tr("Échec de la sauvegarde"), 3000);
    }
}

void MainWindow::onAutoSave()
{
    // Sauvegarde automatique silencieuse (pas de message si vide)
    if (!m_currentFilePath.isEmpty()) {
        if (PersistenceManager::saveToJson(m_currentFilePath, m_taskModel->rootTasks())) {
            saveRepositories();  // Sauvegarder aussi les dépôts Git
            statusBar()->showMessage(tr("Sauvegarde automatique effectuée"), 2000);
        }
    }
}

void MainWindow::onSaveFileAs()
{
    QString fname = QFileDialog::getSaveFileName(
        this, 
        tr("Enregistrer sous"), 
        QString(), 
        tr("Fichiers JSON (*.json);;Tous les fichiers (*)")
    );
    
    if (fname.isEmpty()) return;
    
    // Ajoute l'extension .json si elle n'est pas présente
    if (!fname.endsWith(".json", Qt::CaseInsensitive)) {
        fname += ".json";
    }
    
    m_currentFilePath = fname;
    onSaveFile();
}

void MainWindow::onOpenFile()
{
    // Demande confirmation si des tâches existent déjà
    if (m_taskModel->rootTasks().size() > 0) {
        QMessageBox::StandardButton btn = QMessageBox::question(
            this,
            tr("Ouvrir un fichier"),
            tr("Les tâches actuelles seront perdues. Voulez-vous continuer ?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if (btn != QMessageBox::Yes) return;
    }
    
    QString fname = QFileDialog::getOpenFileName(
        this, 
        tr("Ouvrir un fichier"), 
        QString(), 
        tr("Fichiers JSON (*.json);;Tous les fichiers (*)")
    );
    
    if (fname.isEmpty()) return;
    
    QList<Task*> loaded = PersistenceManager::loadFromJson(fname);
    
    if (loaded.isEmpty()) {
        QMessageBox::warning(this, tr("Attention"), 
                           tr("Aucune tâche n'a pu être chargée depuis ce fichier."));
        return;
    }
    
    m_taskModel->clear();
    for (Task *const t : loaded) {
        m_taskModel->insertTask(t, nullptr);
    }
    
    m_currentFilePath = fname;
    setWindowTitle(tr("ToDoApp - %1").arg(QFileInfo(fname).fileName()));
    statusBar()->showMessage(tr("Fichier chargé : %1 tâche(s)").arg(loaded.size()), 3000);
    
    ui->taskTreeView->expandAll();
}

void MainWindow::onNewFile()
{
    // Demande confirmation si des tâches existent déjà
    if (m_taskModel->rootTasks().size() > 0) {
        QMessageBox::StandardButton btn = QMessageBox::question(
            this,
            tr("Nouveau fichier"),
            tr("Les tâches actuelles seront perdues. Voulez-vous continuer ?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if (btn != QMessageBox::Yes) return;
    }
    
    m_taskModel->clear();
    m_currentFilePath.clear();
    m_detailWidget->setTask(nullptr);
    setWindowTitle(tr("ToDoApp - Nouveau fichier"));
    statusBar()->showMessage(tr("Nouveau fichier créé"), 2000);
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    m_proxyModel->setSearchText(text);
}

void MainWindow::onAbout()
{
    QString aboutText = tr(
        "<h2>ToDoApp v1.0</h2>"
        "<p><b>Application de gestion de tâches hiérarchiques</b></p>"
        "<hr>"
        "<p><b>Auteurs :</b></p>"
        "<ul>"
        "<li>Simon Bélier</li>"
        "<li>Wissal Jalfa</li>"
        "</ul>"
        "<p><b>Fonctionnalités :</b></p>"
        "<ul>"
        "<li>Gestion de tâches et sous-tâches avec hiérarchie</li>"
        "<li>Priorités, statuts et étiquettes</li>"
        "<li>Dates d'échéance et suivi de progression</li>"
        "<li>Descriptions au format Markdown</li>"
        "<li>Pièces jointes</li>"
        "<li>Export PDF avec templates</li>"
        "<li>Recherche avancée (tag:, priority:, status:, date:)</li>"
        "<li>Sauvegarde/chargement JSON</li>"
        "<li>Support multilingue (français/anglais)</li>"
        "</ul>"
        "<p><b>Raccourcis clavier :</b></p>"
        "<ul>"
        "<li>Ctrl+N : Nouvelle tâche</li>"
        "<li>Ctrl+Shift+N : Nouvelle sous-tâche</li>"
        "<li>Del : Supprimer</li>"
        "<li>Ctrl+D : Marquer terminée</li>"
        "<li>Ctrl+P : Promouvoir au niveau parent</li>"
        "<li>Ctrl+S : Enregistrer</li>"
        "<li>Ctrl+O : Ouvrir</li>"
        "</ul>"
        "<hr>"
        "<p><b>Technologies :</b></p>"
        "<ul>"
        "<li>Framework : Qt %1</li>"
        "<li>Langage : C++17</li>"
        "<li>Build : CMake</li>"
        "</ul>"
        "<hr>"
        "<p><b>Licence :</b> GNU General Public License v2.0 (GPLv2)</p>"
        "<p style='font-size:9px;'>Ce programme est un logiciel libre ; vous pouvez le redistribuer "
        "et/ou le modifier selon les termes de la GNU General Public License version 2.</p>"
        "<p style='font-size:9px;'>Copyright © 2025. Projet académique.</p>"
    ).arg(QT_VERSION_STR);
    
    QMessageBox::about(this, tr("À propos de ToDoApp"), aboutText);
}

void MainWindow::updateStatusBar()
{
    // Mettre à jour le widget de statistiques
    if (m_statisticsWidget) {
        m_statisticsWidget->updateStatistics(m_taskModel);
    }
    
    int total = 0;
    int done = 0;
    int overdue = 0;
    
    // Fonction récursive pour compter toutes les tâches
    std::function<void(Task*)> countTasks = [&](Task *t) {
        ++total;
        if (t->isCompleted()) ++done;
        if (t->isOverdue() && !t->isCompleted()) ++overdue;
        for (Task *sub : t->subtasks()) {
            countTasks(sub);
        }
    };
    
    for (Task *t : m_taskModel->rootTasks()) {
        countTasks(t);
    }
    
    int pending = total - done;
    
    // Partie permanente : statistiques globales
    QString permanentStatus = tr("Total: %1 tâche(s) | Complétées: %2 | En cours: %3")
                        .arg(total)
                        .arg(done)
                        .arg(pending);
    
    if (overdue > 0) {
        permanentStatus += tr(" | ⚠ En retard: %1").arg(overdue);
    }
    
    // Partie dynamique : informations de la tâche sélectionnée
    QModelIndex current = ui->taskTreeView->currentIndex();
    if (current.isValid()) {
        QModelIndex src = m_proxyModel->mapToSource(current);
        Task *selectedTask = m_taskModel->getTask(src);
        if (selectedTask) {
            QString taskInfo = tr(" | Sélectionnée: \"%1\" - Priorité: %2 - Statut: %3")
                .arg(selectedTask->title())
                .arg(priorityToString(selectedTask->priority()))
                .arg(statusToString(selectedTask->status()));
            
            if (selectedTask->dueDate().isValid()) {
                taskInfo += tr(" - Échéance: %1").arg(selectedTask->dueDate().toString("dd/MM/yyyy"));
            }
            
            permanentStatus += taskInfo;
        }
    }
    
    statusBar()->showMessage(permanentStatus);
}

void MainWindow::onShowCompletedToggled(bool checked)
{
    m_showCompleted = checked;
    m_proxyModel->setShowCompleted(checked);
}

void MainWindow::onPriorityFilterChanged(int index)
{
    if (index == 0) {
        // "Toutes" sélectionné - désactiver le filtre
        m_proxyModel->setPriorityFilter(Priority::LOW, false);
    } else {
        // Appliquer le filtre (index - 1 car "Toutes" est en position 0)
        m_proxyModel->setPriorityFilter(static_cast<Priority>(index - 1), true);
    }
}

void MainWindow::onStatusFilterChanged(int index)
{
    if (index == 0) {
        // "Tous" sélectionné - désactiver le filtre
        m_proxyModel->setStatusFilter(Status::NOTSTARTED, false);
    } else {
        // Appliquer le filtre (index - 1 car "Tous" est en position 0)
        m_proxyModel->setStatusFilter(static_cast<Status>(index - 1), true);
    }
}

void MainWindow::setLanguage(const QString &lang)
{
    m_currentLanguage = lang;
    
    // Retirer complètement l'ancien traducteur
    qApp->removeTranslator(&m_translator);
    
    // Mapper le code de langue au nom de fichier .qm
    QString qmLang = lang;
    if (lang == "en") {
        qmLang = "en_US";
    }
    
    // Créer un nouveau traducteur et charger depuis les ressources Qt
    QString qmFile = QString(":/i18n/ToDoApp_%1.qm").arg(qmLang);
    if (m_translator.load(qmFile)) {
        qApp->installTranslator(&m_translator);
        qDebug() << "Traduction chargée:" << qmFile;
    } else {
        qWarning() << "Impossible de charger la traduction:" << qmFile;
    }
    
    // IMPORTANT: Installer un traducteur vide si français pour forcer le rechargement
    if (lang == "fr") {
        // Le français est la langue source, donc pas besoin de traducteur
        // mais on force quand même le rechargement
        qApp->removeTranslator(&m_translator);
        // Ne rien installer, laisser Qt utiliser les sources
    }
    
    // Définir la locale appropriée
    QLocale locale;
    if (lang == "fr") {
        locale = QLocale(QLocale::French, QLocale::France);
    } else if (lang == "en") {
        locale = QLocale(QLocale::English, QLocale::UnitedStates);
    } else {
        locale = QLocale::system();
    }
    QLocale::setDefault(locale);
    
    // Recharger l'interface avec les nouvelles traductions
    ui->retranslateUi(this);
    
    // Forcer la mise à jour de TOUS les textes des widgets
    // en fermant et rouvrant les combos pour recharger leurs items
    QStringList priorityItems;
    priorityItems << tr("Toutes") << "Low" << "Medium" << "High" << "Critical";
    int currentPriority = m_priorityFilterCombo->currentIndex();
    m_priorityFilterCombo->clear();
    m_priorityFilterCombo->addItems(priorityItems);
    m_priorityFilterCombo->setCurrentIndex(currentPriority);
    
    QStringList statusItems;
    statusItems << tr("Tous") << "Not started" << "In progress" 
                << "Completed" << "Cancelled";
    int currentStatus = m_statusFilterCombo->currentIndex();
    m_statusFilterCombo->clear();
    m_statusFilterCombo->addItems(statusItems);
    m_statusFilterCombo->setCurrentIndex(currentStatus);
    
    // Forcer la mise à jour du TaskDetailWidget directement
    m_detailWidget->updateTranslations();
    
    // Envoyer un événement LanguageChange pour forcer la retraduction de tous les widgets
    QEvent languageChangeEvent(QEvent::LanguageChange);
    QApplication::sendEvent(m_taskModel, &languageChangeEvent);
    QApplication::sendEvent(m_detailWidget, &languageChangeEvent);
    QApplication::sendEvent(m_proxyModel, &languageChangeEvent);
    QApplication::sendEvent(ui->taskTreeView, &languageChangeEvent);
    
    // Traiter les événements en attente
    QApplication::processEvents();
    
    // Forcer la mise à jour des en-têtes du modèle
    emit m_taskModel->headerDataChanged(Qt::Horizontal, 0, m_taskModel->columnCount(QModelIndex()) - 1);
    
    // Forcer la mise à jour de toutes les cellules du tableau pour retraduire les priorités et statuts
    // (incluant les sous-tâches de manière récursive)
    m_taskModel->refreshAllData();
    
    // Forcer le rafraîchissement du header de la vue
    ui->taskTreeView->header()->reset();
    ui->taskTreeView->header()->update();
    
    // Forcer le rafraîchissement complet de la vue
    m_proxyModel->invalidate();
    ui->taskTreeView->update();
    ui->taskTreeView->viewport()->update();
    
    // Forcer la mise à jour du widget de détail avec la nouvelle locale
    QModelIndex current = ui->taskTreeView->currentIndex();
    if (current.isValid()) {
        QModelIndex src = m_proxyModel->mapToSource(current);
        Task *currentTask = m_taskModel->getTask(src);
        if (currentTask) {
            m_detailWidget->setTask(nullptr);  // Reset
            m_detailWidget->setTask(currentTask);  // Reload avec nouvelle locale
        }
    }
    
    // Mettre à jour la barre de statut
    updateStatusBar();
    
    // Sauvegarder la préférence
    savePreferences();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateViewButtons();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::loadPreferences()
{
    QSettings settings("ToDoApp", "ToDoApp");
    
    // Langue
    m_currentLanguage = settings.value("language", "fr").toString();
    
    // Charger la traduction au démarrage et cocher l'action appropriée
    if (m_currentLanguage == "en") {
        ui->actionEnglish->setChecked(true);
    } else {
        ui->actionFrench->setChecked(true);
    }
    setLanguage(m_currentLanguage);
    
    // Géométrie de la fenêtre
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
    }
    
    // État de la fenêtre (maximisée, etc.)
    if (settings.contains("windowState")) {
        restoreState(settings.value("windowState").toByteArray());
    }
    
    // Dernier fichier ouvert
    m_currentFilePath = settings.value("lastFile", "").toString();
    
    // Affichage des tâches complétées
    bool showCompleted = settings.value("showCompleted", true).toBool();
    ui->actionShowCompleted->setChecked(showCompleted);
    m_showCompleted = showCompleted;
    
    // Confirmation de suppression
    m_askDeleteConfirmation = settings.value("askDeleteConfirmation", true).toBool();
    
    // Thème sombre
    bool darkMode = settings.value("darkMode", false).toBool();
    ui->actionDarkMode->setChecked(darkMode);
    m_isDarkMode = darkMode;
    ThemesManager::applyTheme(darkMode ? ThemesManager::Dark : ThemesManager::Light);
    m_taskModel->setDarkMode(darkMode);
    
    // Sauvegarde automatique (intervalle en minutes, 5 par défaut, 0 = désactivé)
    int autoSaveInterval = settings.value("autoSaveInterval", 5).toInt();
    if (autoSaveInterval > 0) {
        m_autoSaveTimer->start(autoSaveInterval * 60 * 1000); // Convertir minutes en millisecondes
    }
    
    // Charger le dernier fichier si disponible
    if (!m_currentFilePath.isEmpty() && QFile::exists(m_currentFilePath)) {
        QList<Task*> tasks = PersistenceManager::loadFromJson(m_currentFilePath);
        if (!tasks.isEmpty()) {
            m_taskModel->clear();
            for (Task *t : tasks) {
                m_taskModel->insertTask(t);
            }
            setWindowTitle(tr("ToDoApp - %1").arg(QFileInfo(m_currentFilePath).fileName()));
        }
    }
}

void MainWindow::savePreferences()
{
    QSettings settings("ToDoApp", "ToDoApp");
    
    settings.setValue("language", m_currentLanguage);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("lastFile", m_currentFilePath);
    settings.setValue("showCompleted", m_showCompleted);
    settings.setValue("askDeleteConfirmation", m_askDeleteConfirmation);
    settings.setValue("darkMode", ui->actionDarkMode->isChecked());
    
    // Sauvegarder l'intervalle d'auto-save (en minutes)
    int intervalMinutes = m_autoSaveTimer->isActive() ? (m_autoSaveTimer->interval() / 60000) : 5;
    settings.setValue("autoSaveInterval", intervalMinutes);
}

// ========================================
// Export PDF et Impression
// ========================================

QString MainWindow::generateTasksHtml(const PdfExportTemplate &tmpl)
{
    QString html = "<html><head><style>";
    html += tmpl.css();
    html += "</style></head><body>";
    
    // En-tête avec substitution des variables
    QString header = tmpl.headerHtml();
    header.replace("{{TITLE}}", tr("Liste des tâches"));
    header.replace("{{DATE}}", tr("Généré le %1").arg(QDate::currentDate().toString("dddd d MMMM yyyy")));
    html += header;
    
    // Fonction récursive pour générer le HTML des tâches
    std::function<void(Task*, int)> generateTaskHtml = [&](Task* task, int level) {
        if (!task) return;
        
        QString priorityClass = QString("priority-%1").arg(priorityToString(task->priority()).toLower().replace(" ", "-"));
        QString statusClass = task->isCompleted() ? " status-completed" : "";
        
        html += QString("<div class='task %1%2' style='margin-left: %3px;'>")
                    .arg(priorityClass)
                    .arg(statusClass)
                    .arg(level * 20);
        
        html += QString("<div class='task-title'>%1</div>").arg(task->title().toHtmlEscaped());
        
        html += "<div class='task-meta'>";
        
        if (task->dueDate().isValid()) {
            html += "<div><span class='meta-label'>" + tr("Échéance:") + "</span> " 
                    + task->dueDate().toString("dd/MM/yyyy") + "</div>";
        }
        
        html += "<div><span class='meta-label'>" + tr("Priorité:") + "</span> " 
                + priorityToString(task->priority()) + "</div>";
        html += "<div><span class='meta-label'>" + tr("Statut:") + "</span> " 
                + statusToString(task->status()) + "</div>";
        
        if (!task->tags().isEmpty()) {
            html += "<div><span class='meta-label'>" + tr("Étiquettes:") + "</span> ";
            for (const QString &tag : task->tags()) {
                html += "<span class='tags'>" + tag.toHtmlEscaped() + "</span> ";
            }
            html += "</div>";
        }
        
        html += "</div>"; // fin task-meta
        
        if (!task->description().isEmpty()) {
            html += "<div class='task-desc'>" + task->description().toHtmlEscaped() + "</div>";
        }
        
        html += "</div>"; // fin task
        
        // Sous-tâches
        for (Task* subtask : task->subtasks()) {
            generateTaskHtml(subtask, level + 1);
        }
    };
    
    // Parcourir toutes les tâches racines
    for (int i = 0; i < m_taskModel->rowCount(QModelIndex()); ++i) {
        QModelIndex index = m_taskModel->index(i, 0, QModelIndex());
        Task* task = m_taskModel->getTask(index);
        generateTaskHtml(task, 0);
    }
    
    // Pied de page avec substitution
    QString footer = tmpl.footerHtml();
    footer.replace("{{DATE}}", QDate::currentDate().toString("dd/MM/yyyy"));
    html += footer;
    
    html += "</body></html>";
    return html;
}

void MainWindow::onExportPdf()
{
    // Choisir le template
    QList<PdfExportTemplate> templates = PdfExportTemplate::availableTemplates();
    QStringList templateNames;
    for (const auto &tmpl : templates) {
        templateNames << tmpl.name();
    }
    
    bool ok;
    QString selectedName = QInputDialog::getItem(
        this,
        tr("Choisir un template"),
        tr("Sélectionnez le style d'export:"),
        templateNames,
        0,
        false,
        &ok
    );
    
    if (!ok) {
        return;
    }
    
    // Trouver le template sélectionné
    PdfExportTemplate selectedTemplate = templates[0];
    for (const auto &tmpl : templates) {
        if (tmpl.name() == selectedName) {
            selectedTemplate = tmpl;
            break;
        }
    }
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Exporter en PDF"),
        QDir::homePath() + "/tasks.pdf",
        tr("Fichiers PDF (*.pdf)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize::A4);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    
    QTextDocument document;
    document.setHtml(generateTasksHtml(selectedTemplate));
    document.print(&printer);
    
    statusBar()->showMessage(tr("PDF exporté avec succès: %1").arg(fileName), 3000);
}

void MainWindow::onPrintTasks()
{
    // Choisir le template
    QList<PdfExportTemplate> templates = PdfExportTemplate::availableTemplates();
    QStringList templateNames;
    for (const auto &tmpl : templates) {
        templateNames << tmpl.name();
    }
    
    bool ok;
    QString selectedName = QInputDialog::getItem(
        this,
        tr("Choisir un template"),
        tr("Sélectionnez le style d'impression:"),
        templateNames,
        0,
        false,
        &ok
    );
    
    if (!ok) {
        return;
    }
    
    // Trouver le template sélectionné
    PdfExportTemplate selectedTemplate = templates[0];
    for (const auto &tmpl : templates) {
        if (tmpl.name() == selectedName) {
            selectedTemplate = tmpl;
            break;
        }
    }
    
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog printDialog(&printer, this);
    printDialog.setWindowTitle(tr("Imprimer les tâches"));
    
    if (printDialog.exec() == QDialog::Accepted) {
        QTextDocument document;
        document.setHtml(generateTasksHtml(selectedTemplate));
        document.print(&printer);
        
        statusBar()->showMessage(tr("Impression effectuée"), 2000);
    }
}

void MainWindow::onExportSelected()
{
    // Récupérer les tâches sélectionnées
    QModelIndexList selectedIndexes = ui->taskTreeView->selectionModel()->selectedRows();
    
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, tr("Export"), 
                                 tr("Veuillez sélectionner au moins une tâche à exporter."));
        return;
    }
    
    // Convertir les index proxy en index source et récupérer les tâches
    QList<Task*> selectedTasks;
    for (const QModelIndex &proxyIndex : selectedIndexes) {
        QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
        if (sourceIndex.isValid()) {
            Task *task = static_cast<Task*>(sourceIndex.internalPointer());
            if (task) {
                selectedTasks.append(task);
            }
        }
    }
    
    // Demander le chemin de destination
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Exporter les tâches sélectionnées"),
        QDir::homePath(),
        tr("Fichiers JSON (*.json)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Exporter
    if (PersistenceManager::exportSelectedToJson(fileName, selectedTasks)) {
        statusBar()->showMessage(tr("Export réussi: %1 tâche(s) exportée(s)").arg(selectedTasks.size()), 3000);
    } else {
        QMessageBox::critical(this, tr("Erreur"), 
                              tr("Impossible d'exporter les tâches vers: %1").arg(fileName));
    }
}

void MainWindow::onImportTasks()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Importer des tâches"),
        QDir::homePath(),
        tr("Fichiers JSON (*.json)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Importer les tâches
    QList<Task*> importedTasks = PersistenceManager::importFromJson(fileName);
    
    if (importedTasks.isEmpty()) {
        QMessageBox::warning(this, tr("Import"), 
                             tr("Aucune tâche n'a pu être importée depuis: %1").arg(fileName));
        return;
    }
    
    // Ajouter les tâches importées au modèle
    for (Task *task : importedTasks) {
        m_taskModel->insertTask(task);
    }
    
    statusBar()->showMessage(tr("Import réussi: %1 tâche(s) importée(s)").arg(importedTasks.size()), 3000);
    updateStatusBar();
}

void MainWindow::onExportCsv()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Exporter en CSV"),
        QDir::homePath(),
        tr("Fichiers CSV (*.csv)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Récupérer toutes les tâches du modèle
    const QList<Task*>& allTasks = m_taskModel->rootTasks();
    
    if (PersistenceManager::exportToCsv(fileName, allTasks)) {
        statusBar()->showMessage(tr("Export CSV réussi: %1").arg(fileName), 3000);
    } else {
        QMessageBox::critical(this, tr("Erreur"), 
                              tr("Impossible d'exporter vers: %1").arg(fileName));
    }
}

void MainWindow::onExportMarkdown()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Exporter en Markdown"),
        QDir::homePath(),
        tr("Fichiers Markdown (*.md)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Récupérer toutes les tâches du modèle
    const QList<Task*>& allTasks = m_taskModel->rootTasks();
    
    if (PersistenceManager::exportToMarkdown(fileName, allTasks)) {
        statusBar()->showMessage(tr("Export Markdown réussi: %1").arg(fileName), 3000);
    } else {
        QMessageBox::critical(this, tr("Erreur"), 
                              tr("Impossible d'exporter vers: %1").arg(fileName));
    }
}

void MainWindow::onFocusMode()
{
    // Obtenir la tâche sélectionnée
    QModelIndex proxyIndex = ui->taskTreeView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::information(this, tr("Mode Focus"), 
                                 tr("Veuillez sélectionner une tâche à afficher en mode Focus."));
        return;
    }
    
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    Task *task = m_taskModel->getTask(sourceIndex);
    
    if (!task) {
        return;
    }
    
    // Ouvrir le dialog Focus Mode
    FocusModeDialog dialog(task, this);
    dialog.exec();
    
    // Rafraîchir l'affichage après fermeture
    m_detailWidget->setTask(task);
}

void MainWindow::setupRightPanel()
{
    // Créer un widget conteneur principal pour la partie droite du splitter
    QWidget *rightContainer = new QWidget(this);
    QHBoxLayout *rightLayout = new QHBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    
    // Créer un QScrollArea pour le panneau de contenu
    QScrollArea *scrollArea = new QScrollArea(rightContainer);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    // Créer le panneau de contenu (qui sera caché/affiché)
    m_rightPanel = new QWidget();
    m_rightPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QVBoxLayout *panelLayout = new QVBoxLayout(m_rightPanel);
    panelLayout->setContentsMargins(5, 5, 5, 5);
    panelLayout->setSpacing(0);
    
    // Créer le stack de vues
    m_viewStack = new QStackedWidget(m_rightPanel);
    m_viewStack->addWidget(m_detailWidget);        // Index 0
    m_viewStack->addWidget(m_statisticsWidget);    // Index 1
    m_viewStack->addWidget(m_pomodoroWidget);      // Index 2
    m_viewStack->addWidget(m_chartsWidget);        // Index 3
    m_viewStack->addWidget(m_timelineWidget);      // Index 4
    m_viewStack->addWidget(m_burndownWidget);      // Index 5
    m_viewStack->addWidget(m_kanbanView);          // Index 6
    m_viewStack->addWidget(m_heatmapWidget);       // Index 7
    
    panelLayout->addWidget(m_viewStack);
    
    // Définir une largeur minimale pour le contenu
    m_rightPanel->setMinimumWidth(400);
    
    // Ajouter le panneau au scroll area
    scrollArea->setWidget(m_rightPanel);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Créer la barre verticale à droite (reste toujours visible)
    m_viewToolBar = new QToolBar(rightContainer);
    m_viewToolBar->setObjectName("viewToolBar");  // Pour cibler avec le CSS
    m_viewToolBar->setOrientation(Qt::Vertical);
    m_viewToolBar->setMovable(false);
    m_viewToolBar->setFloatable(false);
    m_viewToolBar->setIconSize(QSize(20, 20));
    m_viewToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_viewToolBar->setFixedWidth(80);  // Largeur fixe pour la barre
    
    // Créer le groupe de boutons pour l'exclusivité
    m_viewButtonGroup = new QButtonGroup(this);
    m_viewButtonGroup->setExclusive(true);
    
    // Créer les boutons de vue avec icônes (stockés comme membres pour retranslation)
    m_btnDetails = new QToolButton();
    m_btnDetails->setIcon(QIcon(":/icons/pen.svg"));
    m_btnDetails->setText(tr("Détails"));
    m_btnDetails->setCheckable(true);
    m_btnDetails->setChecked(true);
    m_btnDetails->setToolTip(tr("Afficher les détails de la tâche"));
    m_viewButtonGroup->addButton(m_btnDetails, 0);
    
    m_btnStats = new QToolButton();
    m_btnStats->setIcon(QIcon(":/icons/purcentage.svg"));
    m_btnStats->setText(tr("Stats"));
    m_btnStats->setCheckable(true);
    m_btnStats->setToolTip(tr("Afficher les statistiques"));
    m_viewButtonGroup->addButton(m_btnStats, 1);
    
    m_btnPomodoro = new QToolButton();
    m_btnPomodoro->setIcon(QIcon(":/icons/hourglass.svg"));
    m_btnPomodoro->setText(tr("Timer"));
    m_btnPomodoro->setCheckable(true);
    m_btnPomodoro->setToolTip(tr("Timer Pomodoro"));
    m_viewButtonGroup->addButton(m_btnPomodoro, 2);
    
    m_btnCharts = new QToolButton();
    m_btnCharts->setIcon(QIcon(":/icons/eq-medium-cut.svg"));
    m_btnCharts->setText(tr("Charts"));
    m_btnCharts->setCheckable(true);
    m_btnCharts->setToolTip(tr("Graphiques"));
    m_viewButtonGroup->addButton(m_btnCharts, 3);
    
    m_btnTimeline = new QToolButton();
    m_btnTimeline->setIcon(QIcon(":/icons/calendar.svg"));
    m_btnTimeline->setText(tr("Cal."));
    m_btnTimeline->setCheckable(true);
    m_btnTimeline->setToolTip(tr("Calendrier"));
    m_viewButtonGroup->addButton(m_btnTimeline, 4);
    
    m_btnBurndown = new QToolButton();
    m_btnBurndown->setIcon(QIcon(":/icons/gauge-high.svg"));
    m_btnBurndown->setText(tr("Avanc."));
    m_btnBurndown->setCheckable(true);
    m_btnBurndown->setToolTip(tr("Avancement"));
    m_viewButtonGroup->addButton(m_btnBurndown, 5);
    
    m_btnKanban = new QToolButton();
    m_btnKanban->setIcon(QIcon(":/icons/task.svg"));
    m_btnKanban->setText(tr("Kanban"));
    m_btnKanban->setCheckable(true);
    m_btnKanban->setToolTip(tr("Vue Kanban"));
    m_viewButtonGroup->addButton(m_btnKanban, 6);
    
    m_btnHeatmap = new QToolButton();
    m_btnHeatmap->setIcon(QIcon(":/icons/github-fill.svg"));
    m_btnHeatmap->setText(tr("Activity"));
    m_btnHeatmap->setCheckable(true);
    m_btnHeatmap->setToolTip(tr("Heatmap d'activité"));
    m_viewButtonGroup->addButton(m_btnHeatmap, 7);
    
    // Bouton pour réduire/masquer le panneau
    m_btnHide = new QToolButton();
    m_btnHide->setIcon(QIcon(":/icons/arrow-left.svg"));
    m_btnHide->setText(tr("◀"));
    m_btnHide->setToolTip(tr("Masquer le panneau"));
    connect(m_btnHide, &QToolButton::clicked, this, [this, scrollArea]() {
        if (m_rightPanel->isVisible()) {
            // Sauvegarder les tailles actuelles
            m_savedSplitterSizes = ui->splitter->sizes();
            
            // Masquer le panneau de contenu ET le scrollArea
            m_rightPanel->hide();
            scrollArea->hide();
            m_btnHide->setIcon(QIcon(":/icons/arrow-right.svg"));
            m_btnHide->setText(tr("▶"));
            m_btnHide->setToolTip(tr("Afficher le panneau"));
            
            // Réduire le rightContainer à sa taille minimale (juste la toolbar)
            int toolbarWidth = m_viewToolBar->sizeHint().width();
            QList<int> sizes = ui->splitter->sizes();
            int totalWidth = sizes[0] + sizes[1];
            ui->splitter->setSizes(QList<int>() << (totalWidth - toolbarWidth) << toolbarWidth);
        } else {
            // Afficher le panneau de contenu ET le scrollArea
            scrollArea->show();
            m_rightPanel->show();
            m_btnHide->setIcon(QIcon(":/icons/arrow-left.svg"));
            m_btnHide->setText(tr("◀"));
            m_btnHide->setToolTip(tr("Masquer le panneau"));
            
            // Restaurer les tailles sauvegardées
            if (!m_savedSplitterSizes.isEmpty()) {
                ui->splitter->setSizes(m_savedSplitterSizes);
            }
        }
    });
    
    // Ajouter les boutons à la toolbar
    m_viewToolBar->addWidget(m_btnDetails);
    m_viewToolBar->addWidget(m_btnStats);
    m_viewToolBar->addWidget(m_btnPomodoro);
    m_viewToolBar->addWidget(m_btnCharts);
    m_viewToolBar->addWidget(m_btnTimeline);
    m_viewToolBar->addWidget(m_btnBurndown);
    m_viewToolBar->addWidget(m_btnKanban);
    m_viewToolBar->addWidget(m_btnHeatmap);
    m_viewToolBar->addSeparator();
    m_viewToolBar->addWidget(m_btnHide);
    
    // Connecter les changements de bouton au changement de vue
    connect(m_viewButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &MainWindow::showView);
    
    // Assembler le conteneur de droite: toolbar à gauche + scroll area à droite
    rightLayout->addWidget(m_viewToolBar, 0);  // La toolbar reste fixe à gauche
    rightLayout->addWidget(scrollArea, 1);  // Le scroll area prend l'espace restant
    
    // Définir la taille policy du scroll area
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setMinimumWidth(0);  // Permettre au scroll area de rétrécir complètement
    
    // Remplacer le widget du splitter
    ui->splitter->replaceWidget(1, rightContainer);
    
    // Configurer le splitter pour que la partie gauche soit étirable
    ui->splitter->setStretchFactor(0, 1);  // La liste des tâches s'étire
    ui->splitter->setStretchFactor(1, 0);  // Le panneau de droite garde sa taille
    
    // Définir les tailles initiales du splitter (augmentées pour plus d'espace)
    ui->splitter->setSizes(QList<int>() << 800 << 600);
}

void MainWindow::showView(int index)
{
    if (index >= 0 && index < m_viewStack->count()) {
        m_viewStack->setCurrentIndex(index);
    }
}

void MainWindow::retranslateViewButtons()
{
    if (m_btnDetails) {
        m_btnDetails->setText(tr("Détails"));
        m_btnDetails->setToolTip(tr("Afficher les détails de la tâche"));
    }
    if (m_btnStats) {
        m_btnStats->setText(tr("Stats"));
        m_btnStats->setToolTip(tr("Afficher les statistiques"));
    }
    if (m_btnPomodoro) {
        m_btnPomodoro->setText(tr("Timer"));
        m_btnPomodoro->setToolTip(tr("Timer Pomodoro"));
    }
    if (m_btnCharts) {
        m_btnCharts->setText(tr("Charts"));
        m_btnCharts->setToolTip(tr("Graphiques"));
    }
    if (m_btnTimeline) {
        m_btnTimeline->setText(tr("Cal."));
        m_btnTimeline->setToolTip(tr("Calendrier"));
    }
    if (m_btnBurndown) {
        m_btnBurndown->setText(tr("Avanc."));
        m_btnBurndown->setToolTip(tr("Avancement"));
    }
    if (m_btnKanban) {
        m_btnKanban->setText(tr("Kanban"));
        m_btnKanban->setToolTip(tr("Vue Kanban"));
    }
    if (m_btnHeatmap) {
        m_btnHeatmap->setText(tr("Activity"));
        m_btnHeatmap->setToolTip(tr("Heatmap d'activité"));
    }
    if (m_btnHide) {
        // Update based on current state
        if (m_rightPanel && m_rightPanel->isVisible()) {
            m_btnHide->setText(tr("◀"));
            m_btnHide->setToolTip(tr("Masquer le panneau"));
        } else {
            m_btnHide->setText(tr("▶"));
            m_btnHide->setToolTip(tr("Afficher le panneau"));
        }
    }
}

void MainWindow::onSwitchToPersonalMode()
{
    switchViewMode(PersonalMode);
}

void MainWindow::onSwitchToGitMode()
{
    switchViewMode(GitMode);
}

void MainWindow::switchViewMode(ViewMode mode)
{
    if (m_viewMode == mode)
        return;
    
    m_viewMode = mode;
    
    if (mode == PersonalMode) {
        // Sauvegarder le widget Git et restaurer la vue personnelle
        QWidget *currentWidget = takeCentralWidget();  // Retire sans supprimer
        if (currentWidget && currentWidget != m_personalView) {
            currentWidget->hide();
        }
        
        if (m_personalView) {
            setCentralWidget(m_personalView);
            m_personalView->show();
        }
        ui->mainToolBar->setVisible(true);
        statusBar()->showMessage(tr("Mode Personnel activé"), 2000);
    } else {
        // Sauvegarder la vue personnelle et afficher le widget Git
        QWidget *currentWidget = takeCentralWidget();  // Retire sans supprimer
        if (currentWidget) {
            currentWidget->hide();
        }
        
        if (m_gitProjectWidget) {
            m_gitProjectWidget->setParent(this);  // Donner un parent temporairement
            setCentralWidget(m_gitProjectWidget);
            m_gitProjectWidget->show();
        }
        ui->mainToolBar->setVisible(false);  // Masquer la toolbar des tâches
        statusBar()->showMessage(tr("Mode Git/Issues activé"), 2000);
    }
}

void MainWindow::onTaskCreatedFromIssue(GitIssueTask *issue)
{
    if (!issue)
        return;
    
    // Créer une tâche personnelle liée à cette issue
    Task *personalTask = new Task(tr("Travail sur: %1").arg(issue->title()), this);
    personalTask->setDescription(tr("Issue liée: #%1\n\n%2")
                                .arg(issue->issueNumber())
                                .arg(issue->description()));
    personalTask->setLinkedIssueId(QString("%1/%2#%3")
                                   .arg(issue->repositoryUrl().section("/", -2, -2))  // owner
                                   .arg(issue->repositoryUrl().section("/", -1))      // repo
                                   .arg(issue->issueNumber()));
    
    // Copier les tags
    personalTask->setTags(issue->tags());
    
    // Ajouter au modèle
    m_taskModel->insertTask(personalTask);
    
    // Revenir en mode personnel
    switchViewMode(PersonalMode);
    
    statusBar()->showMessage(tr("Tâche personnelle créée depuis l'issue #%1").arg(issue->issueNumber()), 3000);
}

void MainWindow::onGitSyncRequested(GitRepository *repo)
{
    if (!repo)
        return;
    
    statusBar()->showMessage(tr("Synchronisation de %1/%2...").arg(repo->owner(), repo->repoName()), 2000);
    // La synchronisation est gérée par GitProjectWidget
}

void MainWindow::saveRepositories()
{
    if (!m_repositoryManager)
        return;
    
    QSettings settings;
    settings.beginGroup("GitRepositories");
    
    QJsonArray reposArray = m_repositoryManager->toJson();
    QJsonDocument doc(reposArray);
    settings.setValue("repositories", doc.toJson(QJsonDocument::Compact));
    
    settings.endGroup();
}

void MainWindow::loadRepositories()
{
    if (!m_repositoryManager)
        return;
    
    QSettings settings;
    settings.beginGroup("GitRepositories");
    
    QByteArray data = settings.value("repositories").toByteArray();
    if (!data.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            m_repositoryManager->fromJson(doc.array());
        }
    }
    
    settings.endGroup();
}
