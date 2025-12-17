#ifndef TASKDETAILWIDGET_H
#define TASKDETAILWIDGET_H

#pragma once

#include <QWidget>
#include <QCompleter>
#include <QListWidgetItem>
#include "task.h"

namespace Ui { class TaskDetailWidget; }

/// Modes d'affichage de la description
enum class ViewMode {
    TextOnly,      // Uniquement le texte éditable
    PreviewOnly,   // Uniquement l'aperçu (éditable via double-clic)
    Split          // Les deux côte à côte
};

/**
 * @brief Widget de détail pour éditer une tâche.
 */
class TaskDetailWidget : public QWidget
{
    Q_OBJECT

public:
    /** @brief Constructeur du widget */
    explicit TaskDetailWidget(QWidget *parent = nullptr);
    
    /** @brief Destructeur */
    ~TaskDetailWidget();

    /**
     * @brief Définit la tâche à afficher et éditer.
     * @param task Tâche à afficher
     */
    void setTask(Task *task);
    
    /** @brief Efface le contenu du widget */
    void clearTask();
    
    /** @brief Met à jour les traductions de l'interface */
    void updateTranslations();

signals:
    void taskModified(Task* task);

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool event(QEvent *event) override;

private slots:
    void onUserEdited();
    void onApplyClicked();
    void onTagsEditReturnPressed();
    void onTagItemDoubleClicked(QListWidgetItem *item);
    
    // Markdown toolbar
    void onBoldClicked();
    void onItalicClicked();
    void onCodeClicked();
    void onLinkClicked();
    void onListClicked();
    void onDescriptionTextChanged();
    void onPreviewDoubleClicked();
    
    // View mode
    void onViewModeChanged();
    
    // Attachments
    void onAddAttachmentClicked();
    void onAttachmentItemDoubleClicked(QListWidgetItem *item);

private:
    void updateComboTranslations();
    void updateTagsList();
    void updateAttachmentsList();
    void insertMarkdownFormat(const QString &prefix, const QString &suffix = "");
    void updateMarkdownPreview();
    void updateViewMode();
    
    Ui::TaskDetailWidget *ui;
    Task *m_task;
    QCompleter *m_tagsCompleter;
    ViewMode m_viewMode;
};


#endif // TASKDETAILWIDGET_H
