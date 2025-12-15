#ifndef GITLABCONNECTOR_H
#define GITLABCONNECTOR_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include "gitissuetask.h"

/**
 * @brief Connecteur pour l'API GitLab
 * 
 * Gère l'authentification et la synchronisation bidirectionnelle avec GitLab.
 * Supporte Personal Access Tokens et OAuth2.
 */
class GitLabConnector : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString token READ token WRITE setToken NOTIFY tokenChanged)
    Q_PROPERTY(QString instanceUrl READ instanceUrl WRITE setInstanceUrl NOTIFY instanceUrlChanged)
    Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticatedChanged)
    
public:
    explicit GitLabConnector(QObject *parent = nullptr);
    ~GitLabConnector();
    
    // Configuration
    QString token() const { return m_token; }
    void setToken(const QString &token);
    
    QString instanceUrl() const { return m_instanceUrl; }
    void setInstanceUrl(const QString &url);  // gitlab.com par défaut, ou instance self-hosted
    
    bool isAuthenticated() const { return !m_token.isEmpty(); }
    
    // Opérations API
    void fetchIssue(const QString &projectId, int issueIid);
    void createIssue(const QString &projectId, GitIssueTask *task);
    void updateIssue(const QString &projectId, int issueIid, GitIssueTask *task);
    void closeIssue(const QString &projectId, int issueIid);
    void reopenIssue(const QString &projectId, int issueIid);
    
    // Requêtes avancées
    void listIssues(const QString &projectId, const QString &state = "opened");
    void searchIssues(const QString &query, const QString &projectId = QString());
    
    // Conversion JSON ↔ GitIssueTask
    static GitIssueTask* jsonToTask(const QJsonObject &json, QObject *parent = nullptr);
    static QJsonObject taskToJson(const GitIssueTask *task, bool createMode = false);
    
    // Utilitaires
    static QString projectPathToId(const QString &path);  // owner/repo → project_id
    
signals:
    void tokenChanged();
    void instanceUrlChanged();
    void authenticatedChanged();
    
    void issueFetched(int issueIid, const QJsonObject &data);
    void issueCreated(int issueIid, const QString &url);
    void issueUpdated(int issueIid);
    void issueClosed(int issueIid);
    void issueReopened(int issueIid);
    void issuesListed(const QJsonArray &issues);
    void issuesSearched(const QJsonArray &results);
    
    void errorOccurred(const QString &errorMessage, int httpCode = 0);
    
private slots:
    void handleNetworkReply(QNetworkReply *reply);
    
private:
    enum OperationType {
        FetchIssue,
        CreateIssue,
        UpdateIssue,
        CloseIssue,
        ReopenIssue,
        ListIssues,
        SearchIssues
    };
    
    void sendRequest(const QString &endpoint, OperationType operation, 
                     const QJsonObject &payload = QJsonObject(),
                     const QString &method = "GET");
    
    QNetworkAccessManager *m_networkManager;
    QString m_token;
    QString m_instanceUrl = "https://gitlab.com";
    
    // Suivi des requêtes en cours
    QHash<QNetworkReply*, OperationType> m_pendingRequests;
    QHash<QNetworkReply*, int> m_requestIssueIids;
};

#endif // GITLABCONNECTOR_H
