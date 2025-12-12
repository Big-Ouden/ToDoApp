#include "pomodorotimer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QComboBox>
#include <QFont>
#include <QApplication>

PomodoroTimer::PomodoroTimer(QWidget *parent)
    : QWidget(parent),
      m_timer(new QTimer(this)),
      m_state(State::Idle),
      m_remainingSeconds(WORK_DURATION * 60),
      m_totalSeconds(WORK_DURATION * 60),
      m_completedPomodoros(0)
{
    // Layout principal
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Label d'état
    m_stateLabel = new QLabel(tr("Prêt à commencer"), this);
    m_stateLabel->setAlignment(Qt::AlignCenter);
    QFont stateFont = m_stateLabel->font();
    stateFont.setPointSize(12);
    stateFont.setBold(true);
    m_stateLabel->setFont(stateFont);
    mainLayout->addWidget(m_stateLabel);
    
    // Label temps
    m_timeLabel = new QLabel(formatTime(m_remainingSeconds), this);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    QFont timeFont = m_timeLabel->font();
    timeFont.setPointSize(48);
    timeFont.setBold(true);
    m_timeLabel->setFont(timeFont);
    mainLayout->addWidget(m_timeLabel);
    
    // Barre de progression
    m_progressBar = new QProgressBar(this);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(100);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(10);
    mainLayout->addWidget(m_progressBar);
    
    // Sélection de session
    QHBoxLayout *sessionLayout = new QHBoxLayout();
    QLabel *sessionLabel = new QLabel(tr("Type de session:"), this);
    sessionLayout->addWidget(sessionLabel);
    
    m_sessionCombo = new QComboBox(this);
    m_sessionCombo->addItem(tr("Pomodoro (25 min)"), WORK_DURATION);
    m_sessionCombo->addItem(tr("Pause courte (5 min)"), SHORT_BREAK);
    m_sessionCombo->addItem(tr("Pause longue (15 min)"), LONG_BREAK);
    m_sessionCombo->addItem(tr("Personnalisé (50 min)"), 50);
    sessionLayout->addWidget(m_sessionCombo);
    sessionLayout->addStretch();
    mainLayout->addLayout(sessionLayout);
    
    // Boutons de contrôle
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    m_startPauseButton = new QPushButton(tr("Démarrer"), this);
    m_startPauseButton->setMinimumHeight(40);
    QFont buttonFont = m_startPauseButton->font();
    buttonFont.setPointSize(11);
    buttonFont.setBold(true);
    m_startPauseButton->setFont(buttonFont);
    buttonLayout->addWidget(m_startPauseButton);
    
    m_resetButton = new QPushButton(tr("Réinitialiser"), this);
    m_resetButton->setMinimumHeight(40);
    buttonLayout->addWidget(m_resetButton);
    
    m_skipButton = new QPushButton(tr("Passer"), this);
    m_skipButton->setMinimumHeight(40);
    buttonLayout->addWidget(m_skipButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Label statistiques
    QLabel *statsLabel = new QLabel(tr("Pomodoros complétés: 0"), this);
    statsLabel->setObjectName("statsLabel");
    statsLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statsLabel);
    
    mainLayout->addStretch();
    
    // Connexions
    connect(m_timer, &QTimer::timeout, this, &PomodoroTimer::onTimerTick);
    connect(m_startPauseButton, &QPushButton::clicked, this, &PomodoroTimer::onStartPauseClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &PomodoroTimer::onResetClicked);
    connect(m_skipButton, &QPushButton::clicked, this, &PomodoroTimer::onSkipClicked);
    connect(m_sessionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (m_state == State::Idle) {
            int minutes = m_sessionCombo->itemData(index).toInt();
            m_remainingSeconds = minutes * 60;
            m_totalSeconds = m_remainingSeconds;
            updateDisplay();
        }
    });
}

void PomodoroTimer::setState(State newState)
{
    m_state = newState;
    
    switch (m_state) {
    case State::Idle:
        m_stateLabel->setText(tr("Prêt à commencer"));
        m_stateLabel->setStyleSheet("color: #666666;");
        break;
    case State::Working:
        m_stateLabel->setText(tr("🍅 Session de travail"));
        m_stateLabel->setStyleSheet("color: #d32f2f;");
        m_totalSeconds = WORK_DURATION * 60;
        m_remainingSeconds = m_totalSeconds;
        break;
    case State::ShortBreak:
        m_stateLabel->setText(tr("☕ Pause courte"));
        m_stateLabel->setStyleSheet("color: #388e3c;");
        m_totalSeconds = SHORT_BREAK * 60;
        m_remainingSeconds = m_totalSeconds;
        break;
    case State::LongBreak:
        m_stateLabel->setText(tr("🎉 Pause longue"));
        m_stateLabel->setStyleSheet("color: #1976d2;");
        m_totalSeconds = LONG_BREAK * 60;
        m_remainingSeconds = m_totalSeconds;
        break;
    }
    
    updateDisplay();
    emit stateChanged(newState);
}

void PomodoroTimer::updateDisplay()
{
    m_timeLabel->setText(formatTime(m_remainingSeconds));
    
    int progress = 0;
    if (m_totalSeconds > 0) {
        progress = ((m_totalSeconds - m_remainingSeconds) * 100) / m_totalSeconds;
    }
    m_progressBar->setValue(progress);
    
    // Mettre à jour les statistiques
    QLabel *statsLabel = findChild<QLabel*>("statsLabel");
    if (statsLabel) {
        statsLabel->setText(tr("Pomodoros complétés: %1").arg(m_completedPomodoros));
    }
}

QString PomodoroTimer::formatTime(int seconds) const
{
    int mins = seconds / 60;
    int secs = seconds % 60;
    return QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}

void PomodoroTimer::onTimerTick()
{
    m_remainingSeconds--;
    
    if (m_remainingSeconds <= 0) {
        m_timer->stop();
        
        // Session terminée
        if (m_state == State::Working) {
            m_completedPomodoros++;
            emit pomodoroCompleted();
            
            // Décider de la prochaine pause
            if (m_completedPomodoros % POMODOROS_BEFORE_LONG_BREAK == 0) {
                setState(State::LongBreak);
            } else {
                setState(State::ShortBreak);
            }
            
            // Notification
            QApplication::beep();
            
        } else {
            // Pause terminée
            emit breakCompleted();
            setState(State::Working);
            
            QApplication::beep();
        }
        
        m_startPauseButton->setText(tr("Démarrer"));
    }
    
    updateDisplay();
}

void PomodoroTimer::onStartPauseClicked()
{
    if (m_timer->isActive()) {
        // Pause
        m_timer->stop();
        m_startPauseButton->setText(tr("Reprendre"));
    } else {
        // Démarrage ou reprise
        if (m_state == State::Idle) {
            setState(State::Working);
        }
        m_timer->start(1000); // 1 seconde
        m_startPauseButton->setText(tr("Pause"));
    }
}

void PomodoroTimer::onResetClicked()
{
    m_timer->stop();
    setState(State::Idle);
    
    int minutes = m_sessionCombo->currentData().toInt();
    m_remainingSeconds = minutes * 60;
    m_totalSeconds = m_remainingSeconds;
    
    m_startPauseButton->setText(tr("Démarrer"));
    updateDisplay();
}

void PomodoroTimer::onSkipClicked()
{
    m_timer->stop();
    
    if (m_state == State::Working) {
        // Passer à la pause
        if (m_completedPomodoros % POMODOROS_BEFORE_LONG_BREAK == 0) {
            setState(State::LongBreak);
        } else {
            setState(State::ShortBreak);
        }
    } else {
        // Passer à la session de travail
        setState(State::Working);
    }
    
    m_startPauseButton->setText(tr("Démarrer"));
    updateDisplay();
}
