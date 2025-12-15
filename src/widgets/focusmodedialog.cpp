#include "focusmodedialog.h"
#include "task.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QKeyEvent>
#include <QFont>
#include <QTimer>

/**
 * @brief Constructeur du dialogue de mode focus
 * @param task Tâche à afficher en mode focus
 * @param parent Widget parent
 * @note Ouvre en plein écran avec un minuteur automatique
 */
FocusModeDialog::FocusModeDialog(Task *task, QWidget *parent)
    : QDialog(parent), m_task(task), m_elapsedTime(0, 0, 0)
{
    setupUI();
    updateTaskDisplay();
    
    // Timer pour suivre le temps passé
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &FocusModeDialog::updateTimer);
    m_timer->start(1000);  // Mise à jour chaque seconde
    
    // Maximiser la fenêtre
    showMaximized();
}

/**
 * @brief Configure l'interface utilisateur minimaliste
 * @note Applique un thème sombre pour réduire la fatigue oculaire
 */
void FocusModeDialog::setupUI()
{
    setWindowTitle(tr("Mode Focus"));
    setModal(true);
    
    // Style minimaliste avec fond sombre
    setStyleSheet(
        "QDialog { background-color: #2b2b2b; }"
        "QLabel { color: #ffffff; }"
        "QTextBrowser { background-color: #353535; color: #e0e0e0; border: none; }"
        "QPushButton { "
        "  background-color: #4CAF50; "
        "  color: white; "
        "  border: none; "
        "  padding: 15px 30px; "
        "  font-size: 16px; "
        "  border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton#breakButton { background-color: #FF9800; }"
        "QPushButton#breakButton:hover { background-color: #F57C00; }"
        "QPushButton#exitButton { background-color: #f44336; }"
        "QPushButton#exitButton:hover { background-color: #da190b; }"
    );
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    
    // Timer en haut
    m_timerLabel = new QLabel("00:00:00");
    m_timerLabel->setAlignment(Qt::AlignCenter);
    QFont timerFont;
    timerFont.setPointSize(24);
    timerFont.setBold(true);
    m_timerLabel->setFont(timerFont);
    mainLayout->addWidget(m_timerLabel);
    
    // Titre de la tâche
    m_titleLabel = new QLabel();
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setWordWrap(true);
    QFont titleFont;
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    mainLayout->addWidget(m_titleLabel);
    
    // Méta-informations (priorité, échéance)
    m_metaLabel = new QLabel();
    m_metaLabel->setAlignment(Qt::AlignCenter);
    QFont metaFont;
    metaFont.setPointSize(14);
    m_metaLabel->setFont(metaFont);
    mainLayout->addWidget(m_metaLabel);
    
    // Description
    m_descriptionBrowser = new QTextBrowser();
    m_descriptionBrowser->setReadOnly(true);
    QFont descFont;
    descFont.setPointSize(14);
    m_descriptionBrowser->setFont(descFont);
    mainLayout->addWidget(m_descriptionBrowser, 1);
    
    // Boutons d'action
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);
    
    m_completeButton = new QPushButton(tr("✓ Marquer comme terminé"));
    connect(m_completeButton, &QPushButton::clicked, this, &FocusModeDialog::onMarkCompleted);
    buttonLayout->addWidget(m_completeButton);
    
    m_breakButton = new QPushButton(tr("☕ Prendre une pause"));
    m_breakButton->setObjectName("breakButton");
    connect(m_breakButton, &QPushButton::clicked, this, &FocusModeDialog::onTakeBreak);
    buttonLayout->addWidget(m_breakButton);
    
    m_exitButton = new QPushButton(tr("← Quitter le mode Focus"));
    m_exitButton->setObjectName("exitButton");
    connect(m_exitButton, &QPushButton::clicked, this, &FocusModeDialog::onExit);
    buttonLayout->addWidget(m_exitButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Instructions en bas
    QLabel *instructions = new QLabel(tr("Appuyez sur ESC pour quitter"));
    instructions->setAlignment(Qt::AlignCenter);
    QFont instructionsFont;
    instructionsFont.setPointSize(10);
    instructionsFont.setItalic(true);
    instructions->setFont(instructionsFont);
    instructions->setStyleSheet("color: #999999;");  // Slightly lighter for better visibility
    mainLayout->addWidget(instructions);
}

void FocusModeDialog::updateTaskDisplay()
{
    if (!m_task) return;
    
    m_titleLabel->setText(m_task->title());
    
    // Méta-informations
    QString meta;
    QString priorityStr;
    switch (m_task->priority()) {
        case Priority::LOW: priorityStr = tr("🟢 Priorité basse"); break;
        case Priority::MEDIUM: priorityStr = tr("🟡 Priorité moyenne"); break;
        case Priority::HIGH: priorityStr = tr("🟠 Priorité haute"); break;
        case Priority::CRITICAL: priorityStr = tr("🔴 Priorité critique"); break;
    }
    meta += priorityStr;
    
    if (m_task->dueDate().isValid()) {
        meta += " • 📅 " + m_task->dueDate().toString("dd/MM/yyyy");
    }
    
    if (m_task->estimatedMinutes() > 0) {
        int hours = m_task->estimatedMinutes() / 60;
        int mins = m_task->estimatedMinutes() % 60;
        if (hours > 0) {
            meta += QString(" • ⏱ Estimé: %1h%2").arg(hours).arg(mins, 2, 10, QChar('0'));
        } else {
            meta += QString(" • ⏱ Estimé: %1 min").arg(mins);
        }
    }
    
    m_metaLabel->setText(meta);
    
    // Description avec support Markdown
    if (!m_task->description().isEmpty()) {
        m_descriptionBrowser->setMarkdown(m_task->description());
    } else {
        m_descriptionBrowser->setPlainText(tr("Aucune description"));
    }
}

void FocusModeDialog::updateTimer()
{
    m_elapsedTime = m_elapsedTime.addSecs(1);
    m_timerLabel->setText(m_elapsedTime.toString("hh:mm:ss"));
}

void FocusModeDialog::onMarkCompleted()
{
    if (!m_task) return;
    
    m_task->setStatus(Status::COMPLETED);
    
    // Ajouter le temps passé au temps réel
    int minutesSpent = (m_elapsedTime.hour() * 60) + m_elapsedTime.minute();
    m_task->addActualMinutes(minutesSpent);
    
    accept();
}

void FocusModeDialog::onTakeBreak()
{
    // Pause le timer
    if (m_timer->isActive()) {
        m_timer->stop();
        m_breakButton->setText(tr("▶ Reprendre le travail"));
    } else {
        m_timer->start(1000);
        m_breakButton->setText(tr("☕ Prendre une pause"));
    }
}

void FocusModeDialog::onExit()
{
    // Demander confirmation si du temps a été passé
    if (m_elapsedTime.hour() > 0 || m_elapsedTime.minute() > 0) {
        // Ajouter le temps passé
        int minutesSpent = (m_elapsedTime.hour() * 60) + m_elapsedTime.minute();
        if (m_task && minutesSpent > 0) {
            m_task->addActualMinutes(minutesSpent);
        }
    }
    
    reject();
}

void FocusModeDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        onExit();
    }
    
    QDialog::keyPressEvent(event);
}
