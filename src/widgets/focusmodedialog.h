#ifndef FOCUSMODEDIALOG_H
#define FOCUSMODEDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QTime>

class Task;
class QLabel;
class QPushButton;
class QTextBrowser;

/**
 * @brief Mode Focus - Affiche une seule tâche en plein écran sans distractions.
 * Inclut un timer optionnel pour suivre le temps de travail.
 */
class FocusModeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FocusModeDialog(Task *task, QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onMarkCompleted();
    void onTakeBreak();
    void onExit();
    void updateTimer();

private:
    void setupUI();
    void updateTaskDisplay();

    Task *m_task;
    QLabel *m_titleLabel;
    QTextBrowser *m_descriptionBrowser;
    QLabel *m_timerLabel;
    QLabel *m_metaLabel;
    QPushButton *m_completeButton;
    QPushButton *m_breakButton;
    QPushButton *m_exitButton;
    
    QTimer *m_timer;
    QTime m_elapsedTime;
};

#endif // FOCUSMODEDIALOG_H
