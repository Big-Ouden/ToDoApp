#ifndef REPOSITORYMANAGER_H
#define REPOSITORYMANAGER_H

#include <QObject>
#include <QList>
#include <QJsonArray>
#include "gitrepository.h"
#include "githubconnector.h"
#include "gitlabconnector.h"

/**
 * @brief Gestionnaire de dépôts Git multiples
 * 
 * Gère une collection de dépôts GitHub/GitLab configurés.
 * Fournit des connecteurs appropriés pour chaque plateforme.
 */
class RepositoryManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(GitRepository* currentRepository READ currentRepository NOTIFY currentRepositoryChanged)
    
public:
    explicit RepositoryManager(QObject *parent = nullptr);
    ~RepositoryManager();
    
    // Gestion des dépôts
    void addRepository(GitRepository *repo);
    void removeRepository(int index);
    void removeRepository(GitRepository *repo);
    GitRepository* repository(int index) const;
    GitRepository* findRepository(const QString &owner, const QString &repoName) const;
    int count() const { return m_repositories.size(); }
    QList<GitRepository*> repositories() const { return m_repositories; }
    QList<GitRepository*> enabledRepositories() const;
    
    // Dépôt actuel
    GitRepository* currentRepository() const { return m_currentRepository; }
    void setCurrentRepository(GitRepository *repo);
    void setCurrentRepository(int index);
    
    // Connecteurs
    GitHubConnector* githubConnector() { return m_githubConnector; }
    GitLabConnector* gitlabConnector() { return m_gitlabConnector; }
    
    // Obtenir le connecteur approprié pour un dépôt
    QObject* connectorForRepository(GitRepository *repo);
    
    // Sérialisation
    QJsonArray toJson() const;
    void fromJson(const QJsonArray &json);
    
signals:
    void repositoryAdded(GitRepository *repo);
    void repositoryRemoved(int index);
    void countChanged();
    void currentRepositoryChanged();
    
private:
    QList<GitRepository*> m_repositories;
    GitRepository *m_currentRepository;
    
    GitHubConnector *m_githubConnector;
    GitLabConnector *m_gitlabConnector;
};

#endif // REPOSITORYMANAGER_H
