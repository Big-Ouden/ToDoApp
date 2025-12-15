#ifndef GITPROJECTWIDGET_H
#define GITPROJECTWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include "repositorymanager.h"
#include "gitissuetask.h"

namespace Ui {
class GitProjectWidget;
}

/**
 * @brief Widget pour gérer les projets Git et leurs issues
 * 
 * Affiche la liste des dépôts configurés, les issues de chaque dépôt,
 * et permet la synchronisation bidirectionnelle.
 */
class GitProjectWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit GitProjectWidget(QWidget *parent = nullptr);
    ~GitProjectWidget();
    
    void setRepositoryManager(RepositoryManager *manager);
    RepositoryManager* repositoryManager() const { return m_repositoryManager; }
    
    // Issues actuellement affichées
    QList<GitIssueTask*> displayedIssues() const { return m_displayedIssues; }
    GitIssueTask* selectedIssue() const;
    
signals:
    void issueSelected(GitIssueTask *issue);
    void taskCreatedFromIssue(GitIssueTask *issue);
    void syncRequested(GitRepository *repo);
    void repositoryAdded(GitRepository *repo);
    void repositoryModified(GitRepository *repo);
    void repositoryDeleted(GitRepository *repo);
    
private slots:
    void onRepositoryChanged(int index);
    void onAddRepository();
    void onEditRepository();
    void onRemoveRepository();
    void onSyncRepository();
    
    void onFilterChanged();
    void onSearchTextChanged(const QString &text);
    void onIssueSelectionChanged();
    void onIssueDoubleClicked(QListWidgetItem *item);
    
    void onCreateIssue();
    void onCreateTaskFromIssue();
    
    // Slots pour les réponses API
    void onIssuesListed(const QJsonArray &issues);
    void onIssueCreated(int issueNumber, const QString &url);
    void onApiError(const QString &error, int httpCode);
    
private:
    void updateRepositoryCombo();
    void updateRepositoryInfo();
    void loadIssuesForCurrentRepository();
    void refreshIssuesList();
    void connectRepositorySignals();
    
    Ui::GitProjectWidget *ui;
    RepositoryManager *m_repositoryManager;
    QList<GitIssueTask*> m_allIssues;
    QList<GitIssueTask*> m_displayedIssues;
    bool m_loading;
};

#endif // GITPROJECTWIDGET_H
