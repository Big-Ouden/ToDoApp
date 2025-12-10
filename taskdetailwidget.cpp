#include "taskdetailwidget.h"
#include "ui_taskdetailwidget.h"
#include <QEvent>

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
    // Connect signals from UI to propagate modifications
    connect(ui->titleEdit, &QLineEdit::textEdited, this, &TaskDetailWidget::onUserEdited);
    connect(ui->descEdit, &QTextEdit::textChanged, this, &TaskDetailWidget::onUserEdited);
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &TaskDetailWidget::onUserEdited);
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
    ui->titleEdit->setText(m_task->title());
    ui->descEdit->setPlainText(m_task->description());
    if (m_task->dueDate().isValid())
        ui->dateEdit->setDate(m_task->dueDate());
    else
        ui->dateEdit->setDate(QDate::currentDate());
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
    if (!m_task) return;
    m_task->setTitle(ui->titleEdit->text());
    m_task->setDescription(ui->descEdit->toPlainText());
    m_task->setDueDate(ui->dateEdit->date());
    emit taskModified();
}

void TaskDetailWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
    QWidget::changeEvent(event);
}
