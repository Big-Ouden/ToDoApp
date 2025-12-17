#include "gitsyncdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSettings>
#include <QMessageBox>
#include <QDateTime>
#include <QJsonArray>

/**
 * @brief Constructeur du dialogue de synchronisation Git
 * @param parent Widget parent
 * @param mode Mode du dialogue (SyncMode ou AddRepositoryMode)
 */
GitSyncDialog::GitSyncDialog(QWidget *parent, DialogMode mode)
    : QDialog(parent),
      m_githubConnector(nullptr),
      m_gitlabConnector(nullptr),
      m_dialogMode(mode),
      m_currentPlatform(GitPlatform::GitHub),
      m_syncInProgress(false),
      m_testingConnection(false),
      m_syncButton(nullptr),
      m_cancelButton(nullptr),
      m_closeButton(nullptr),
      m_okButton(nullptr)
{
    setupUi();
    loadSettings();
    
    setWindowTitle(tr("Synchronisation Git"));
    resize(800, 600);
}

/**
 * @brief Destructeur - sauvegarde les paramètres
 */
GitSyncDialog::~GitSyncDialog()
{
    saveSettings();
}

/**
 * @brief Construit l'interface utilisateur du dialogue
 * @note Crée les widgets et connecte les signaux
 */
void GitSyncDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Configuration de la plateforme
    QGroupBox *platformGroup = new QGroupBox(tr("Plateforme Git"));
    QFormLayout *platformLayout = new QFormLayout(platformGroup);
    
    m_platformCombo = new QComboBox();
    m_platformCombo->addItem("GitHub", static_cast<int>(GitPlatform::GitHub));
    m_platformCombo->addItem("GitLab", static_cast<int>(GitPlatform::GitLab));
    m_platformCombo->addItem("Gitea", static_cast<int>(GitPlatform::Gitea));
    connect(m_platformCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GitSyncDialog::onPlatformChanged);
    platformLayout->addRow(tr("Plateforme:"), m_platformCombo);
    
    m_tokenEdit = new QLineEdit();
    m_tokenEdit->setEchoMode(QLineEdit::Password);
    m_tokenEdit->setPlaceholderText(tr("Token (optionnel pour repos publics)"));
    platformLayout->addRow(tr("Token:"), m_tokenEdit);
    
    m_instanceUrlEdit = new QLineEdit();
    m_instanceUrlEdit->setPlaceholderText(tr("https://gitlab.example.com"));
    m_instanceUrlEdit->setVisible(false);
    platformLayout->addRow(tr("URL instance:"), m_instanceUrlEdit);
    
    QHBoxLayout *connectionLayout = new QHBoxLayout();
    m_testConnectionBtn = new QPushButton(tr("Tester la connexion"));
    connect(m_testConnectionBtn, &QPushButton::clicked, this, &GitSyncDialog::onTestConnection);
    connectionLayout->addWidget(m_testConnectionBtn);
    m_connectionStatus = new QLabel();
    connectionLayout->addWidget(m_connectionStatus);
    connectionLayout->addStretch();
    platformLayout->addRow("", connectionLayout);
    
    mainLayout->addWidget(platformGroup);
    
    // Configuration du dépôt
    QGroupBox *repoGroup = new QGroupBox(tr("Dépôt"));
    QFormLayout *repoLayout = new QFormLayout(repoGroup);
    
    QHBoxLayout *repoInputLayout = new QHBoxLayout();
    m_repositoryEdit = new QLineEdit();
    m_repositoryEdit->setPlaceholderText(tr("owner/repository"));
    repoInputLayout->addWidget(m_repositoryEdit);
    m_browseRepoBtn = new QPushButton(tr("Parcourir..."));
    connect(m_browseRepoBtn, &QPushButton::clicked, this, &GitSyncDialog::onBrowseRepositories);
    repoInputLayout->addWidget(m_browseRepoBtn);
    repoLayout->addRow(tr("Dépôt:"), repoInputLayout);
    
    mainLayout->addWidget(repoGroup);
    
    // Options de synchronisation
    QGroupBox *syncGroup = new QGroupBox(tr("Options de synchronisation"));
    QFormLayout *syncLayout = new QFormLayout(syncGroup);
    
    m_syncModeCombo = new QComboBox();
    m_syncModeCombo->addItem(tr("Import uniquement"), static_cast<int>(SyncModeType::ImportOnly));
    m_syncModeCombo->addItem(tr("Export uniquement"), static_cast<int>(SyncModeType::ExportOnly));
    m_syncModeCombo->addItem(tr("Bidirectionnel"), static_cast<int>(SyncModeType::Bidirectional));
    syncLayout->addRow(tr("Mode:"), m_syncModeCombo);
    
    m_conflictResolutionCombo = new QComboBox();
    m_conflictResolutionCombo->addItem(tr("Local prioritaire"), static_cast<int>(LocalWins));
    m_conflictResolutionCombo->addItem(tr("Distant prioritaire"), static_cast<int>(RemoteWins));
    m_conflictResolutionCombo->addItem(tr("Manuel"), static_cast<int>(Manual));
    syncLayout->addRow(tr("Conflits:"), m_conflictResolutionCombo);
    
    m_autoSyncCheckBox = new QCheckBox(tr("Synchronisation automatique (toutes les 15 min)"));
    syncLayout->addRow("", m_autoSyncCheckBox);
    
    mainLayout->addWidget(syncGroup);
    
    // Filtres d'import
    QGroupBox *filterGroup = new QGroupBox(tr("Filtres d'import"));
    QVBoxLayout *filterLayout = new QVBoxLayout(filterGroup);
    
    m_importOpenIssues = new QCheckBox(tr("Issues ouvertes"));
    m_importOpenIssues->setChecked(true);
    filterLayout->addWidget(m_importOpenIssues);
    
    m_importClosedIssues = new QCheckBox(tr("Issues fermées"));
    filterLayout->addWidget(m_importClosedIssues);
    
    QFormLayout *labelFilterLayout = new QFormLayout();
    m_labelFilterEdit = new QLineEdit();
    m_labelFilterEdit->setPlaceholderText(tr("bug, enhancement (vide = tous)"));
    labelFilterLayout->addRow(tr("Labels:"), m_labelFilterEdit);
    filterLayout->addLayout(labelFilterLayout);
    
    mainLayout->addWidget(filterGroup);
    
    // Aperçu et logs
    QGroupBox *previewGroup = new QGroupBox(tr("Aperçu"));
    QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);
    
    m_previewList = new QListWidget();
    m_previewList->setMaximumHeight(150);
    previewLayout->addWidget(m_previewList);
    
    m_logText = new QTextEdit();
    m_logText->setReadOnly(true);
    m_logText->setMaximumHeight(100);
    previewLayout->addWidget(m_logText);
    
    mainLayout->addWidget(previewGroup);
    
    // Barre de progression
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);
    
    // Boutons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    if (m_dialogMode == AddRepositoryMode) {
        // Mode ajout: bouton OK pour confirmer sans synchroniser
        m_okButton = new QPushButton(tr("Ajouter"));
        m_okButton->setDefault(true);
        connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
        buttonLayout->addWidget(m_okButton);
        
        QPushButton *cancelBtn = new QPushButton(tr("Annuler"));
        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
        buttonLayout->addWidget(cancelBtn);
    } else {
        // Mode synchronisation: boutons sync/annuler/fermer
        m_syncButton = new QPushButton(tr("Synchroniser"));
        m_syncButton->setDefault(true);
        connect(m_syncButton, &QPushButton::clicked, this, &GitSyncDialog::onStartSync);
        buttonLayout->addWidget(m_syncButton);
        
        m_cancelButton = new QPushButton(tr("Annuler"));
        m_cancelButton->setVisible(false);
        connect(m_cancelButton, &QPushButton::clicked, this, &GitSyncDialog::onCancelSync);
        buttonLayout->addWidget(m_cancelButton);
        
        m_closeButton = new QPushButton(tr("Fermer"));
        connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);
        buttonLayout->addWidget(m_closeButton);
    }
    
    mainLayout->addLayout(buttonLayout);
}

void GitSyncDialog::setGitHubConnector(GitHubConnector *connector)
{
    m_githubConnector = connector;
    
    if (m_githubConnector) {
        connect(m_githubConnector, &GitHubConnector::issuesListed,
                this, &GitSyncDialog::onIssuesListed);
        connect(m_githubConnector, &GitHubConnector::issueFetched,
                this, &GitSyncDialog::onIssueFetched);
        connect(m_githubConnector, &GitHubConnector::issueCreated,
                this, &GitSyncDialog::onIssueCreated);
        connect(m_githubConnector, &GitHubConnector::errorOccurred,
                this, &GitSyncDialog::onApiError);
    }
}

void GitSyncDialog::setGitLabConnector(GitLabConnector *connector)
{
    m_gitlabConnector = connector;
    
    if (m_gitlabConnector) {
        connect(m_gitlabConnector, &GitLabConnector::issuesListed,
                this, &GitSyncDialog::onIssuesListed);
        connect(m_gitlabConnector, &GitLabConnector::issueFetched,
                this, &GitSyncDialog::onIssueFetched);
        connect(m_gitlabConnector, &GitLabConnector::issueCreated,
                this, &GitSyncDialog::onIssueCreated);
        connect(m_gitlabConnector, &GitLabConnector::errorOccurred,
                this, &GitSyncDialog::onApiError);
    }
}

void GitSyncDialog::onPlatformChanged(int index)
{
    m_currentPlatform = static_cast<GitPlatform>(m_platformCombo->currentData().toInt());
    
    // Afficher/masquer l'URL d'instance pour GitLab
    bool isGitLab = (m_currentPlatform == GitPlatform::GitLab);
    m_instanceUrlEdit->setVisible(isGitLab);
    
    m_connectionStatus->clear();
}

void GitSyncDialog::onTestConnection()
{
    QString token = m_tokenEdit->text().trimmed();
    QString repo = m_repositoryEdit->text().trimmed();
    
    m_logText->clear();
    
    if (token.isEmpty()) {
        m_logText->append(tr("ℹ️ Mode sans authentification:"));
        m_logText->append(tr("  • GitHub: 60 requêtes/heure pour repos publics"));
        m_logText->append(tr("  • Ajoutez un token pour 5000 req/h ou accès aux repos privés"));
        updateConnectionStatus(true, tr("Mode public (60 req/h)"));
    } else {
        m_logText->append(tr("ℹ️ Token configuré: accès complet (5000 req/h)"));
        updateConnectionStatus(false, tr("Test en cours..."));
    }
    
    if (m_currentPlatform == GitPlatform::GitHub && m_githubConnector) {
        if (!token.isEmpty()) {
            m_githubConnector->setToken(token);
        }
        // Test avec une requête simple (liste des issues du repo si spécifié)
        if (!repo.isEmpty() && repo.contains("/")) {
            QStringList parts = repo.split("/");
            m_logText->append(tr("🔍 Test de connexion à %1/%2...").arg(parts[0], parts[1]));
            // Utiliser un flag pour indiquer que c'est un test
            m_testingConnection = true;
            m_githubConnector->listIssues(parts[0], parts[1], "open");
        } else {
            updateConnectionStatus(true, tr("Configuré (spécifiez un dépôt pour tester)"));
            m_logText->append(tr("⚠️ Spécifiez un dépôt (ex: torvalds/linux) pour tester la connexion"));
        }
    } else if (m_currentPlatform == GitPlatform::GitLab && m_gitlabConnector) {
        if (!token.isEmpty()) {
            m_gitlabConnector->setToken(token);
        }
        QString instanceUrl = m_instanceUrlEdit->text().trimmed();
        if (!instanceUrl.isEmpty()) {
            m_gitlabConnector->setInstanceUrl(instanceUrl);
        }
        m_logText->append(tr("✓ GitLab configuré: %1").arg(instanceUrl));
        updateConnectionStatus(true, tr("Configuré"));
    } else {
        updateConnectionStatus(false, tr("Connecteur non initialisé"));
        m_logText->append(tr("❌ Erreur: connecteur non initialisé"));
    }
}

void GitSyncDialog::onBrowseRepositories()
{
    QMessageBox::information(this, tr("Navigation"),
                            tr("Entrez manuellement owner/repository (ex: torvalds/linux)"));
}

void GitSyncDialog::onStartSync()
{
    if (m_repositoryEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Erreur"), tr("Veuillez spécifier un dépôt."));
        return;
    }
    
    m_syncInProgress = true;
    if (m_syncButton) m_syncButton->setEnabled(false);
    if (m_cancelButton) m_cancelButton->setVisible(true);
    m_progressBar->setVisible(true);
    m_logText->clear();
    m_previewList->clear();
    
    SyncModeType mode = static_cast<SyncModeType>(m_syncModeCombo->currentData().toInt());
    
    QString repo = m_repositoryEdit->text().trimmed();
    m_logText->append(tr("🔄 Démarrage de la synchronisation..."));
    m_logText->append(tr("📦 Dépôt: %1").arg(repo));
    m_logText->append(tr("📋 Mode: %1").arg(m_syncModeCombo->currentText()));
    m_logText->append("");
    
    switch (mode) {
        case SyncModeType::ImportOnly:
            startImport();
            break;
        case SyncModeType::ExportOnly:
            startExport();
            break;
        case SyncModeType::Bidirectional:
            startBidirectionalSync();
            break;
    }
}

void GitSyncDialog::onCancelSync()
{
    m_syncInProgress = false;
    if (m_syncButton) m_syncButton->setEnabled(true);
    if (m_cancelButton) m_cancelButton->setVisible(false);
    m_progressBar->setVisible(false);
    m_logText->append(tr("Synchronisation annulée."));
}

void GitSyncDialog::onIssuesListed(const QJsonArray &issues)
{
    // Si c'est un test de connexion
    if (m_testingConnection) {
        m_testingConnection = false;
        m_logText->append(tr("✓ Connexion réussie: %1 issue(s) trouvée(s)").arg(issues.size()));
        updateConnectionStatus(true, tr("Connecté avec succès"));
        return;
    }
    
    if (!m_syncInProgress)
        return;
    
    m_issuesToImport.clear();
    for (const QJsonValue &val : issues) {
        m_issuesToImport.append(val.toObject());
    }
    
    m_progressBar->setMaximum(m_issuesToImport.size());
    m_progressBar->setValue(0);
    
    m_logText->append(tr("📥 Trouvé %1 issue(s) à importer.").arg(issues.size()));
    
    int imported = 0;
    for (const QJsonObject &issueJson : m_issuesToImport) {
        GitIssueTask *task = nullptr;
        
        if (m_currentPlatform == GitPlatform::GitHub) {
            task = GitHubConnector::jsonToTask(issueJson, this);
        } else if (m_currentPlatform == GitPlatform::GitLab) {
            task = GitLabConnector::jsonToTask(issueJson, this);
        }
        
        if (task) {
            emit taskImported(task);
            QString itemText = QString("#%1: %2").arg(task->issueNumber()).arg(task->title());
            m_previewList->addItem(itemText);
            m_logText->append(tr("  ✓ Importé: %1").arg(itemText));
            imported++;
            m_progressBar->setValue(imported);
        }
    }
    
    m_logText->append(tr("\n✅ Import terminé: %1 tâche(s) importée(s).").arg(imported));
    emit syncCompleted(imported, 0, 0);
    
    m_syncInProgress = false;
    m_syncButton->setEnabled(true);
    m_cancelButton->setVisible(false);
}

void GitSyncDialog::onIssueFetched(int issueNumber, const QJsonObject &data)
{
    m_logText->append(tr("Issue #%1 récupérée.").arg(issueNumber));
}

void GitSyncDialog::onIssueCreated(int issueNumber, const QString &url)
{
    m_logText->append(tr("Issue #%1 créée: %2").arg(issueNumber).arg(url));
}

void GitSyncDialog::onApiError(const QString &error, int httpCode)
{
    if (m_testingConnection) {
        m_testingConnection = false;
        m_logText->append(tr("❌ Erreur de connexion: %1 (HTTP %2)").arg(error).arg(httpCode));
        
        // Messages d'aide selon le code d'erreur
        if (httpCode == 404) {
            m_logText->append(tr("💡 Le dépôt n'existe pas ou est privé"));
            m_logText->append(tr("   Vérifiez l'orthographe: owner/repository"));
        } else if (httpCode == 401 || httpCode == 403) {
            m_logText->append(tr("💡 Problème d'authentification"));
            m_logText->append(tr("   Pour un repo privé, ajoutez un token valide"));
        } else if (httpCode == 0) {
            m_logText->append(tr("💡 Problème réseau: vérifiez votre connexion internet"));
        }
        
        updateConnectionStatus(false, tr("Erreur: %1").arg(error));
        return;
    }
    
    updateConnectionStatus(false, tr("Erreur: %1 (code %2)").arg(error).arg(httpCode));
    m_logText->append(tr("❌ Erreur API: %1").arg(error));
    
    m_syncInProgress = false;
    if (m_syncButton) m_syncButton->setEnabled(true);
    if (m_cancelButton) m_cancelButton->setVisible(false);
    m_progressBar->setVisible(false);
}

void GitSyncDialog::updateConnectionStatus(bool connected, const QString &message)
{
    if (connected) {
        m_connectionStatus->setText("✓ " + (message.isEmpty() ? tr("Connecté") : message));
        m_connectionStatus->setStyleSheet("color: green;");
    } else {
        m_connectionStatus->setText("✗ " + (message.isEmpty() ? tr("Non connecté") : message));
        m_connectionStatus->setStyleSheet("color: red;");
    }
}

void GitSyncDialog::startImport()
{
    QString repo = m_repositoryEdit->text().trimmed();
    QStringList parts = repo.split("/");
    
    if (parts.size() != 2) {
        m_logText->append(tr("Erreur: format de dépôt invalide (utilisez owner/repo)"));
        onCancelSync();
        return;
    }
    
    QString state = m_importOpenIssues->isChecked() ? "open" : "closed";
    if (m_importOpenIssues->isChecked() && m_importClosedIssues->isChecked()) {
        state = "all";
    }
    
    if (m_currentPlatform == GitPlatform::GitHub && m_githubConnector) {
        m_githubConnector->listIssues(parts[0], parts[1], state);
    } else if (m_currentPlatform == GitPlatform::GitLab && m_gitlabConnector) {
        QString projectId = parts[0] + "/" + parts[1];
        QString gitlabState = (state == "open") ? "opened" : state;
        m_gitlabConnector->listIssues(projectId, gitlabState);
    }
}

void GitSyncDialog::startExport()
{
    m_logText->append(tr("Export non encore implémenté."));
    onCancelSync();
}

void GitSyncDialog::startBidirectionalSync()
{
    m_logText->append(tr("Synchronisation bidirectionnelle non encore implémentée."));
    onCancelSync();
}

void GitSyncDialog::handleConflict(GitIssueTask *localTask, const QJsonObject &remoteIssue)
{
    ConflictResolution resolution = static_cast<ConflictResolution>(
        m_conflictResolutionCombo->currentData().toInt());
    
    if (resolution == Manual) {
        // Afficher un dialogue de résolution manuelle
    }
}

void GitSyncDialog::loadSettings()
{
    QSettings settings;
    settings.beginGroup("GitSync");
    
    int platform = settings.value("platform", static_cast<int>(GitPlatform::GitHub)).toInt();
    m_platformCombo->setCurrentIndex(m_platformCombo->findData(platform));
    
    m_tokenEdit->setText(settings.value("token").toString());
    m_instanceUrlEdit->setText(settings.value("instanceUrl", "https://gitlab.com").toString());
    m_repositoryEdit->setText(settings.value("repository").toString());
    
    m_syncModeCombo->setCurrentIndex(settings.value("syncMode", 0).toInt());
    m_conflictResolutionCombo->setCurrentIndex(settings.value("conflictResolution", 0).toInt());
    m_autoSyncCheckBox->setChecked(settings.value("autoSync", false).toBool());
    
    m_importOpenIssues->setChecked(settings.value("importOpen", true).toBool());
    m_importClosedIssues->setChecked(settings.value("importClosed", false).toBool());
    
    settings.endGroup();
}

void GitSyncDialog::saveSettings()
{
    QSettings settings;
    settings.beginGroup("GitSync");
    
    settings.setValue("platform", m_platformCombo->currentData().toInt());
    settings.setValue("token", m_tokenEdit->text());
    settings.setValue("instanceUrl", m_instanceUrlEdit->text());
    settings.setValue("repository", m_repositoryEdit->text());
    
    settings.setValue("syncMode", m_syncModeCombo->currentIndex());
    settings.setValue("conflictResolution", m_conflictResolutionCombo->currentIndex());
    settings.setValue("autoSync", m_autoSyncCheckBox->isChecked());
    
    settings.setValue("importOpen", m_importOpenIssues->isChecked());
    settings.setValue("importClosed", m_importClosedIssues->isChecked());
    
    settings.endGroup();
}
