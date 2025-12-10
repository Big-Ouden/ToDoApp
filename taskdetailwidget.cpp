#include "taskdetailwidget.h"
#include "ui_taskdetailwidget.h"
#include <QEvent>
#include <QLocale>
#include <QApplication>
#include <QDebug>

/**
 * @file taskdetailwidget.cpp
 * @brief Widget d'édition/visualisation d'une tâche.
 */

TaskDetailWidget::TaskDetailWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::TaskDetailWidget),
    m_task(nullptr)
{
    ui->setupUi(this);
    
    // Configure date avec locale français
    ui->dateEdit->setLocale(QLocale(QLocale::French, QLocale::France));
    ui->dateEdit->setDisplayFormat("dd/MM/yyyy");
    ui->dateEdit->setDate(QDate::currentDate());
    
    // Remplir les combos avec les traductions
    updateComboTranslations();
    
    // Connect signals from UI to propagate modifications
    connect(ui->titleEdit, &QLineEdit::textEdited, this, &TaskDetailWidget::onUserEdited);
    connect(ui->descEdit, &QTextEdit::textChanged, this, &TaskDetailWidget::onUserEdited);
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &TaskDetailWidget::onUserEdited);
    connect(ui->priorityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TaskDetailWidget::onUserEdited);
    connect(ui->statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TaskDetailWidget::onUserEdited);
    connect(ui->applyButton, &QPushButton::clicked, this, &TaskDetailWidget::onApplyClicked);
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
        ui->applyButton->setText("Appliquer");
    } else {
        // Utiliser les textes anglais directement
        ui->labelTitle->setText("Title:");
        ui->labelDesc->setText("Description:");
        ui->labelDate->setText("Due Date:");
        ui->labelPriority->setText("Priority:");
        ui->labelStatus->setText("Status:");
        ui->applyButton->setText("Apply");
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
