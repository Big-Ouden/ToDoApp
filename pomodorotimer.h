#ifndef POMODOROTIMER_H
#define POMODOROTIMER_H

#include <QWidget>
#include <QTimer>
#include <QTime>

class QLabel;
class QPushButton;
class QProgressBar;
class QComboBox;

/**
 * @brief Widget Timer Pomodoro avec sessions de travail et pauses
 */
class PomodoroTimer : public QWidget
{
    Q_OBJECT

public:
    enum class State {
        Idle,       // En attente
        Working,    // Session de travail
        ShortBreak, // Pause courte
        LongBreak   // Pause longue
    };

    explicit PomodoroTimer(QWidget *parent = nullptr);
    ~PomodoroTimer() override = default;

    /** @return État actuel du timer */
    State currentState() const { return m_state; }
    
    /** @return Temps restant en secondes */
    int remainingSeconds() const { return m_remainingSeconds; }

signals:
    void pomodoroCompleted();
    void breakCompleted();
    void stateChanged(State newState);

private slots:
    void onTimerTick();
    void onStartPauseClicked();
    void onResetClicked();
    void onSkipClicked();

private:
    void setState(State newState);
    void updateDisplay();
    void playNotificationSound();
    QString formatTime(int seconds) const;
    
    QTimer *m_timer;
    QLabel *m_timeLabel;
    QLabel *m_stateLabel;
    QPushButton *m_startPauseButton;
    QPushButton *m_resetButton;
    QPushButton *m_skipButton;
    QProgressBar *m_progressBar;
    QComboBox *m_sessionCombo;
    
    State m_state;
    int m_remainingSeconds;
    int m_totalSeconds;
    int m_completedPomodoros;
    
    // Durées par défaut (en minutes)
    static const int WORK_DURATION = 25;
    static const int SHORT_BREAK = 5;
    static const int LONG_BREAK = 15;
    static const int POMODOROS_BEFORE_LONG_BREAK = 4;
};

#endif // POMODOROTIMER_H
