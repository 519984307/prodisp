#ifndef TASK_H
#define TASK_H

#include <QDebug>
#include <QString>
#include <QObject>
#include <QProcess>

#include "json.hpp"
#include "taskinfo.h"
#include "functions.h"

class QTimer;

class Task : public QObject
{
    Q_OBJECT
public:
    Task(QObject *parent = nullptr);
    Task(const QString &exe, const QString &args, const QString &pwd, uint64_t timeout, QObject *parent = nullptr);
    ~Task();

    Task(const Task &other) = delete;
    Task(Task &&other) = delete;
    Task &operator=(const Task &other) = delete;
    Task &operator=(Task &&other) = delete;

    friend bool operator==(const Task &p1, const Task &p2)
    {
        return prodisp::streq(p1.info.exe, p2.info.exe);
    }

    friend bool operator!=(const Task &p1, const Task &p2)
    {
        return !(p1 == p2);
    }

#ifndef QT_NO_DEBUG_STREAM
friend QDebug operator<<(QDebug debug, const Task &p)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug.noquote();
    debug << "Task: ";
    debug << "name=\"" << p.name() << "\", ";
    debug << "exe=\"" << p.info.exe << "\", ";
    debug << "args=\"" << p.info.args << "\", ";
    debug << "pwd=\"" << p.info.pwd << "\", ";
    debug << "timeout=" << p.info.timeout << ".";
    return debug;
}
#endif // !QT_NO_DEBUG_STREAM

    static Task * from_json_array_object_ordered(const jsoncons::ojson &array_value);
    jsoncons::ojson to_json_array_object_ordered() const;

    QString name() const noexcept;

    const QString &exe() const noexcept { return info.exe; }
    const QString &exe(const QString &new_value) noexcept { info.exe = new_value; return info.exe; }

    const QString &args() const noexcept { return info.args; }
    const QString &args(const QString &new_value) noexcept { info.args = new_value; return info.args; }

    const QString &pwd() const noexcept { return info.pwd; }
    const QString &pwd(const QString &new_value) noexcept { info.pwd = new_value; return info.pwd; }

    const uint64_t &timeout() const noexcept { return info.timeout; }
    const uint64_t &timeout(const uint64_t &new_value) noexcept { info.timeout = new_value; return info.timeout; }

    qint64 pid() const noexcept;

public slots:
    void start();
    void stop();

private slots:
    void started();
    void readyReadStandardOutput();
    void readyReadStandardError();
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    void errorOccurred(QProcess::ProcessError error);
#endif

    void timer_timeout();

private:
    friend class TaskExecutor;

    TaskInfo info;

    QProcess *m_process = nullptr;
    QTimer *m_timer = nullptr;

signals:
    void task_started(const QString &task_name);
    void task_finished(const QString &task_name, int exit_code);
    void task_failed_to_start(const QString &task_name);
    void stdout_msg(const QString &task_name, const QString &msg);
    void stderr_msg(const QString &task_name, const QString &msg);
};

#endif // TASK_H
