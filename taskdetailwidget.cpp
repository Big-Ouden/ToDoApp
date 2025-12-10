#include "taskdetailwidget.h"
#include "ui_taskdetailwidget.h"
#include <QEvent>
#include <QLocale>
#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QDesktopServices>

/**
 * @file taskdetailwidget.cpp
 * @brief Widget d'édition/visualisation d'une tâche.
 */

TaskDetailWidget::TaskDetailWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::TaskDetailWidget),
    m_task(nullptr),
    m_tagsCompleter(nullptr),
    m_viewMode(ViewMode::TextOnly)
{
    ui->setupUi(this);
    
    // Configure date avec locale français
    ui->dateEdit->setLocale(QLocale(QLocale::French, QLocale::France));
    ui->dateEdit->setDisplayFormat("dd/MM/yyyy");
    ui->dateEdit->setDate(QDate::currentDate());
    
    // Remplir les combos avec les traductions
    updateComboTranslations();
    
    // Configurer l'autocomplétion pour les tags (sera mise à jour dynamiquement)
    m_tagsCompleter = new QCompleter(this);
    m_tagsCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    ui->tagsEdit->setCompleter(m_tagsCompleter);
    
    // Connect signals from UI to propagate modifications
    connect(ui->titleEdit, &QLineEdit::textEdited, this, &TaskDetailWidget::onUserEdited);
    connect(ui->descEdit, &QTextEdit::textChanged, this, &TaskDetailWidget::onUserEdited);
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &TaskDetailWidget::onUserEdited);
    connect(ui->priorityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TaskDetailWidget::onUserEdited);
    connect(ui->statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TaskDetailWidget::onUserEdited);
    connect(ui->applyButton, &QPushButton::clicked, this, &TaskDetailWidget::onApplyClicked);
    
    // Gestion des tags
    connect(ui->tagsEdit, &QLineEdit::returnPressed, this, &TaskDetailWidget::onTagsEditReturnPressed);
    connect(ui->tagsListWidget, &QListWidget::itemDoubleClicked, this, &TaskDetailWidget::onTagItemDoubleClicked);
    
    // Markdown toolbar
    connect(ui->boldButton, &QToolButton::clicked, this, &TaskDetailWidget::onBoldClicked);
    connect(ui->italicButton, &QToolButton::clicked, this, &TaskDetailWidget::onItalicClicked);
    connect(ui->codeButton, &QToolButton::clicked, this, &TaskDetailWidget::onCodeClicked);
    connect(ui->linkButton, &QToolButton::clicked, this, &TaskDetailWidget::onLinkClicked);
    connect(ui->listButton, &QToolButton::clicked, this, &TaskDetailWidget::onListClicked);
    connect(ui->descEdit, &QTextEdit::textChanged, this, &TaskDetailWidget::onDescriptionTextChanged);
    
    // Installer un event filter pour détecter le double-clic sur l'aperçu
    ui->previewBrowser->installEventFilter(this);
    
    // View mode buttons
    connect(ui->textOnlyButton, &QToolButton::clicked, this, &TaskDetailWidget::onViewModeChanged);
    connect(ui->previewOnlyButton, &QToolButton::clicked, this, &TaskDetailWidget::onViewModeChanged);
    connect(ui->splitViewButton, &QToolButton::clicked, this, &TaskDetailWidget::onViewModeChanged);
    
    // Attachments
    connect(ui->addAttachmentButton, &QPushButton::clicked, this, &TaskDetailWidget::onAddAttachmentClicked);
    connect(ui->attachmentsListWidget, &QListWidget::itemDoubleClicked, this, &TaskDetailWidget::onAttachmentItemDoubleClicked);
    
    // Initialiser la vue
    updateViewMode();
}

TaskDetailWidget::~TaskDetailWidget()
{
    delete ui;
}

void TaskDetailWidget::setTask(Task *task)
{
    m_task = task;
    if (!m_task) {
        clearTask();
        return;
    }
    
    // Bloquer les signaux pendant la mise à jour
    ui->titleEdit->blockSignals(true);
    ui->descEdit->blockSignals(true);
    ui->dateEdit->blockSignals(true);
    ui->priorityCombo->blockSignals(true);
    ui->statusCombo->blockSignals(true);
    
    ui->titleEdit->setText(m_task->title());
    ui->descEdit->setPlainText(m_task->description());
    if (m_task->dueDate().isValid())
        ui->dateEdit->setDate(m_task->dueDate());
    else
        ui->dateEdit->setDate(QDate::currentDate());
    
    // Mettre à jour priorité
    ui->priorityCombo->setCurrentIndex(static_cast<int>(m_task->priority()));
    
    // Mettre à jour status
    ui->statusCombo->setCurrentIndex(static_cast<int>(m_task->status()));
    
    // Mettre à jour les tags
    updateTagsList();
    
    // Mettre à jour les attachments
    updateAttachmentsList();
    
    // Mettre à jour l'aperçu Markdown si activé
    if (m_viewMode != ViewMode::TextOnly) {
        updateMarkdownPreview();
    }
    
    // Débloquer les signaux
    ui->titleEdit->blockSignals(false);
    ui->descEdit->blockSignals(false);
    ui->dateEdit->blockSignals(false);
    ui->priorityCombo->blockSignals(false);
    ui->statusCombo->blockSignals(false);
}

void TaskDetailWidget::clearTask()
{
    ui->titleEdit->clear();
    ui->descEdit->clear();
    ui->dateEdit->setDate(QDate::currentDate());
    ui->tagsListWidget->clear();
    ui->tagsEdit->clear();
    ui->attachmentsListWidget->clear();
    m_task = nullptr;
}

void TaskDetailWidget::onUserEdited()
{
    // Cette fonction est appelée en temps réel mais n'applique pas les changements
    // Les changements sont appliqués uniquement via le bouton Appliquer
}

void TaskDetailWidget::onApplyClicked()
{
    if (!m_task) return;
    
    m_task->setTitle(ui->titleEdit->text());
    m_task->setDescription(ui->descEdit->toPlainText());
    m_task->setDueDate(ui->dateEdit->date());
    m_task->setPriority(static_cast<Priority>(ui->priorityCombo->currentIndex()));
    m_task->setStatus(static_cast<Status>(ui->statusCombo->currentIndex()));
    
    emit taskModified(m_task);
}

void TaskDetailWidget::updateTranslations()
{
    qDebug() << "TaskDetailWidget::updateTranslations() appelée";
    qDebug() << "  QLocale actuelle:" << QLocale().name();
    qDebug() << "  Traducteur français:" << qApp->translate("TaskDetailWidget", "Titre :");
    
    // Forcer la mise à jour manuelle des labels SANS tr() - utiliser directement les textes
    QLocale currentLocale = QLocale();
    if (currentLocale.language() == QLocale::French) {
        // Utiliser les textes français directement
        ui->labelTitle->setText("Titre :");
        ui->labelDesc->setText("Description :");
        ui->labelDate->setText("Échéance :");
        ui->labelPriority->setText("Priorité :");
        ui->labelStatus->setText("Statut :");
        ui->labelTags->setText("Étiquettes :");
        ui->labelAttachments->setText("Pièces jointes :");
        ui->applyButton->setText("Appliquer");
        ui->tagsEdit->setPlaceholderText("Ajouter une étiquette (Entrée pour valider)");
        ui->addAttachmentButton->setText("Ajouter un fichier");
    } else {
        // Utiliser les textes anglais directement
        ui->labelTitle->setText("Title:");
        ui->labelDesc->setText("Description:");
        ui->labelDate->setText("Due Date:");
        ui->labelPriority->setText("Priority:");
        ui->labelStatus->setText("Status:");
        ui->labelTags->setText("Tags:");
        ui->labelAttachments->setText("Attachments:");
        ui->applyButton->setText("Apply");
        ui->tagsEdit->setPlaceholderText("Add a tag (press Enter to validate)");
        ui->addAttachmentButton->setText("Add file");
    }
    
    qDebug() << "  labelTitle après setText:" << ui->labelTitle->text();
    qDebug() << "  labelDate après setText:" << ui->labelDate->text();
    
    // Mettre à jour les combos
    updateComboTranslations();
    
    // Mettre à jour le format de date selon la locale actuelle
    ui->dateEdit->setLocale(currentLocale);
    if (currentLocale.language() == QLocale::French) {
        ui->dateEdit->setDisplayFormat("dd/MM/yyyy");
    } else {
        ui->dateEdit->setDisplayFormat("MM/dd/yyyy");
    }
}

void TaskDetailWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        updateTranslations();
    }
    QWidget::changeEvent(event);
}

bool TaskDetailWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->previewBrowser && event->type() == QEvent::MouseButtonDblClick) {
        onPreviewDoubleClicked();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void TaskDetailWidget::updateComboTranslations()
{
    // Sauvegarder les index actuels
    int currentPriority = ui->priorityCombo->currentIndex();
    int currentStatus = ui->statusCombo->currentIndex();
    
    // Bloquer les signaux pendant la mise à jour
    ui->priorityCombo->blockSignals(true);
    ui->statusCombo->blockSignals(true);
    
    // Mettre à jour les items du combo Priorité
    ui->priorityCombo->clear();
    ui->priorityCombo->addItem(tr("Très faible"));
    ui->priorityCombo->addItem(tr("Faible"));
    ui->priorityCombo->addItem(tr("Moyenne"));
    ui->priorityCombo->addItem(tr("Élevée"));
    ui->priorityCombo->addItem(tr("Critique"));
    ui->priorityCombo->setCurrentIndex(currentPriority);
    
    // Mettre à jour les items du combo Statut
    ui->statusCombo->clear();
    ui->statusCombo->addItem(tr("Non démarrée"));
    ui->statusCombo->addItem(tr("En cours"));
    ui->statusCombo->addItem(tr("Complétée"));
    ui->statusCombo->addItem(tr("Annulée"));
    ui->statusCombo->setCurrentIndex(currentStatus);
    
    // Débloquer les signaux
    ui->priorityCombo->blockSignals(false);
    ui->statusCombo->blockSignals(false);
}

// ========================================
// Gestion des tags
// ========================================

void TaskDetailWidget::updateTagsList()
{
    ui->tagsListWidget->clear();
    
    if (!m_task) return;
    
    for (const QString &tag : m_task->tags()) {
        ui->tagsListWidget->addItem(tag);
    }
}

void TaskDetailWidget::onTagsEditReturnPressed()
{
    if (!m_task) return;
    
    QString newTag = ui->tagsEdit->text().trimmed();
    if (!newTag.isEmpty()) {
        m_task->addTag(newTag);
        updateTagsList();
        ui->tagsEdit->clear();
        emit taskModified(m_task);
    }
}

void TaskDetailWidget::onTagItemDoubleClicked(QListWidgetItem *item)
{
    if (!m_task || !item) return;
    
    QString tagToRemove = item->text();
    m_task->removeTag(tagToRemove);
    updateTagsList();
    emit taskModified(m_task);
}

// ========================================
// Support Markdown
// ========================================

void TaskDetailWidget::insertMarkdownFormat(const QString &prefix, const QString &suffix)
{
    QTextCursor cursor = ui->descEdit->textCursor();
    
    if (cursor.hasSelection()) {
        // Si du texte est sélectionné, l'entourer avec le format
        QString selectedText = cursor.selectedText();
        cursor.insertText(prefix + selectedText + suffix);
    } else {
        // Sinon, insérer le format avec un placeholder
        cursor.insertText(prefix + "texte" + suffix);
        // Sélectionner le placeholder
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, suffix.length() + 5);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 5);
    }
    
    ui->descEdit->setTextCursor(cursor);
    ui->descEdit->setFocus();
}

void TaskDetailWidget::onBoldClicked()
{
    insertMarkdownFormat("**", "**");
}

void TaskDetailWidget::onItalicClicked()
{
    insertMarkdownFormat("*", "*");
}

void TaskDetailWidget::onCodeClicked()
{
    insertMarkdownFormat("`", "`");
}

void TaskDetailWidget::onLinkClicked()
{
    QTextCursor cursor = ui->descEdit->textCursor();
    QString selectedText = cursor.selectedText();
    
    if (!selectedText.isEmpty()) {
        cursor.insertText("[" + selectedText + "](url)");
    } else {
        cursor.insertText("[texte](url)");
    }
    
    ui->descEdit->setTextCursor(cursor);
    ui->descEdit->setFocus();
}

void TaskDetailWidget::onListClicked()
{
    QTextCursor cursor = ui->descEdit->textCursor();
    cursor.insertText("- élément\n");
    ui->descEdit->setTextCursor(cursor);
    ui->descEdit->setFocus();
}

void TaskDetailWidget::onDescriptionTextChanged()
{
    if (m_viewMode != ViewMode::TextOnly) {
        updateMarkdownPreview();
    }
    onUserEdited();
}

void TaskDetailWidget::updateMarkdownPreview()
{
    QString markdown = ui->descEdit->toPlainText();
    
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    // Qt 5.14+ supporte setMarkdown
    ui->previewBrowser->setMarkdown(markdown);
#else
    // Fallback : affichage simple du HTML
    QString html = markdown;
    html.replace("\n", "<br>");
    html.replace("**", "<b>").replace("**", "</b>");  // Simplification très basique
    html.replace("*", "<i>").replace("*", "</i>");
    ui->previewBrowser->setHtml(html);
#endif
}

// ========================================
// Gestion des pièces jointes
// ========================================

void TaskDetailWidget::updateAttachmentsList()
{
    ui->attachmentsListWidget->clear();
    
    if (!m_task) return;
    
    for (const QUrl &url : m_task->attachments()) {
        QString displayName = url.isLocalFile() ? url.toLocalFile() : url.toString();
        QListWidgetItem *item = new QListWidgetItem(displayName);
        item->setData(Qt::UserRole, url);
        item->setToolTip(tr("Double-cliquer pour ouvrir"));
        ui->attachmentsListWidget->addItem(item);
    }
}

void TaskDetailWidget::onAddAttachmentClicked()
{
    if (!m_task) return;
    
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Sélectionner des fichiers à joindre"),
        QString(),
        tr("Tous les fichiers (*.*)")
    );
    
    for (const QString &file : files) {
        QUrl url = QUrl::fromLocalFile(file);
        m_task->addAttachment(url);
    }
    
    updateAttachmentsList();
}

void TaskDetailWidget::onAttachmentItemDoubleClicked(QListWidgetItem *item)
{
    QUrl url = item->data(Qt::UserRole).toUrl();
    QDesktopServices::openUrl(url);
}

// ========================================
// Gestion des modes de vue
// ========================================

void TaskDetailWidget::onViewModeChanged()
{
    if (sender() == ui->textOnlyButton) {
        m_viewMode = ViewMode::TextOnly;
    } else if (sender() == ui->previewOnlyButton) {
        m_viewMode = ViewMode::PreviewOnly;
    } else if (sender() == ui->splitViewButton) {
        m_viewMode = ViewMode::Split;
    }
    
    updateViewMode();
}

void TaskDetailWidget::updateViewMode()
{
    switch (m_viewMode) {
        case ViewMode::TextOnly:
            ui->descEdit->setVisible(true);
            ui->previewBrowser->setVisible(false);
            ui->textOnlyButton->setChecked(true);
            ui->previewOnlyButton->setChecked(false);
            ui->splitViewButton->setChecked(false);
            // Rendre la toolbar visible car on peut éditer
            ui->boldButton->setVisible(true);
            ui->italicButton->setVisible(true);
            ui->codeButton->setVisible(true);
            ui->linkButton->setVisible(true);
            ui->listButton->setVisible(true);
            break;
            
        case ViewMode::PreviewOnly:
            ui->descEdit->setVisible(false);
            ui->previewBrowser->setVisible(true);
            ui->textOnlyButton->setChecked(false);
            ui->previewOnlyButton->setChecked(true);
            ui->splitViewButton->setChecked(false);
            // Cacher la toolbar car on ne peut pas éditer directement
            ui->boldButton->setVisible(false);
            ui->italicButton->setVisible(false);
            ui->codeButton->setVisible(false);
            ui->linkButton->setVisible(false);
            ui->listButton->setVisible(false);
            updateMarkdownPreview();
            break;
            
        case ViewMode::Split:
            ui->descEdit->setVisible(true);
            ui->previewBrowser->setVisible(true);
            ui->textOnlyButton->setChecked(false);
            ui->previewOnlyButton->setChecked(false);
            ui->splitViewButton->setChecked(true);
            // Rendre la toolbar visible car on peut éditer
            ui->boldButton->setVisible(true);
            ui->italicButton->setVisible(true);
            ui->codeButton->setVisible(true);
            ui->linkButton->setVisible(true);
            ui->listButton->setVisible(true);
            updateMarkdownPreview();
            break;
    }
}

void TaskDetailWidget::onPreviewDoubleClicked()
{
    // Passer en mode texte pour éditer
    m_viewMode = ViewMode::TextOnly;
    updateViewMode();
    ui->descEdit->setFocus();
}
