#ifndef TASKEXECUTOR_H
#define TASKEXECUTOR_H

#include <QObject>

class Task;
class QThread;
struct TaskInfo;

class TaskExecutor : public QObject
{
    Q_OBJECT
public:
    TaskExecutor(const QStringList &extra_args = QStringList(), QObject *parent = nullptr);
    ~TaskExecutor();

    TaskExecutor(const TaskExecutor&) = delete;
    TaskExecutor(TaskExecutor&&) = delete;
    TaskExecutor &operator=(const TaskExecutor&) = delete;
    TaskExecutor &operator=(TaskExecutor&&) = delete;

    static QStringList extra_args() noexcept;
    static QString taskslist_name() noexcept;
    static QString taskslist_path() noexcept;

public slots:
    void start();
    void stop();
    void restart(bool run);
    void ask_for_info();
    void need_edit_taskslist();
    void send_signal(const QString &task_name, int sig);

    void read_file();

private slots:

private:
    static TaskExecutor *self;

    void complete_quit();
    void parse_file(const QByteArray &data);

    QThread *m_tasks_thread = nullptr;
    QList<Task*> m_tasks;

    QStringList m_extra_args;

signals:
    void task_started(const QString &task_name);
    void task_finished(const QString &task_name, int exit_code);
    void task_failed_to_start(const QString &task_name);
    void stdout_msg(const QString &task_name, const QString &msg);
    void stderr_msg(const QString &task_name, const QString &msg);

    void info_reply(const QList<TaskInfo> &info);
    void edit_taskslist(const QList<TaskInfo> &info);

    void started();
    void finished();
};

#endif // TASKEXECUTOR_H
