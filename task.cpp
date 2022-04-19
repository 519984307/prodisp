#include "task.h"
#include "taskexecutor.h"

#include <iostream>

#include <QDebug>
#include <QTimer>
#include <QFileInfo>

Task::Task(QObject *parent)
    : QObject(parent)
{

}

Task::Task(const QString &exe, const QString &args, const QString &pwd, uint64_t timeout, QObject *parent)
    : QObject(parent)
{
    info.exe = exe;
    info.args = args;
    info.pwd = pwd;
    info.timeout = timeout;
}

Task::~Task()
{

}

void Task::start()
{
    stop();

    QStringList extra_args = TaskExecutor::extra_args();
    QStringList args = info.args.isEmpty() ? QStringList() : info.args.split(' ');
    if (!extra_args.empty() && info.extra_args)
        args += extra_args;

    m_timer = new QTimer;
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->setSingleShot(true);
    QObject::connect(m_timer, &QTimer::timeout, this, &Task::timer_timeout);

    m_process = new QProcess;

    QObject::connect(m_process, &QProcess::started, this, &Task::started);
    QObject::connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &Task::finished);
    QObject::connect(m_process, &QProcess::readyReadStandardOutput, this, &Task::readyReadStandardOutput);
    QObject::connect(m_process, &QProcess::readyReadStandardError, this, &Task::readyReadStandardError);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QObject::connect(m_process, &QProcess::errorOccurred, this, &Task::errorOccurred);
#endif

    m_process->setWorkingDirectory(info.pwd);
    m_process->setArguments(args);
    m_process->setProgram(info.exe);

    m_process->start();
    if (!m_process->waitForStarted(1000))
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
        emit task_failed_to_start(name());
#endif
    }
}

void Task::stop()
{
    if (m_process)
    {
        m_process->kill();
        m_process->waitForFinished();

        delete m_process;
        m_process = nullptr;
    }

    if (m_timer)
    {
        m_timer->stop();
        delete m_timer;
        m_timer = nullptr;
    }
}

void Task::started()
{
    emit task_started(name());
}

void Task::readyReadStandardOutput()
{
    QString msg(m_process->readAllStandardOutput());
    emit stdout_msg(name(), msg);
}

void Task::readyReadStandardError()
{
    QString msg(m_process->readAllStandardError());
    emit stderr_msg(name(), msg);
}

void Task::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit)
        std::cerr << name().toStdString() << " crashed" << std::endl;

    emit task_finished(name(), exitCode);
    m_timer->start(info.timeout);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
void Task::errorOccurred(QProcess::ProcessError error)
{
    switch (error)
    {
    case QProcess::FailedToStart:
        emit task_failed_to_start(name());
        break;
    default:
        break;
    }
}
#endif

void Task::timer_timeout()
{
    if (!m_process)
    {
        std::cerr << "process (" << name().toStdString() << ") == nullptr" << std::endl;
        return;
    }

    m_process->start();
}

Task *Task::from_json_array_object_ordered(const jsoncons::ojson &array_value)
{
    if (!array_value.is_object())
    {
        std::cerr << "array_value is not an object" << std::endl;
        return nullptr;
    }

    try
    {
        std::unique_ptr<Task> ptr(new Task);
        ptr->info.name = array_value["name"].as_string_view().data();
        ptr->info.exe = array_value["exe"].as_string_view().data();
        ptr->info.args = array_value["args"].as_string_view().data();
        ptr->info.pwd = array_value["pwd"].as_string_view().data();
        ptr->info.timeout = array_value["timeout"].as<uint64_t>();
        ptr->info.extra_args = array_value["extra_args"].as_bool();
        return ptr.release();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "from_json_array_ordered(): unknown exception" << std::endl;
    }

    return nullptr;
}

jsoncons::ojson Task::to_json_array_object_ordered() const
{
    return info.to_json_array_object_ordered();
}

QString Task::name() const noexcept
{
    return info.name;
}

qint64 Task::pid() const noexcept
{
    return (m_process ? m_process->processId() : 0);
}
