#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "taskitemdelegate.h"
#include "persistencemanager.h"
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

    // Place le widget de détail dans le panneau de droite
    ui->splitter->replaceWidget(1, m_detailWidget);
    ui->splitter->setSizes(QList<int>() << 600 << 400);

    // Configure la TreeView
    ui->taskTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->taskTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    ui->taskTreeView->expandAll();
    
    // Activer drag & drop
    ui->taskTreeView->setDragEnabled(true);
    ui->taskTreeView->setAcceptDrops(true);
    ui->taskTreeView->setDropIndicatorShown(true);
    ui->taskTreeView->setDragDropMode(QAbstractItemView::InternalMove);
    
    // Ajuster la largeur des colonnes
    ui->taskTreeView->setColumnWidth(0, 300); // Titre - plus large
    ui->taskTreeView->setColumnWidth(1, 120); // Date d'échéance
    ui->taskTreeView->setColumnWidth(2, 100); // Priorité
    ui->taskTreeView->setColumnWidth(3, 100); // Statut
    ui->taskTreeView->setColumnWidth(4, 100); // Catégorie
    ui->taskTreeView->setColumnWidth(5, 200); // Étiquettes

    // ========================================
    // Connexions des signaux - TreeView
    // ========================================
    // ========================================
    // Configuration des connexions
    // ========================================
    setupConnections();

    // ========================================
    // Initialisation
    // ========================================
    m_showCompleted = true;
    m_currentLanguage = "fr";
    
    // Charger les préférences avant de définir la locale
    loadPreferences();
    
    // Appliquer la langue chargée depuis les préférences
    setLanguage(m_currentLanguage);
    
    updateStatusBar();
    setWindowTitle(tr("ToDoApp - Nouveau fichier"));
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

    // ========================================
    // Connexions des signaux - Boutons de la barre d'outils
    // ========================================
    connect(ui->addTaskButton, &QToolButton::clicked, this, &MainWindow::onAddTask);
    connect(ui->addSubtaskButton, &QToolButton::clicked, this, &MainWindow::onAddSubtask);
    connect(ui->deleteTaskButton, &QToolButton::clicked, this, &MainWindow::onDeleteTask);
    connect(ui->markCompletedButton, &QToolButton::clicked, this, &MainWindow::onMarkCompleted);

    // ========================================
    // Connexions des signaux - Actions du menu
    // ========================================
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onNewFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::onSaveFileAs);
    
    // Créer les actions d'export PDF et d'impression
    QAction *exportPdfAction = new QAction(tr("Exporter en PDF..."), this);
    exportPdfAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    connect(exportPdfAction, &QAction::triggered, this, &MainWindow::onExportPdf);
    
    QAction *printAction = new QAction(tr("Imprimer..."), this);
    printAction->setShortcut(QKeySequence::Print);
    connect(printAction, &QAction::triggered, this, &MainWindow::onPrintTasks);
    
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
    
    connect(ui->actionExpandAll, &QAction::triggered, ui->taskTreeView, &QTreeView::expandAll);
    connect(ui->actionCollapseAll, &QAction::triggered, ui->taskTreeView, &QTreeView::collapseAll);
    
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

    // ========================================
    // Connexions des signaux - Recherche
    // ========================================
    ui->searchLineEdit->setPlaceholderText(tr("Rechercher (essayez: tag:, priority:, status:, date:)"));
    ui->searchLineEdit->setToolTip(tr("Recherche avancée:\n"
                                       "• tag:urgent - Rechercher par étiquette\n"
                                       "• priority:high - Rechercher par priorité\n"
                                       "• status:completed - Rechercher par statut\n"
                                       "• date:2024-12 - Rechercher par date\n"
                                       "• Texte simple - Recherche dans titre et description"));
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    
    // ========================================
    // Connexions des signaux - Filtres
    // ========================================
    connect(ui->priorityFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onPriorityFilterChanged);
    connect(ui->statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onStatusFilterChanged);

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
    m_taskModel->insertTask(t, nullptr);
    
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
    m_taskModel->insertTask(t, parent);
    
    // Développe le parent et sélectionne la nouvelle sous-tâche
    ui->taskTreeView->expand(cur);
}

void MainWindow::onDeleteTask()
{
    QModelIndex cur = ui->taskTreeView->currentIndex();
    if (!cur.isValid()) {
        QMessageBox::information(this, tr("Information"), 
                                tr("Veuillez d'abord sélectionner une tâche à supprimer."));
        return;
    }
    
    QModelIndex src = m_proxyModel->mapToSource(cur);
    Task *t = m_taskModel->getTask(src);
    if (!t) return;
    
    // Si l'utilisateur a désactivé la confirmation, supprimer directement
    if (!m_askDeleteConfirmation) {
        m_taskModel->removeTask(src);
        m_detailWidget->setTask(nullptr);
        return;
    }
    
    QString message = tr("Êtes-vous sûr de vouloir supprimer la tâche \"%1\" ?").arg(t->title());
    if (t->subtasks().size() > 0) {
        message += tr("\n\nCette tâche contient %1 sous-tâche(s) qui seront également supprimée(s).").arg(t->subtasks().size());
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
        m_taskModel->removeTask(src);
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
        statusBar()->showMessage(tr("Fichier enregistré : %1").arg(m_currentFilePath), 3000);
        setWindowTitle(tr("ToDoApp - %1").arg(QFileInfo(m_currentFilePath).fileName()));
    } else {
        QMessageBox::critical(this, tr("Erreur"), 
                            tr("Impossible d'enregistrer le fichier."));
        statusBar()->showMessage(tr("Échec de la sauvegarde"), 3000);
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
    priorityItems << tr("Tous") << tr("Très faible") << tr("Faible") 
                  << tr("Moyen") << tr("Élevé") << tr("Urgent");
    int currentPriority = ui->priorityFilterCombo->currentIndex();
    ui->priorityFilterCombo->clear();
    ui->priorityFilterCombo->addItems(priorityItems);
    ui->priorityFilterCombo->setCurrentIndex(currentPriority);
    
    QStringList statusItems;
    statusItems << tr("Tous") << tr("Non démarrée") << tr("En cours") 
                << tr("Complétée") << tr("Annulée");
    int currentStatus = ui->statusFilterCombo->currentIndex();
    ui->statusFilterCombo->clear();
    ui->statusFilterCombo->addItems(statusItems);
    ui->statusFilterCombo->setCurrentIndex(currentStatus);
    
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
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
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
