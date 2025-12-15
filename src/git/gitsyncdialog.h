#ifndef GITSYNCDIALOG_H
#define GITSYNCDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>
#include <QCheckBox>
#include "gitissuetask.h"
#include "githubconnector.h"
#include "gitlabconnector.h"

/**
 * @brief Dialogue de synchronisation avec les plateformes Git
 * 
 * Permet de:
 * - Configurer les connexions (tokens, URLs)
 * - Sélectionner des dépôts
 * - Importer des issues existantes
 * - Exporter des tâches locales
 * - Synchroniser bidirectionnellement
 * - Gérer les conflits
 */
class GitSyncDialog : public QDialog
{
    Q_OBJECT
    
public:
    enum DialogMode {
        SyncMode,           // Mode synchronisation normale
        AddRepositoryMode   // Mode ajout de dépôt
    };
    
    enum SyncModeType {
        ImportOnly,      // Issues → Tâches uniquement
        ExportOnly,      // Tâches → Issues uniquement
        Bidirectional    // Synchronisation bidirectionnelle
    };
    
    enum ConflictResolution {
        LocalWins,       // Priorité aux modifications locales
        RemoteWins,      // Priorité aux modifications distantes
        Manual           // Demander à l'utilisateur pour chaque conflit
    };
    
    explicit GitSyncDialog(QWidget *parent = nullptr, DialogMode mode = SyncMode);
    ~GitSyncDialog();
    
    // Configuration
    void setGitHubConnector(GitHubConnector *connector);
    void setGitLabConnector(GitLabConnector *connector);
    
    // Getters pour récupérer les informations du dialogue
    GitPlatform currentPlatform() const { return m_currentPlatform; }
    QString repositoryName() const { return m_repositoryEdit ? m_repositoryEdit->text().trimmed() : QString(); }
    QString token() const { return m_tokenEdit ? m_tokenEdit->text().trimmed() : QString(); }
    QString instanceUrl() const { return m_instanceUrlEdit ? m_instanceUrlEdit->text().trimmed() : QString(); }
    
signals:
    void syncCompleted(int importedCount, int exportedCount, int conflictsCount);
    void taskImported(GitIssueTask *task);
    void taskExported(GitIssueTask *task);
    
private slots:
    void onPlatformChanged(int index);
    void onTestConnection();
    void onBrowseRepositories();
    void onStartSync();
    void onCancelSync();
    
    // Slots pour les réponses API
    void onIssuesListed(const QJsonArray &issues);
    void onIssueFetched(int issueNumber, const QJsonObject &data);
    void onIssueCreated(int issueNumber, const QString &url);
    void onApiError(const QString &error, int httpCode);
    
private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    
    void updateConnectionStatus(bool connected, const QString &message = QString());
    void startImport();
    void startExport();
    void startBidirectionalSync();
    
    void handleConflict(GitIssueTask *localTask, const QJsonObject &remoteIssue);
    
    // Configuration
    QComboBox *m_platformCombo;
    QLineEdit *m_tokenEdit;
    QLineEdit *m_instanceUrlEdit;  // Pour GitLab self-hosted
    QPushButton *m_testConnectionBtn;
    QLabel *m_connectionStatus;
    
    // Sélection du dépôt
    QLineEdit *m_repositoryEdit;  // owner/repo ou URL complète
    QPushButton *m_browseRepoBtn;
    
    // Mode de synchronisation
    QComboBox *m_syncModeCombo;
    QComboBox *m_conflictResolutionCombo;
    QCheckBox *m_autoSyncCheckBox;
    
    // Filtres d'import
    QCheckBox *m_importOpenIssues;
    QCheckBox *m_importClosedIssues;
    QLineEdit *m_labelFilterEdit;
    
    // Aperçu
    QListWidget *m_previewList;
    QTextEdit *m_logText;
    
    // Contrôles
    QProgressBar *m_progressBar;
    QPushButton *m_syncButton;
    QPushButton *m_cancelButton;
    QPushButton *m_closeButton;
    QPushButton *m_okButton;
    
    // Backend
    GitHubConnector *m_githubConnector;
    GitLabConnector *m_gitlabConnector;
    
    DialogMode m_dialogMode;
    GitPlatform m_currentPlatform;
    bool m_syncInProgress;
    bool m_testingConnection;
    QList<QJsonObject> m_issuesToImport;
    QList<GitIssueTask*> m_tasksToExport;
};

#endif // GITSYNCDIALOG_H
