#include "taskexecutor.h"
#include "application.h"
#include "json.hpp"
#include "task.h"

#include <QFile>
#include <QDebug>
#include <QThread>
#include <QSettings>

TaskExecutor::TaskExecutor(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    qRegisterMetaType<TaskInfo>("TaskInfo");
    m_tasks_thread = new QThread(this);
}

TaskExecutor::~TaskExecutor()
{
    complete_quit();
}

QString TaskExecutor::taskslist_name() noexcept
{
    return QString("TasksList.json");
}

QString TaskExecutor::taskslist_path() noexcept
{
    QSettings sett;
    QString path = sett.value("path", Application::applicationDirPath()).toString();

    return QString("%1/%2").arg(path).arg(taskslist_name());
}

void TaskExecutor::start()
{
    m_tasks_thread->start();
}

void TaskExecutor::stop()
{
    m_tasks_thread->quit();
}

void TaskExecutor::restart(bool run)
{
    complete_quit();
    read_file();

    if (run)
        start();
}

void TaskExecutor::ask_for_info()
{
    QList<TaskInfo> info;
    for (Task *t : m_tasks)
    {
        info.push_back(t->info);
    }

    emit info_reply(info);
}

void TaskExecutor::need_edit_taskslist()
{
    QList<TaskInfo> info;
    for (Task *t : m_tasks)
    {
        info.push_back(t->info);
    }

    emit edit_taskslist(info);
}

void TaskExecutor::read_file()
{
    complete_quit();

    QFile f(taskslist_path());
    if (f.open(QIODevice::ReadOnly))
    {
        QByteArray data = f.readAll();
        parse_file(data);
    }
    else
    {
        std::cerr << "open(" << taskslist_path().toStdString() << ") failed" << std::endl;
    }
}

void TaskExecutor::complete_quit()
{
    m_tasks_thread->quit();
    m_tasks_thread->wait(1000);

    for (Task *t : m_tasks)
    {
        delete t;
    }

    m_tasks.clear();
}

void TaskExecutor::parse_file(const QByteArray &data)
{
    jsoncons::ojson doc;
    try
    {
        doc = jsoncons::ojson::parse(data.constBegin(), data.constEnd());
        if (!doc.contains("tasks"))
            throw std::runtime_error("parse_file(): does not contains field \"tasks\"");

        const jsoncons::ojson &t = doc["tasks"];
        for (const auto &item : t.array_range())
        {
            Task *t = Task::from_json_array_object_ordered(item);
            if (t)
            {
                t->moveToThread(m_tasks_thread);
                QObject::connect(m_tasks_thread, &QThread::started, t, &Task::start);
                QObject::connect(m_tasks_thread, &QThread::finished, t, &Task::stop);

                QObject::connect(t, &Task::task_started, this, &TaskExecutor::task_started);
                QObject::connect(t, &Task::task_finished, this, &TaskExecutor::task_finished);
                QObject::connect(t, &Task::task_failed_to_start, this, &TaskExecutor::task_failed_to_start);
                QObject::connect(t, &Task::stdout_msg, this, &TaskExecutor::stdout_msg);
                QObject::connect(t, &Task::stderr_msg, this, &TaskExecutor::stderr_msg);

                m_tasks.push_back(t);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "parse_file(): unknown exception" << std::endl;
    }
}
