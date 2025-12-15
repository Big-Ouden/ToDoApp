#include "gitprojectwidget.h"
#include "ui_gitprojectwidget.h"
#include "gitsyncdialog.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QDesktopServices>

/**
 * @brief Constructeur du widget de gestion de projets Git
 * @param parent Widget parent
 */
GitProjectWidget::GitProjectWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::GitProjectWidget),
      m_repositoryManager(nullptr),
      m_loading(false)
{
    ui->setupUi(this);
    
    // Connecter les signaux de l'UI
    connect(ui->repoComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GitProjectWidget::onRepositoryChanged);
    connect(ui->addRepoButton, &QPushButton::clicked, this, &GitProjectWidget::onAddRepository);
    connect(ui->editRepoButton, &QPushButton::clicked, this, &GitProjectWidget::onEditRepository);
    connect(ui->removeRepoButton, &QPushButton::clicked, this, &GitProjectWidget::onRemoveRepository);
    connect(ui->syncButton, &QPushButton::clicked, this, &GitProjectWidget::onSyncRepository);
    
    connect(ui->showOpenIssues, &QCheckBox::toggled, this, &GitProjectWidget::onFilterChanged);
    connect(ui->showClosedIssues, &QCheckBox::toggled, this, &GitProjectWidget::onFilterChanged);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &GitProjectWidget::onSearchTextChanged);
    
    connect(ui->issuesListWidget, &QListWidget::itemSelectionChanged,
            this, &GitProjectWidget::onIssueSelectionChanged);
    connect(ui->issuesListWidget, &QListWidget::itemDoubleClicked,
            this, &GitProjectWidget::onIssueDoubleClicked);
    
    connect(ui->createIssueButton, &QPushButton::clicked, this, &GitProjectWidget::onCreateIssue);
    connect(ui->createTaskFromIssueButton, &QPushButton::clicked, 
            this, &GitProjectWidget::onCreateTaskFromIssue);
    
    // État initial
    ui->repoComboBox->setEnabled(false);
    ui->editRepoButton->setEnabled(false);
    ui->removeRepoButton->setEnabled(false);
    ui->syncButton->setEnabled(false);
    ui->createTaskFromIssueButton->setEnabled(false);
}

/**
 * @brief Destructeur - libère les issues allouées
 */
GitProjectWidget::~GitProjectWidget()
{
    qDeleteAll(m_allIssues);
    delete ui;
}

/**
 * @brief Définit le gestionnaire de dépôts
 * @param manager Gestionnaire de dépôts
 * @note Connecte automatiquement les signaux nécessaires
 */
void GitProjectWidget::setRepositoryManager(RepositoryManager *manager)
{
    m_repositoryManager = manager;
    
    if (m_repositoryManager) {
        connect(m_repositoryManager, &RepositoryManager::repositoryAdded,
                this, &GitProjectWidget::updateRepositoryCombo);
        connect(m_repositoryManager, &RepositoryManager::repositoryRemoved,
                this, &GitProjectWidget::updateRepositoryCombo);
        connect(m_repositoryManager, &RepositoryManager::countChanged,
                this, &GitProjectWidget::updateRepositoryCombo);
        connect(m_repositoryManager, &RepositoryManager::currentRepositoryChanged,
                this, &GitProjectWidget::updateRepositoryInfo);
        
        connectRepositorySignals();
        updateRepositoryCombo();
        updateRepositoryInfo();
    }
}

/**
 * @brief Met à jour la combo box des dépôts
 * @note Active/désactive les boutons selon la présence de dépôts
 */
void GitProjectWidget::updateRepositoryCombo()
{
    // Bloquer les signaux pendant la mise à jour pour éviter les changements intempestifs
    ui->repoComboBox->blockSignals(true);
    ui->repoComboBox->clear();
    
    if (!m_repositoryManager) {
        ui->repoComboBox->blockSignals(false);
        return;
    }
    
    for (GitRepository *repo : m_repositoryManager->repositories()) {
        QString displayName = QString("%1 (%2/%3)")
            .arg(repo->name())
            .arg(repo->owner())
            .arg(repo->repoName());
        ui->repoComboBox->addItem(displayName);
    }
    
    bool hasRepos = m_repositoryManager->count() > 0;
    ui->repoComboBox->setEnabled(hasRepos);
    ui->editRepoButton->setEnabled(hasRepos);
    ui->removeRepoButton->setEnabled(hasRepos);
    ui->syncButton->setEnabled(hasRepos);
    
    // Rétablir les signaux
    ui->repoComboBox->blockSignals(false);
    
    // Sélectionner le dépôt actuel
    if (hasRepos && m_repositoryManager->currentRepository()) {
        int currentIndex = m_repositoryManager->repositories().indexOf(m_repositoryManager->currentRepository());
        if (currentIndex >= 0) {
            ui->repoComboBox->setCurrentIndex(currentIndex);
        }
    }
}

/**
 * @brief Met à jour l'affichage des informations du dépôt actuel
 */
void GitProjectWidget::updateRepositoryInfo()
{
    if (!m_repositoryManager || !m_repositoryManager->currentRepository()) {
        ui->repoInfoLabel->setText(tr("Aucun dépôt configuré"));
        return;
    }
    
    GitRepository *repo = m_repositoryManager->currentRepository();
    QString platformName;
    switch (repo->platform()) {
        case GitPlatform::GitHub: platformName = "GitHub"; break;
        case GitPlatform::GitLab: platformName = "GitLab"; break;
        case GitPlatform::Gitea: platformName = "Gitea"; break;
    }
    
    QString info = QString("<b>%1</b> - %2 - <a href=\"%3\">%3</a>")
        .arg(platformName)
        .arg(repo->fullName())
        .arg(repo->url());
    
    ui->repoInfoLabel->setText(info);
    ui->repoInfoLabel->setOpenExternalLinks(true);
    
    loadIssuesForCurrentRepository();
}

void GitProjectWidget::onRepositoryChanged(int index)
{
    if (m_repositoryManager && index >= 0) {
        m_repositoryManager->setCurrentRepository(index);
        updateRepositoryInfo();
    }
}

void GitProjectWidget::onAddRepository()
{
    GitSyncDialog *dialog = new GitSyncDialog(this, GitSyncDialog::AddRepositoryMode);
    dialog->setWindowTitle(tr("Ajouter un dépôt"));
    dialog->setModal(true);
    
    if (m_repositoryManager) {
        dialog->setGitHubConnector(m_repositoryManager->githubConnector());
        dialog->setGitLabConnector(m_repositoryManager->gitlabConnector());
    }
    
    connect(dialog, &QDialog::accepted, [this, dialog]() {
        if (!m_repositoryManager) {
            dialog->deleteLater();
            return;
        }
        
        // Récupérer les infos du dialogue
        QString repoName = dialog->repositoryName();
        if (repoName.isEmpty() || !repoName.contains("/")) {
            QMessageBox::warning(this, tr("Erreur"), 
                tr("Format de dépôt invalide. Utilisez: owner/repository"));
            dialog->deleteLater();
            return;
        }
        
        QStringList parts = repoName.split("/");
        QString owner = parts[0];
        QString repo = parts[1];
        
        // Créer un nouveau dépôt (le RepositoryManager sera le parent)
        GitRepository *newRepo = new GitRepository(m_repositoryManager);
        newRepo->setName(QString("%1/%2").arg(owner, repo));
        newRepo->setOwner(owner);
        newRepo->setRepoName(repo);
        newRepo->setPlatform(dialog->currentPlatform());
        newRepo->setToken(dialog->token());
        
        if (dialog->currentPlatform() == GitPlatform::GitLab) {
            QString instanceUrl = dialog->instanceUrl();
            if (instanceUrl.isEmpty()) {
                instanceUrl = "https://gitlab.com";
            }
            newRepo->setInstanceUrl(instanceUrl);
        }
        
        newRepo->setEnabled(true);
        
        // Ajouter au gestionnaire
        m_repositoryManager->addRepository(newRepo);
        
        // Sauvegarder dans QSettings via MainWindow
        emit repositoryAdded(newRepo);
        
        ui->statusLabel->setText(tr("✓ Dépôt %1 ajouté").arg(newRepo->name()));
        
        dialog->deleteLater();
    });
    
    connect(dialog, &QDialog::rejected, [dialog]() {
        dialog->deleteLater();
    });
    
    dialog->exec();
}

void GitProjectWidget::onEditRepository()
{
    if (!m_repositoryManager || !m_repositoryManager->currentRepository())
        return;
    
    // TODO: Ouvrir un dialogue d'édition
    QMessageBox::information(this, tr("Éditer"), 
        tr("Fonction d'édition de dépôt à implémenter"));
}

void GitProjectWidget::onRemoveRepository()
{
    if (!m_repositoryManager || !m_repositoryManager->currentRepository())
        return;
    
    GitRepository *repo = m_repositoryManager->currentRepository();
    auto reply = QMessageBox::question(this, tr("Supprimer le dépôt"),
        tr("Supprimer le dépôt '%1/%2' ?\nLes issues synchronisées seront conservées.")
            .arg(repo->owner(), repo->repoName()));
    
    if (reply == QMessageBox::Yes) {
        int index = ui->repoComboBox->currentIndex();
        m_repositoryManager->removeRepository(index);
        emit repositoryDeleted(repo);
        ui->statusLabel->setText(tr("✓ Dépôt supprimé"));
    }
}

void GitProjectWidget::onSyncRepository()
{
    if (!m_repositoryManager || !m_repositoryManager->currentRepository())
        return;
    
    GitRepository *repo = m_repositoryManager->currentRepository();
    
    // Avertir si pas de token (mais autoriser quand même)
    if (repo->token().isEmpty()) {
        ui->statusLabel->setText(tr("⚠ Pas de token - limité à 60 req/h pour repos publics"));
    }
    
    emit syncRequested(repo);
    loadIssuesForCurrentRepository();
}

void GitProjectWidget::loadIssuesForCurrentRepository()
{
    if (!m_repositoryManager || !m_repositoryManager->currentRepository()) {
        ui->statusLabel->setText(tr("❌ Aucun dépôt sélectionné"));
        return;
    }
    
    GitRepository *repo = m_repositoryManager->currentRepository();
    m_loading = true;
    ui->statusLabel->setText(tr("🔄 Chargement des issues de %1/%2...")
                            .arg(repo->owner(), repo->repoName()));
    
    QString state = "open";
    if (ui->showOpenIssues->isChecked() && ui->showClosedIssues->isChecked()) {
        state = "all";
    } else if (ui->showClosedIssues->isChecked()) {
        state = "closed";
    }
    
    qDebug() << "Loading issues for" << repo->owner() << "/" << repo->repoName() 
             << "platform:" << (int)repo->platform() << "state:" << state;
    
    if (repo->platform() == GitPlatform::GitHub) {
        GitHubConnector *connector = m_repositoryManager->githubConnector();
        if (!connector) {
            ui->statusLabel->setText(tr("❌ Connecteur GitHub non initialisé"));
            m_loading = false;
            return;
        }
        connector->setToken(repo->token());
        qDebug() << "Calling GitHub listIssues for" << repo->owner() << repo->repoName();
        connector->listIssues(repo->owner(), repo->repoName(), state);
    } else if (repo->platform() == GitPlatform::GitLab) {
        GitLabConnector *connector = m_repositoryManager->gitlabConnector();
        if (!connector) {
            ui->statusLabel->setText(tr("❌ Connecteur GitLab non initialisé"));
            m_loading = false;
            return;
        }
        connector->setToken(repo->token());
        connector->setInstanceUrl(repo->instanceUrl());
        QString gitlabState = (state == "open") ? "opened" : state;
        qDebug() << "Calling GitLab listIssues for" << repo->projectId();
        connector->listIssues(repo->projectId(), gitlabState);
    }
}

void GitProjectWidget::connectRepositorySignals()
{
    if (!m_repositoryManager)
        return;
    
    // GitHub
    GitHubConnector *github = m_repositoryManager->githubConnector();
    connect(github, &GitHubConnector::issuesListed,
            this, &GitProjectWidget::onIssuesListed, Qt::UniqueConnection);
    connect(github, &GitHubConnector::issueCreated,
            this, &GitProjectWidget::onIssueCreated, Qt::UniqueConnection);
    connect(github, &GitHubConnector::errorOccurred,
            this, &GitProjectWidget::onApiError, Qt::UniqueConnection);
    
    // GitLab
    GitLabConnector *gitlab = m_repositoryManager->gitlabConnector();
    connect(gitlab, &GitLabConnector::issuesListed,
            this, &GitProjectWidget::onIssuesListed, Qt::UniqueConnection);
    connect(gitlab, &GitLabConnector::issueCreated,
            this, &GitProjectWidget::onIssueCreated, Qt::UniqueConnection);
    connect(gitlab, &GitLabConnector::errorOccurred,
            this, &GitProjectWidget::onApiError, Qt::UniqueConnection);
}

void GitProjectWidget::onIssuesListed(const QJsonArray &issues)
{
    qDebug() << "onIssuesListed called with" << issues.size() << "issues";
    m_loading = false;
    
    // Nettoyer les anciennes issues
    qDeleteAll(m_allIssues);
    m_allIssues.clear();
    
    if (!m_repositoryManager || !m_repositoryManager->currentRepository()) {
        ui->statusLabel->setText(tr("❌ Aucun dépôt actif"));
        return;
    }
    
    GitRepository *repo = m_repositoryManager->currentRepository();
    
    // Convertir en GitIssueTasks
    for (const QJsonValue &val : issues) {
        QJsonObject json = val.toObject();
        GitIssueTask *task = nullptr;
        
        if (repo->platform() == GitPlatform::GitHub) {
            task = GitHubConnector::jsonToTask(json, this);
        } else if (repo->platform() == GitPlatform::GitLab) {
            task = GitLabConnector::jsonToTask(json, this);
        }
        
        if (task) {
            task->setRepositoryUrl(repo->url());
            m_allIssues.append(task);
        }
    }
    
    ui->statusLabel->setText(tr("%1 issue(s) chargée(s)").arg(m_allIssues.size()));
    refreshIssuesList();
}

void GitProjectWidget::onIssueCreated(int issueNumber, const QString &url)
{
    ui->statusLabel->setText(tr("Issue #%1 créée").arg(issueNumber));
    loadIssuesForCurrentRepository();
}

void GitProjectWidget::onApiError(const QString &error, int httpCode)
{
    qDebug() << "onApiError called:" << error << "code:" << httpCode;
    m_loading = false;
    QString msg = tr("Erreur API: %1").arg(error);
    if (httpCode > 0) {
        msg += tr(" (code %1)").arg(httpCode);
    }
    ui->statusLabel->setText(msg);
    QMessageBox::warning(this, tr("Erreur"), msg);
}

void GitProjectWidget::refreshIssuesList()
{
    ui->issuesListWidget->clear();
    m_displayedIssues.clear();
    
    QString searchText = ui->searchLineEdit->text().toLower();
    
    for (GitIssueTask *issue : m_allIssues) {
        // Filtrer par recherche
        if (!searchText.isEmpty()) {
            QString title = issue->title().toLower();
            QString desc = issue->description().toLower();
            if (!title.contains(searchText) && !desc.contains(searchText)) {
                continue;
            }
        }
        
        m_displayedIssues.append(issue);
        
        QString itemText = QString("#%1: %2").arg(issue->issueNumber()).arg(issue->title());
        if (issue->status() == Status::COMPLETED) {
            itemText = "✓ " + itemText;
        }
        
        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, QVariant::fromValue(issue));
        ui->issuesListWidget->addItem(item);
    }
}

void GitProjectWidget::onFilterChanged()
{
    loadIssuesForCurrentRepository();
}

void GitProjectWidget::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text);
    refreshIssuesList();
}

void GitProjectWidget::onIssueSelectionChanged()
{
    GitIssueTask *issue = selectedIssue();
    ui->createTaskFromIssueButton->setEnabled(issue != nullptr);
    
    if (issue) {
        emit issueSelected(issue);
    }
}

void GitProjectWidget::onIssueDoubleClicked(QListWidgetItem *item)
{
    if (!item)
        return;
    
    GitIssueTask *issue = item->data(Qt::UserRole).value<GitIssueTask*>();
    if (issue && !issue->issueUrl().isEmpty()) {
        QDesktopServices::openUrl(issue->issueUrl());
    }
}

GitIssueTask* GitProjectWidget::selectedIssue() const
{
    QListWidgetItem *item = ui->issuesListWidget->currentItem();
    if (!item)
        return nullptr;
    
    return item->data(Qt::UserRole).value<GitIssueTask*>();
}

void GitProjectWidget::onCreateIssue()
{
    // TODO: Dialogue de création d'issue
    QMessageBox::information(this, tr("Créer une issue"),
        tr("Fonction de création d'issue à implémenter"));
}

void GitProjectWidget::onCreateTaskFromIssue()
{
    GitIssueTask *issue = selectedIssue();
    if (issue) {
        emit taskCreatedFromIssue(issue);
    }
}
