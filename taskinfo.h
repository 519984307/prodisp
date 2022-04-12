#ifndef TASKINFO_H
#define TASKINFO_H

#include <QString>
#include <QFileInfo>

struct TaskInfo
{
    TaskInfo() = default;

    TaskInfo(const TaskInfo &other)
    {
        *this = other;
    }

    TaskInfo(TaskInfo &&other)
    {
        *this = std::move(other);
    }

    TaskInfo &operator=(const TaskInfo &other)
    {
        if (this != &other)
        {
            exe = other.exe;
            args = other.args;
            pwd = other.pwd;
            timeout = other.timeout;
        }

        return *this;
    }

    TaskInfo &operator=(TaskInfo &&other)
    {
        if (this != &other)
        {
            exe = std::move(other.exe);
            args = std::move(other.args);
            pwd = std::move(other.pwd);
            timeout = other.timeout;

            other.timeout = 0;
        }

        return *this;
    }

    QString name() const noexcept { return QFileInfo(exe).baseName(); }

    QString exe;
    QString args;
    QString pwd;
    uint64_t timeout = 0;
};

#endif // TASKINFO_H
