#ifndef GITREPOSITORY_H
#define GITREPOSITORY_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include "gitissuetask.h"

/**
 * @brief Représente un dépôt Git configuré
 * 
 * Stocke la configuration d'un dépôt GitHub/GitLab avec authentification.
 * Permet la sérialisation JSON pour persistance.
 */
class GitRepository : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString owner READ owner WRITE setOwner NOTIFY ownerChanged)
    Q_PROPERTY(QString repoName READ repoName WRITE setRepoName NOTIFY repoNameChanged)
    Q_PROPERTY(GitPlatform platform READ platform WRITE setPlatform NOTIFY platformChanged)
    Q_PROPERTY(QString token READ token WRITE setToken NOTIFY tokenChanged)
    Q_PROPERTY(QString instanceUrl READ instanceUrl WRITE setInstanceUrl NOTIFY instanceUrlChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    
public:
    explicit GitRepository(QObject *parent = nullptr);
    explicit GitRepository(const QString &name, GitPlatform platform, 
                          const QString &owner, const QString &repoName,
                          QObject *parent = nullptr);
    
    // Getters
    QString name() const { return m_name; }
    QString owner() const { return m_owner; }
    QString repoName() const { return m_repoName; }
    GitPlatform platform() const { return m_platform; }
    QString token() const { return m_token; }
    QString instanceUrl() const { return m_instanceUrl; }
    bool isEnabled() const { return m_enabled; }
    
    // Setters
    void setName(const QString &name);
    void setOwner(const QString &owner);
    void setRepoName(const QString &repoName);
    void setPlatform(GitPlatform platform);
    void setToken(const QString &token);
    void setInstanceUrl(const QString &url);
    void setEnabled(bool enabled);
    
    // Helpers
    QString fullName() const { return QString("%1/%2").arg(m_owner, m_repoName); }
    QString url() const;
    QString projectId() const;  // Pour GitLab: owner/repo ou ID numérique
    
    // Sérialisation
    QJsonObject toJson() const;
    static GitRepository* fromJson(const QJsonObject &json, QObject *parent = nullptr);
    
signals:
    void nameChanged();
    void ownerChanged();
    void repoNameChanged();
    void platformChanged();
    void tokenChanged();
    void instanceUrlChanged();
    void enabledChanged();
    
private:
    QString m_name;          // Nom d'affichage (ex: "Kernel Linux")
    QString m_owner;         // Propriétaire (ex: "torvalds")
    QString m_repoName;      // Nom du dépôt (ex: "linux")
    GitPlatform m_platform;  // GitHub, GitLab, Gitea
    QString m_token;         // Token d'authentification
    QString m_instanceUrl;   // Pour GitLab self-hosted
    bool m_enabled;          // Actif ou non
};

#endif // GITREPOSITORY_H
