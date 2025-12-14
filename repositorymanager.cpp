#include "repositorymanager.h"

/**
 * @brief Constructeur du gestionnaire de dépôts
 * @param parent Objet parent Qt
 */
RepositoryManager::RepositoryManager(QObject *parent)
    : QObject(parent),
      m_currentRepository(nullptr)
{
    m_githubConnector = new GitHubConnector(this);
    m_gitlabConnector = new GitLabConnector(this);
}

/**
 * @brief Destructeur - libère tous les dépôts
 */
RepositoryManager::~RepositoryManager()
{
    qDeleteAll(m_repositories);
}

/**
 * @brief Ajoute un dépôt à la collection
 * @param repo Dépôt à ajouter (ownership transféré)
 * @note Définit automatiquement le premier dépôt comme actuel
 */
void RepositoryManager::addRepository(GitRepository *repo)
{
    if (!repo)
        return;
    
    repo->setParent(this);
    m_repositories.append(repo);
    emit repositoryAdded(repo);
    emit countChanged();
    
    // Si c'est le premier dépôt, le définir comme actuel
    if (m_repositories.size() == 1) {
        setCurrentRepository(repo);
    }
}

/**
 * @brief Supprime un dépôt par index
 * @param index Index du dépôt à supprimer
 */
void RepositoryManager::removeRepository(int index)
{
    if (index < 0 || index >= m_repositories.size())
        return;
    
    GitRepository *repo = m_repositories.takeAt(index);
    
    if (repo == m_currentRepository) {
        m_currentRepository = m_repositories.isEmpty() ? nullptr : m_repositories.first();
        emit currentRepositoryChanged();
    }
    
    delete repo;
    emit repositoryRemoved(index);
    emit countChanged();
}

/**
 * @brief Supprime un dépôt par pointeur
 * @param repo Dépôt à supprimer
 */
void RepositoryManager::removeRepository(GitRepository *repo)
{
    int index = m_repositories.indexOf(repo);
    if (index >= 0) {
        removeRepository(index);
    }
}

/**
 * @brief Récupère un dépôt par index
 * @param index Index du dépôt
 * @return Pointeur vers le dépôt ou nullptr si invalide
 */
GitRepository* RepositoryManager::repository(int index) const
{
    if (index < 0 || index >= m_repositories.size())
        return nullptr;
    return m_repositories[index];
}

/**
 * @brief Recherche un dépôt par propriétaire et nom
 * @param owner Propriétaire du dépôt
 * @param repoName Nom du dépôt
 * @return Pointeur vers le dépôt ou nullptr si non trouvé
 */
GitRepository* RepositoryManager::findRepository(const QString &owner, const QString &repoName) const
{
    for (GitRepository *repo : m_repositories) {
        if (repo->owner() == owner && repo->repoName() == repoName) {
            return repo;
        }
    }
    return nullptr;
}

/**
 * @brief Retourne la liste des dépôts activés
 * @return Liste des dépôts avec enabled=true
 */
QList<GitRepository*> RepositoryManager::enabledRepositories() const
{
    QList<GitRepository*> enabled;
    for (GitRepository *repo : m_repositories) {
        if (repo->isEnabled()) {
            enabled.append(repo);
        }
    }
    return enabled;
}

/**
 * @brief Définit le dépôt actuel
 * @param repo Dépôt à définir comme actuel
 * @note Configure automatiquement le connecteur approprié
 */
void RepositoryManager::setCurrentRepository(GitRepository *repo)
{
    if (m_currentRepository != repo && m_repositories.contains(repo)) {
        m_currentRepository = repo;
        
        // Configurer le connecteur approprié
        if (repo) {
            if (repo->platform() == GitPlatform::GitHub) {
                m_githubConnector->setToken(repo->token());
            } else if (repo->platform() == GitPlatform::GitLab) {
                m_gitlabConnector->setToken(repo->token());
                m_gitlabConnector->setInstanceUrl(repo->instanceUrl());
            }
        }
        
        emit currentRepositoryChanged();
    }
}

/**
 * @brief Définit le dépôt actuel par index
 * @param index Index du dépôt
 */
void RepositoryManager::setCurrentRepository(int index)
{
    if (index >= 0 && index < m_repositories.size()) {
        setCurrentRepository(m_repositories[index]);
    }
}

/**
 * @brief Obtient le connecteur approprié pour un dépôt
 * @param repo Dépôt source
 * @return Connecteur GitHub ou GitLab selon la plateforme
 */
QObject* RepositoryManager::connectorForRepository(GitRepository *repo)
{
    if (!repo)
        return nullptr;
    
    if (repo->platform() == GitPlatform::GitHub) {
        return m_githubConnector;
    } else if (repo->platform() == GitPlatform::GitLab) {
        return m_gitlabConnector;
    }
    
    return nullptr;
}

/**
 * @brief Sérialise tous les dépôts en JSON
 * @return Tableau JSON des dépôts
 */
QJsonArray RepositoryManager::toJson() const
{
    QJsonArray array;
    for (GitRepository *repo : m_repositories) {
        array.append(repo->toJson());
    }
    return array;
}

/**
 * @brief Charge les dépôts depuis un tableau JSON
 * @param json Tableau JSON source
 * @note Supprime les dépôts existants avant le chargement
 */
void RepositoryManager::fromJson(const QJsonArray &json)
{
    // Nettoyer les dépôts existants
    qDeleteAll(m_repositories);
    m_repositories.clear();
    m_currentRepository = nullptr;
    
    // Charger les nouveaux dépôts
    for (const QJsonValue &val : json) {
        GitRepository *repo = GitRepository::fromJson(val.toObject(), this);
        m_repositories.append(repo);
    }
    
    // Définir le premier comme actuel
    if (!m_repositories.isEmpty()) {
        setCurrentRepository(m_repositories.first());
    }
    
    emit countChanged();
    emit currentRepositoryChanged();
}
