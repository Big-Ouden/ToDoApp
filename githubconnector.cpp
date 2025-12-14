#include "githubconnector.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QDateTime>
#include <QUrlQuery>
#include <QRegularExpression>

/**
 * @brief Constructeur du connecteur GitHub
 * @param parent Objet parent Qt
 */
GitHubConnector::GitHubConnector(QObject *parent)
    : QObject(parent),
      m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &GitHubConnector::handleNetworkReply);
}

GitHubConnector::~GitHubConnector()
{
}

/**
 * @brief Définit le token d'authentification
 * @param token Personal Access Token GitHub
 * @note Augmente la limite de 60 à 5000 requêtes/heure
 */
void GitHubConnector::setToken(const QString &token)
{
    if (m_token != token) {
        m_token = token;
        emit tokenChanged();
        emit authenticatedChanged();
    }
}

/**
 * @brief Récupère une issue GitHub
 * @param owner Propriétaire du dépôt
 * @param repo Nom du dépôt
 * @param issueNumber Numéro de l'issue
 */
void GitHubConnector::fetchIssue(const QString &owner, const QString &repo, int issueNumber)
{
    QString endpoint = QString("/repos/%1/%2/issues/%3").arg(owner, repo).arg(issueNumber);
    sendRequest(endpoint, FetchIssue);
}

/**
 * @brief Crée une nouvelle issue sur GitHub
 * @param owner Propriétaire du dépôt
 * @param repo Nom du dépôt
 * @param task Tâche source à convertir en issue
 */
void GitHubConnector::createIssue(const QString &owner, const QString &repo, GitIssueTask *task)
{
    QString endpoint = QString("/repos/%1/%2/issues").arg(owner, repo);
    QJsonObject payload = taskToJson(task, true);
    sendRequest(endpoint, CreateIssue, payload, "POST");
}

/**
 * @brief Met à jour une issue existante
 * @param owner Propriétaire du dépôt
 * @param repo Nom du dépôt
 * @param issueNumber Numéro de l'issue
 * @param task Tâche avec nouvelles données
 */
void GitHubConnector::updateIssue(const QString &owner, const QString &repo, int issueNumber, GitIssueTask *task)
{
    QString endpoint = QString("/repos/%1/%2/issues/%3").arg(owner, repo).arg(issueNumber);
    QJsonObject payload = taskToJson(task, false);
    sendRequest(endpoint, UpdateIssue, payload, "PATCH");
}

/**
 * @brief Ferme une issue GitHub
 * @param owner Propriétaire du dépôt
 * @param repo Nom du dépôt
 * @param issueNumber Numéro de l'issue
 */
void GitHubConnector::closeIssue(const QString &owner, const QString &repo, int issueNumber)
{
    QString endpoint = QString("/repos/%1/%2/issues/%3").arg(owner, repo).arg(issueNumber);
    QJsonObject payload;
    payload["state"] = "closed";
    sendRequest(endpoint, CloseIssue, payload, "PATCH");
}

/**
 * @brief Réouvre une issue fermée
 * @param owner Propriétaire du dépôt
 * @param repo Nom du dépôt
 * @param issueNumber Numéro de l'issue
 */
void GitHubConnector::reopenIssue(const QString &owner, const QString &repo, int issueNumber)
{
    QString endpoint = QString("/repos/%1/%2/issues/%3").arg(owner, repo).arg(issueNumber);
    QJsonObject payload;
    payload["state"] = "open";
    sendRequest(endpoint, ReopenIssue, payload, "PATCH");
}

/**
 * @brief Liste les issues d'un dépôt
 * @param owner Propriétaire du dépôt
 * @param repo Nom du dépôt
 * @param state Filtre d'état ("open", "closed", "all")
 */
void GitHubConnector::listIssues(const QString &owner, const QString &repo, const QString &state)
{
    qDebug() << "[GitHubConnector::listIssues] START - owner:" << owner << "repo:" << repo << "state:" << state;
    QString endpoint = QString("/repos/%1/%2/issues?state=%3").arg(owner, repo, state);
    qDebug() << "[GitHubConnector::listIssues] Endpoint:" << endpoint;
    sendRequest(endpoint, ListIssues);
    qDebug() << "[GitHubConnector::listIssues] Request sent";
}

/**
 * @brief Recherche des issues par requête
 * @param query Requête de recherche GitHub (syntaxe GitHub Search)
 */
void GitHubConnector::searchIssues(const QString &query)
{
    QString endpoint = QString("/search/issues?q=%1").arg(QString(QUrl::toPercentEncoding(query)));
    sendRequest(endpoint, SearchIssues);
}

/**
 * @brief Envoie une requête HTTP à l'API GitHub
 * @param endpoint Point de terminaison API (ex: /repos/owner/repo/issues)
 * @param operation Type d'opération effectuée
 * @param payload Données JSON pour POST/PATCH
 * @param method Méthode HTTP (GET, POST, PATCH)
 * @note Ajoute automatiquement les headers requis par GitHub
 */
void GitHubConnector::sendRequest(const QString &endpoint, OperationType operation,
                                   const QJsonObject &payload, const QString &method)
{
    qDebug() << "[GitHubConnector::sendRequest] START - endpoint:" << endpoint << "operation:" << operation << "method:" << method;
    QUrl url(m_apiBaseUrl + endpoint);
    qDebug() << "[GitHubConnector::sendRequest] URL:" << url.toString();
    QNetworkRequest request(url);
    
    // Headers requis par GitHub API
    request.setRawHeader("Accept", "application/vnd.github+json");
    
    // Token optionnel - sans token: 60 req/h, avec token: 5000 req/h
    if (!m_token.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_token).toUtf8());
    }
    
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    
    QNetworkReply *reply = nullptr;
    
    if (method == "GET") {
        reply = m_networkManager->get(request);
    } else if (method == "POST" || method == "PATCH") {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact);
        
        if (method == "POST")
            reply = m_networkManager->post(request, data);
        else
            reply = m_networkManager->sendCustomRequest(request, "PATCH", data);
    }
    
    if (reply) {
        qDebug() << "[GitHubConnector::sendRequest] Reply created, connecting finished signal";
        m_pendingRequests[reply] = operation;
        if (operation == FetchIssue || operation == UpdateIssue || operation == CloseIssue || operation == ReopenIssue) {
            // Extraire le numéro d'issue du endpoint
            QRegularExpression re("/issues/(\\d+)");
            QRegularExpressionMatch match = re.match(endpoint);
            if (match.hasMatch()) {
                m_requestIssueNumbers[reply] = match.captured(1).toInt();
            }
        }
        qDebug() << "[GitHubConnector::sendRequest] Request added to pending list, count:" << m_pendingRequests.size();
    } else {
        qDebug() << "[GitHubConnector::sendRequest] ERROR: reply is nullptr!";
    }
}

void GitHubConnector::handleNetworkReply(QNetworkReply *reply)
{
    qDebug() << "[GitHubConnector::handleNetworkReply] START";
    qDebug() << "[GitHubConnector::handleNetworkReply] Reply:" << reply;
    
    if (!reply) {
        qDebug() << "[GitHubConnector::handleNetworkReply] ERROR: reply is nullptr";
        return;
    }
    qDebug() << "[GitHubConnector::handleNetworkReply] Reply received from:" << reply->url().toString();
    
    OperationType operation = m_pendingRequests.value(reply, FetchIssue);
    int issueNumber = m_requestIssueNumbers.value(reply, 0);
    
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray responseData = reply->readAll();
    qDebug() << "[GitHubConnector::handleNetworkReply] Status code:" << statusCode << "operation:" << operation;
    qDebug() << "[GitHubConnector::handleNetworkReply] Response size:" << responseData.size() << "bytes";
    
    m_pendingRequests.remove(reply);
    m_requestIssueNumbers.remove(reply);
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = QString("Erreur réseau: %1").arg(reply->errorString());
        qDebug() << "[GitHubConnector::handleNetworkReply] Network error:" << errorMsg;
        emit errorOccurred(errorMsg, statusCode);
        reply->deleteLater();
        return;
    }
    
    if (statusCode < 200 || statusCode >= 300) {
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QString errorMsg = doc.object()["message"].toString("Erreur API GitHub inconnue");
        qDebug() << "[GitHubConnector::handleNetworkReply] HTTP error:" << statusCode << errorMsg;
        emit errorOccurred(errorMsg, statusCode);
        reply->deleteLater();
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    qDebug() << "[GitHubConnector::handleNetworkReply] Parsing JSON, operation:" << operation;
    
    switch (operation) {
        case FetchIssue:
            qDebug() << "[GitHubConnector::handleNetworkReply] Emitting issueFetched for issue" << issueNumber;
            emit issueFetched(issueNumber, doc.object());
            break;
            
        case CreateIssue: {
            QJsonObject obj = doc.object();
            int newIssueNumber = obj["number"].toInt();
            QString url = obj["html_url"].toString();
            emit issueCreated(newIssueNumber, url);
            break;
        }
        
        case UpdateIssue:
            emit issueUpdated(issueNumber);
            break;
            
        case CloseIssue:
            emit issueClosed(issueNumber);
            break;
            
        case ReopenIssue:
            emit issueReopened(issueNumber);
            break;
            
        case ListIssues:
            qDebug() << "[GitHubConnector::handleNetworkReply] Emitting issuesListed with" << doc.array().size() << "issues";
            emit issuesListed(doc.array());
            break;
            
        case SearchIssues: {
            QJsonArray items = doc.object()["items"].toArray();
            emit issuesSearched(items);
            break;
        }
    }
    
    reply->deleteLater();
}

GitIssueTask* GitHubConnector::jsonToTask(const QJsonObject &json, QObject *parent)
{
    QString title = json["title"].toString();
    GitIssueTask *task = new GitIssueTask(title, parent);
    
    // Métadonnées de base
    task->setIssueNumber(json["number"].toInt());
    task->setDescription(json["body"].toString());
    task->setRepositoryUrl(json["repository_url"].toString());
    
    // État
    QString state = json["state"].toString();
    if (state == "closed")
        task->setStatus(Status::COMPLETED);
    else
        task->setStatus(Status::NOTSTARTED);
    
    // Assignés
    QStringList assignees;
    for (const QJsonValue &val : json["assignees"].toArray()) {
        assignees << val.toObject()["login"].toString();
    }
    task->setAssignees(assignees);
    
    // Milestone
    QJsonObject milestone = json["milestone"].toObject();
    if (!milestone.isEmpty()) {
        task->setMilestone(milestone["title"].toString());
    }
    
    // Labels → Tags
    QStringList tags;
    for (const QJsonValue &val : json["labels"].toArray()) {
        tags << val.toObject()["name"].toString();
    }
    task->setTags(tags);
    
    // Dates - Task n'a pas de creationDate, juste lastSyncDate pour la sync
    QString updatedAt = json["updated_at"].toString();
    if (!updatedAt.isEmpty()) {
        task->setLastSyncDate(QDateTime::fromString(updatedAt, Qt::ISODate));
    }
    
    task->clearLocallyModified();  // Marquer comme synchronisé
    
    return task;
}

QJsonObject GitHubConnector::taskToJson(const GitIssueTask *task, bool createMode)
{
    QJsonObject json;
    
    json["title"] = task->title();
    json["body"] = task->description();
    
    // État (uniquement pour update)
    if (!createMode) {
        if (task->status() == Status::COMPLETED)
            json["state"] = "closed";
        else
            json["state"] = "open";
    }
    
    // Assignés
    QJsonArray assignees;
    for (const QString &assignee : task->assignees()) {
        assignees.append(assignee);
    }
    if (!assignees.isEmpty()) {
        json["assignees"] = assignees;
    }
    
    // Milestone
    if (!task->milestone().isEmpty()) {
        json["milestone"] = task->milestone();
    }
    
    // Tags → Labels
    QJsonArray labels;
    for (const QString &tag : task->tags()) {
        labels.append(tag);
    }
    if (!labels.isEmpty()) {
        json["labels"] = labels;
    }
    
    return json;
}
