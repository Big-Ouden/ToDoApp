#ifndef GITHUBCONNECTOR_H
#define GITHUBCONNECTOR_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include "gitissuetask.h"

/**
 * @brief Connecteur pour l'API GitHub
 * 
 * Gère l'authentification et la synchronisation bidirectionnelle avec GitHub.
 * Supporte Personal Access Tokens et OAuth.
 */
class GitHubConnector : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString token READ token WRITE setToken NOTIFY tokenChanged)
    Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticatedChanged)
    
public:
    explicit GitHubConnector(QObject *parent = nullptr);
    ~GitHubConnector();
    
    // Authentification
    QString token() const { return m_token; }
    void setToken(const QString &token);
    bool isAuthenticated() const { return !m_token.isEmpty(); }
    
    // Opérations API
    void fetchIssue(const QString &owner, const QString &repo, int issueNumber);
    void createIssue(const QString &owner, const QString &repo, GitIssueTask *task);
    void updateIssue(const QString &owner, const QString &repo, int issueNumber, GitIssueTask *task);
    void closeIssue(const QString &owner, const QString &repo, int issueNumber);
    void reopenIssue(const QString &owner, const QString &repo, int issueNumber);
    
    // Requêtes avancées
    void listIssues(const QString &owner, const QString &repo, const QString &state = "open");
    void searchIssues(const QString &query);
    
    // Conversion JSON ↔ GitIssueTask
    static GitIssueTask* jsonToTask(const QJsonObject &json, QObject *parent = nullptr);
    static QJsonObject taskToJson(const GitIssueTask *task, bool createMode = false);
    
signals:
    void tokenChanged();
    void authenticatedChanged();
    
    void issueFetched(int issueNumber, const QJsonObject &data);
    void issueCreated(int issueNumber, const QString &url);
    void issueUpdated(int issueNumber);
    void issueClosed(int issueNumber);
    void issueReopened(int issueNumber);
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
    QString m_apiBaseUrl = "https://api.github.com";
    
    // Suivi des requêtes en cours
    QHash<QNetworkReply*, OperationType> m_pendingRequests;
    QHash<QNetworkReply*, int> m_requestIssueNumbers;
};

#endif // GITHUBCONNECTOR_H
