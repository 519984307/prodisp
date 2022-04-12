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
    TaskExecutor(QObject *parent = nullptr);
    ~TaskExecutor();

    static QString taskslist_name() noexcept;
    static QString taskslist_path() noexcept;

public slots:
    void start();
    void stop();
    void restart(bool run);
    void ask_for_info();
    void need_edit_taskslist();

    void read_file();

private slots:

private:
    void complete_quit();
    void parse_file(const QByteArray &data);

    QThread *m_tasks_thread = nullptr;
    QList<Task*> m_tasks;

signals:
    void task_started(const QString &task_name);
    void task_finished(const QString &task_name, int exit_code);
    void task_failed_to_start(const QString &task_name);
    void stdout_msg(const QString &task_name, const QString &msg);
    void stderr_msg(const QString &task_name, const QString &msg);

    void info_reply(const QList<TaskInfo> &info);
    void edit_taskslist(const QList<TaskInfo> &info);
};

#endif // TASKEXECUTOR_H
