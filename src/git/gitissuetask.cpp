#include "gitissuetask.h"

/**
 * @brief Constructeur d'une tâche liée à une issue Git
 * @param title Titre de la tâche
 * @param parent Objet parent Qt
 * @note Connecte automatiquement les signaux pour détecter les modifications locales
 */
GitIssueTask::GitIssueTask(const QString &title, QObject *parent)
    : Task(title, parent),
      m_issueNumber(0),
      m_platform(GitPlatform::GitHub)
{
    // Connecter les signaux de Task pour détecter les modifications locales
    connect(this, &Task::titleChanged, this, &GitIssueTask::markLocallyModified);
    connect(this, &Task::descriptionChanged, this, &GitIssueTask::markLocallyModified);
    connect(this, &Task::statusChanged, this, &GitIssueTask::markLocallyModified);
    connect(this, &Task::priorityChanged, this, &GitIssueTask::markLocallyModified);
    connect(this, &Task::tagsChanged, this, &GitIssueTask::markLocallyModified);
}

/**
 * @brief Construit l'URL de l'issue sur la plateforme
 * @return URL complète de l'issue (vide si non configurée)
 * @note Format adapté selon la plateforme (GitHub: /issues/, GitLab: /-/issues/)
 */
QUrl GitIssueTask::issueUrl() const
{
    if (m_repositoryUrl.isEmpty() || m_issueNumber <= 0)
        return QUrl();
    
    QString url = m_repositoryUrl;
    if (!url.endsWith("/"))
        url += "/";
    
    switch (m_platform) {
        case GitPlatform::GitHub:
        case GitPlatform::Gitea:
            url += QString("issues/%1").arg(m_issueNumber);
            break;
        case GitPlatform::GitLab:
            url += QString("-/issues/%1").arg(m_issueNumber);
            break;
    }
    
    return QUrl(url);
}

/**
 * @brief Vérifie si la tâche a été modifiée localement depuis la dernière synchro
 * @return true si modifiée ou jamais synchronisée
 */
bool GitIssueTask::isModifiedSinceSync() const
{
    if (!m_lastSyncDate.isValid())
        return true;  // Jamais synchronisé
    
    return m_lastLocalModification > m_lastSyncDate;
}

/**
 * @brief Définit le numéro de l'issue
 * @param number Numéro d'issue (GitHub) ou IID (GitLab)
 */
void GitIssueTask::setIssueNumber(int number)
{
    if (m_issueNumber != number) {
        m_issueNumber = number;
        emit issueNumberChanged();
    }
}

void GitIssueTask::setRepositoryUrl(const QString &url)
{
    if (m_repositoryUrl != url) {
        m_repositoryUrl = url;
        emit repositoryUrlChanged();
    }
}

void GitIssueTask::setPlatform(GitPlatform platform)
{
    if (m_platform != platform) {
        m_platform = platform;
        emit platformChanged();
    }
}

void GitIssueTask::setAssignees(const QStringList &assignees)
{
    if (m_assignees != assignees) {
        m_assignees = assignees;
        emit assigneesChanged();
        markLocallyModified();
    }
}

void GitIssueTask::setMilestone(const QString &milestone)
{
    if (m_milestone != milestone) {
        m_milestone = milestone;
        emit milestoneChanged();
        markLocallyModified();
    }
}

void GitIssueTask::setLastSyncDate(const QDateTime &date)
{
    if (m_lastSyncDate != date) {
        m_lastSyncDate = date;
        emit lastSyncDateChanged();
    }
}

/**
 * @brief Marque la tâche comme modifiée localement
 * @note Enregistre l'heure actuelle comme timestamp de modification
 */
void GitIssueTask::markLocallyModified()
{
    m_lastLocalModification = QDateTime::currentDateTime();
}

void GitIssueTask::clearLocallyModified()
{
    m_lastLocalModification = QDateTime();
}
