#include "gitlabconnector.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QDateTime>
#include <QUrlQuery>
#include <QRegularExpression>

/**
 * @brief Constructeur du connecteur GitLab
 * @param parent Objet parent Qt
 */
GitLabConnector::GitLabConnector(QObject *parent)
    : QObject(parent),
      m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &GitLabConnector::handleNetworkReply);
}

GitLabConnector::~GitLabConnector()
{
}

/**
 * @brief Définit le token d'authentification GitLab
 * @param token Personal/Project Access Token
 */
void GitLabConnector::setToken(const QString &token)
{
    if (m_token != token) {
        m_token = token;
        emit tokenChanged();
        emit authenticatedChanged();
    }
}

/**
 * @brief Définit l'URL de l'instance GitLab
 * @param url URL de base (ex: https://gitlab.com ou instance auto-hébergée)
 * @note Supprime automatiquement le slash final
 */
void GitLabConnector::setInstanceUrl(const QString &url)
{
    if (m_instanceUrl != url) {
        m_instanceUrl = url;
        if (m_instanceUrl.endsWith("/"))
            m_instanceUrl.chop(1);
        emit instanceUrlChanged();
    }
}

/**
 * @brief Récupère une issue GitLab
 * @param projectId ID du projet (numérique ou owner/repo)
 * @param issueIid IID de l'issue (internal ID dans le projet)
 */
void GitLabConnector::fetchIssue(const QString &projectId, int issueIid)
{
    QString endpoint = QString("/api/v4/projects/%1/issues/%2")
                       .arg(QString(QUrl::toPercentEncoding(projectId))).arg(issueIid);
    sendRequest(endpoint, FetchIssue);
}

/**
 * @brief Crée une nouvelle issue GitLab
 * @param projectId ID du projet
 * @param task Tâche source à convertir en issue
 */
void GitLabConnector::createIssue(const QString &projectId, GitIssueTask *task)
{
    QString endpoint = QString("/api/v4/projects/%1/issues")
                       .arg(QString(QUrl::toPercentEncoding(projectId)));
    QJsonObject payload = taskToJson(task, true);
    sendRequest(endpoint, CreateIssue, payload, "POST");
}

/**
 * @brief Met à jour une issue GitLab
 * @param projectId ID du projet
 * @param issueIid IID de l'issue
 * @param task Tâche avec nouvelles données
 */
void GitLabConnector::updateIssue(const QString &projectId, int issueIid, GitIssueTask *task)
{
    QString endpoint = QString("/api/v4/projects/%1/issues/%2")
                       .arg(QString(QUrl::toPercentEncoding(projectId))).arg(issueIid);
    QJsonObject payload = taskToJson(task, false);
    sendRequest(endpoint, UpdateIssue, payload, "PUT");
}

/**
 * @brief Ferme une issue GitLab
 * @param projectId ID du projet
 * @param issueIid IID de l'issue
 */
void GitLabConnector::closeIssue(const QString &projectId, int issueIid)
{
    QString endpoint = QString("/api/v4/projects/%1/issues/%2")
                       .arg(QString(QUrl::toPercentEncoding(projectId))).arg(issueIid);
    QJsonObject payload;
    payload["state_event"] = "close";
    sendRequest(endpoint, CloseIssue, payload, "PUT");
}

/**
 * @brief Réouvre une issue fermée
 * @param projectId ID du projet
 * @param issueIid IID de l'issue
 */
void GitLabConnector::reopenIssue(const QString &projectId, int issueIid)
{
    QString endpoint = QString("/api/v4/projects/%1/issues/%2")
                       .arg(QString(QUrl::toPercentEncoding(projectId))).arg(issueIid);
    QJsonObject payload;
    payload["state_event"] = "reopen";
    sendRequest(endpoint, ReopenIssue, payload, "PUT");
}

void GitLabConnector::listIssues(const QString &projectId, const QString &state)
{
    QString endpoint = QString("/api/v4/projects/%1/issues?state=%2")
                       .arg(QString(QUrl::toPercentEncoding(projectId)), state);
    sendRequest(endpoint, ListIssues);
}

void GitLabConnector::searchIssues(const QString &query, const QString &projectId)
{
    QString endpoint;
    if (projectId.isEmpty()) {
        endpoint = QString("/api/v4/issues?search=%1").arg(QString(QUrl::toPercentEncoding(query)));
    } else {
        endpoint = QString("/api/v4/projects/%1/issues?search=%2")
                   .arg(QString(QUrl::toPercentEncoding(projectId)))
                   .arg(QString(QUrl::toPercentEncoding(query)));
    }
    sendRequest(endpoint, SearchIssues);
}

void GitLabConnector::sendRequest(const QString &endpoint, OperationType operation,
                                   const QJsonObject &payload, const QString &method)
{
    QUrl url(m_instanceUrl + endpoint);
    QNetworkRequest request(url);
    
    // Header d'authentification GitLab (optionnel pour repos publics)
    if (!m_token.isEmpty()) {
        request.setRawHeader("PRIVATE-TOKEN", m_token.toUtf8());
    }
    
    QNetworkReply *reply = nullptr;
    
    if (method == "GET") {
        reply = m_networkManager->get(request);
    } else if (method == "POST" || method == "PUT") {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact);
        
        if (method == "POST")
            reply = m_networkManager->post(request, data);
        else
            reply = m_networkManager->put(request, data);
    }
    
    if (reply) {
        m_pendingRequests[reply] = operation;
        if (operation == FetchIssue || operation == UpdateIssue || operation == CloseIssue || operation == ReopenIssue) {
            // Extraire le iid de l'issue du endpoint
            QRegularExpression re("/issues/(\\d+)");
            QRegularExpressionMatch match = re.match(endpoint);
            if (match.hasMatch()) {
                m_requestIssueIids[reply] = match.captured(1).toInt();
            }
        }
    }
}

void GitLabConnector::handleNetworkReply(QNetworkReply *reply)
{
    if (!reply)
        return;
    
    OperationType operation = m_pendingRequests.value(reply, FetchIssue);
    int issueIid = m_requestIssueIids.value(reply, 0);
    
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray responseData = reply->readAll();
    
    m_pendingRequests.remove(reply);
    m_requestIssueIids.remove(reply);
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = QString("Erreur réseau: %1").arg(reply->errorString());
        emit errorOccurred(errorMsg, statusCode);
        reply->deleteLater();
        return;
    }
    
    if (statusCode < 200 || statusCode >= 300) {
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QString errorMsg = doc.object()["message"].toString("Erreur API GitLab inconnue");
        emit errorOccurred(errorMsg, statusCode);
        reply->deleteLater();
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    
    switch (operation) {
        case FetchIssue:
            emit issueFetched(issueIid, doc.object());
            break;
            
        case CreateIssue: {
            QJsonObject obj = doc.object();
            int newIssueIid = obj["iid"].toInt();
            QString url = obj["web_url"].toString();
            emit issueCreated(newIssueIid, url);
            break;
        }
        
        case UpdateIssue:
            emit issueUpdated(issueIid);
            break;
            
        case CloseIssue:
            emit issueClosed(issueIid);
            break;
            
        case ReopenIssue:
            emit issueReopened(issueIid);
            break;
            
        case ListIssues:
            emit issuesListed(doc.array());
            break;
            
        case SearchIssues:
            emit issuesSearched(doc.array());
            break;
    }
    
    reply->deleteLater();
}

GitIssueTask* GitLabConnector::jsonToTask(const QJsonObject &json, QObject *parent)
{
    QString title = json["title"].toString();
    GitIssueTask *task = new GitIssueTask(title, parent);
    
    // Métadonnées de base
    task->setIssueNumber(json["iid"].toInt());  // GitLab utilise "iid" (internal id)
    task->setDescription(json["description"].toString());
    task->setRepositoryUrl(json["web_url"].toString().section("/-/issues/", 0, 0));
    task->setPlatform(GitPlatform::GitLab);
    
    // État
    QString state = json["state"].toString();
    if (state == "closed")
        task->setStatus(Status::COMPLETED);
    else
        task->setStatus(Status::NOTSTARTED);
    
    // Assignés
    QStringList assignees;
    for (const QJsonValue &val : json["assignees"].toArray()) {
        assignees << val.toObject()["username"].toString();
    }
    task->setAssignees(assignees);
    
    // Milestone
    QJsonObject milestone = json["milestone"].toObject();
    if (!milestone.isEmpty()) {
        task->setMilestone(milestone["title"].toString());
    }
    
    // Labels → Tags
    QStringList tags = json["labels"].toVariant().toStringList();
    task->setTags(tags);
    
    // Dates - Task n'a pas de creationDate, juste lastSyncDate pour la sync
    QString updatedAt = json["updated_at"].toString();
    if (!updatedAt.isEmpty()) {
        task->setLastSyncDate(QDateTime::fromString(updatedAt, Qt::ISODate));
    }
    
    task->clearLocallyModified();  // Marquer comme synchronisé
    
    return task;
}

QJsonObject GitLabConnector::taskToJson(const GitIssueTask *task, bool createMode)
{
    QJsonObject json;
    
    json["title"] = task->title();
    json["description"] = task->description();
    
    // État (GitLab utilise "state_event")
    if (!createMode) {
        if (task->status() == Status::COMPLETED)
            json["state_event"] = "close";
        else
            json["state_event"] = "reopen";
    }
    
    // Assignés (GitLab supporte assignee_ids, mais on utilise assignee_id pour simplicité)
    if (!task->assignees().isEmpty()) {
        // Pour simplifier, on n'assigne que le premier assignee
        // Dans une implémentation complète, il faudrait résoudre les usernames → user IDs
        json["assignee_id"] = task->assignees().first();
    }
    
    // Milestone (nécessite milestone_id, pas le titre)
    // Dans une implémentation complète, il faudrait résoudre milestone title → ID
    
    // Tags → Labels
    if (!task->tags().isEmpty()) {
        // GitLab attend une chaîne séparée par virgules
        json["labels"] = task->tags().join(",");
    }
    
    return json;
}

QString GitLabConnector::projectPathToId(const QString &path)
{
    // GitLab accepte "owner/repo" encodé en URL comme project ID
    return QString(QUrl::toPercentEncoding(path));
}
