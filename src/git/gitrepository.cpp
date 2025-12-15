#include "gitrepository.h"
#include <QJsonObject>

/**
 * @brief Constructeur par défaut
 * @param parent Objet parent Qt
 */
GitRepository::GitRepository(QObject *parent)
    : QObject(parent),
      m_platform(GitPlatform::GitHub),
      m_instanceUrl("https://gitlab.com"),
      m_enabled(true)
{
}

/**
 * @brief Constructeur avec paramètres
 * @param name Nom du dépôt
 * @param platform Plateforme Git (GitHub, GitLab, Gitea)
 * @param owner Propriétaire du dépôt
 * @param repoName Nom du dépôt
 * @param parent Objet parent Qt
 */
GitRepository::GitRepository(const QString &name, GitPlatform platform,
                           const QString &owner, const QString &repoName,
                           QObject *parent)
    : QObject(parent),
      m_name(name),
      m_owner(owner),
      m_repoName(repoName),
      m_platform(platform),
      m_instanceUrl("https://gitlab.com"),
      m_enabled(true)
{
}

/**
 * @brief Définit le nom du dépôt
 * @param name Nouveau nom
 */
void GitRepository::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

/**
 * @brief Définit le propriétaire du dépôt
 * @param owner Nouveau propriétaire
 */
void GitRepository::setOwner(const QString &owner)
{
    if (m_owner != owner) {
        m_owner = owner;
        emit ownerChanged();
    }
}

/**
 * @brief Définit le nom du dépôt
 * @param repoName Nouveau nom de dépôt
 */
void GitRepository::setRepoName(const QString &repoName)
{
    if (m_repoName != repoName) {
        m_repoName = repoName;
        emit repoNameChanged();
    }
}

/**
 * @brief Définit la plateforme Git
 * @param platform Nouvelle plateforme (GitHub, GitLab, Gitea)
 */
void GitRepository::setPlatform(GitPlatform platform)
{
    if (m_platform != platform) {
        m_platform = platform;
        emit platformChanged();
    }
}

/**
 * @brief Définit le token d'authentification
 * @param token Nouveau token
 */
void GitRepository::setToken(const QString &token)
{
    if (m_token != token) {
        m_token = token;
        emit tokenChanged();
    }
}

/**
 * @brief Définit l'URL de l'instance (pour GitLab/Gitea auto-hébergé)
 * @param url Nouvelle URL d'instance
 */
void GitRepository::setInstanceUrl(const QString &url)
{
    if (m_instanceUrl != url) {
        m_instanceUrl = url;
        emit instanceUrlChanged();
    }
}

/**
 * @brief Active ou désactive le dépôt
 * @param enabled true pour activer, false pour désactiver
 */
void GitRepository::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged();
    }
}

/**
 * @brief Retourne l'URL complète du dépôt
 * @return URL du dépôt (ex: https://github.com/owner/repo)
 */
QString GitRepository::url() const
{
    QString baseUrl;
    
    switch (m_platform) {
        case GitPlatform::GitHub:
            baseUrl = "https://github.com";
            break;
        case GitPlatform::GitLab:
            baseUrl = m_instanceUrl;
            break;
        case GitPlatform::Gitea:
            baseUrl = m_instanceUrl;
            break;
    }
    
    return QString("%1/%2/%3").arg(baseUrl, m_owner, m_repoName);
}

/**
 * @brief Retourne l'identifiant du projet (format owner/repo)
 * @return Identifiant du projet utilisé par GitLab
 */
QString GitRepository::projectId() const
{
    // GitLab accepte owner/repo comme project ID
    return QString("%1/%2").arg(m_owner, m_repoName);
}

/**
 * @brief Sérialise le dépôt en JSON
 * @return Objet JSON contenant toutes les propriétés du dépôt
 */
QJsonObject GitRepository::toJson() const
{
    QJsonObject json;
    json["name"] = m_name;
    json["owner"] = m_owner;
    json["repoName"] = m_repoName;
    json["platform"] = static_cast<int>(m_platform);
    json["token"] = m_token;
    json["instanceUrl"] = m_instanceUrl;
    json["enabled"] = m_enabled;
    return json;
}

/**
 * @brief Crée un dépôt à partir d'un objet JSON
 * @param json Objet JSON source
 * @param parent Objet parent Qt
 * @return Nouveau dépôt alloué dynamiquement
 */
GitRepository* GitRepository::fromJson(const QJsonObject &json, QObject *parent)
{
    GitRepository *repo = new GitRepository(parent);
    repo->setName(json["name"].toString());
    repo->setOwner(json["owner"].toString());
    repo->setRepoName(json["repoName"].toString());
    repo->setPlatform(static_cast<GitPlatform>(json["platform"].toInt()));
    repo->setToken(json["token"].toString());
    repo->setInstanceUrl(json["instanceUrl"].toString("https://gitlab.com"));
    repo->setEnabled(json["enabled"].toBool(true));
    return repo;
}
