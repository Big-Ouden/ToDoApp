#ifndef GITISSUETASK_H
#define GITISSUETASK_H

#include "task.h"
#include <QString>
#include <QUrl>
#include <QDateTime>

/**
 * @brief Type de plateforme Git supportée
 */
enum class GitPlatform {
    GitHub,
    GitLab,
    Gitea
};

/**
 * @brief Tâche synchronisée avec une issue Git (GitHub/GitLab/Gitea)
 * 
 * Cette classe étend Task pour ajouter les métadonnées spécifiques aux issues Git:
 * - Numéro d'issue
 * - URL du dépôt
 * - Plateforme (GitHub, GitLab, etc.)
 * - Labels
 * - Assignés
 * - Milestone
 * - État de synchronisation
 */
class GitIssueTask : public Task
{
    Q_OBJECT
    
    Q_PROPERTY(int issueNumber READ issueNumber WRITE setIssueNumber NOTIFY issueNumberChanged)
    Q_PROPERTY(QString repositoryUrl READ repositoryUrl WRITE setRepositoryUrl NOTIFY repositoryUrlChanged)
    Q_PROPERTY(GitPlatform platform READ platform WRITE setPlatform NOTIFY platformChanged)
    Q_PROPERTY(QStringList assignees READ assignees WRITE setAssignees NOTIFY assigneesChanged)
    Q_PROPERTY(QString milestone READ milestone WRITE setMilestone NOTIFY milestoneChanged)
    Q_PROPERTY(QDateTime lastSyncDate READ lastSyncDate WRITE setLastSyncDate NOTIFY lastSyncDateChanged)

public:
    explicit GitIssueTask(const QString &title = "", QObject *parent = nullptr);
    
    /** @return Numéro de l'issue (ex: #123) */
    int issueNumber() const { return m_issueNumber; }
    
    /** @return URL du dépôt (ex: https://github.com/user/repo) */
    QString repositoryUrl() const { return m_repositoryUrl; }
    
    /** @return Plateforme Git */
    GitPlatform platform() const { return m_platform; }
    
    /** @return Liste des assignés (usernames) */
    QStringList assignees() const { return m_assignees; }
    
    /** @return Nom du milestone */
    QString milestone() const { return m_milestone; }
    
    /** @return Date de dernière synchronisation */
    QDateTime lastSyncDate() const { return m_lastSyncDate; }
    
    /** @return URL complète de l'issue */
    QUrl issueUrl() const;
    
    /** @return true si la tâche n'a jamais été synchronisée */
    bool needsSync() const { return !m_lastSyncDate.isValid(); }
    
    /** @return true si la tâche a été modifiée depuis la dernière sync */
    bool isModifiedSinceSync() const;
    
    void setIssueNumber(int number);
    void setRepositoryUrl(const QString &url);
    void setPlatform(GitPlatform platform);
    void setAssignees(const QStringList &assignees);
    void setMilestone(const QString &milestone);
    void setLastSyncDate(const QDateTime &date);
    
    /**
     * @brief Marque la tâche comme modifiée localement
     * Utilisé pour détecter les conflits de synchronisation
     */
    void markLocallyModified();
    
    /**
     * @brief Réinitialise le flag de modification locale après sync
     */
    void clearLocallyModified();

signals:
    void issueNumberChanged();
    void repositoryUrlChanged();
    void platformChanged();
    void assigneesChanged();
    void milestoneChanged();
    void lastSyncDateChanged();
    void syncRequested();  ///< Émis quand la tâche demande une synchronisation

private:
    int m_issueNumber;
    QString m_repositoryUrl;
    GitPlatform m_platform;
    QStringList m_assignees;
    QString m_milestone;
    QDateTime m_lastSyncDate;
    QDateTime m_lastLocalModification;
};

#endif // GITISSUETASK_H
