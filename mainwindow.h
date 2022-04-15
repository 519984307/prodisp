#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QTabWidget;
struct TaskInfo;
namespace Ui { class MainWindow; }
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

public slots:
    void task_started(const QString &task_name);
    void task_finished(const QString &task_name, int exit_code);
    void task_failed_to_start(const QString &task_name);
    void stdout_msg(const QString &task_name, const QString &msg);
    void stderr_msg(const QString &task_name, const QString &msg);

    void info_reply(const QList<TaskInfo> &info);
    void edit_taskslist(const QList<TaskInfo> &info);

    void started();
    void finished();

private slots:
    void create_taskslist_action_triggered();
    void edit_taskslist_action_triggered();
    void choose_path_to_taskslist_action_triggered();
    void play_pause_toggled(bool state);
    void auto_launch_toggled(bool state);
    void save_output_action_toggled(bool state);
    void quit_action_triggered();
    void update_taskslist();
    void tab_bar_custom_context_menu_requested(const QPoint &pos);

private:
    void save_settings();
    void restore_settings();

    void ask_to_restart();
    void update_permanent_widget();

    void create_tabs(const QList<TaskInfo> &info);
    void configure_ui(bool state);

    Ui::MainWindow *ui;
    QTabWidget *m_tabs = nullptr;
    QLabel *m_label = nullptr;

    QAction *m_create_taskslist_action = nullptr;
    QAction *m_edit_taskslist_action = nullptr;
    QAction *m_choose_path_to_taskslist_action = nullptr;
    QAction *m_play_pause_action = nullptr;
    QAction *m_quit_action = nullptr;

signals:
    void start();
    void stop();

    void restart(bool run);
    void ask_for_info();
    void need_update_permanent_widget();
    void need_edit_taskslist();
    void send_signal(const QString &task_name, int sig);
};
#endif // MAINWINDOW_H
