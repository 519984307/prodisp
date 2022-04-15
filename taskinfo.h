#ifndef TASKINFO_H
#define TASKINFO_H

#include "json.hpp"

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
            extra_args = other.extra_args;
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
            extra_args = other.extra_args;

            other.timeout = 0;
            other.extra_args = true;
        }

        return *this;
    }

    QString name() const noexcept { return QFileInfo(exe).baseName(); }

    static TaskInfo from_json_array_object_ordered(const jsoncons::ojson &array_value)
    {
        if (!array_value.is_object())
        {
            std::cerr << "array_value is not an object" << std::endl;
            return TaskInfo();
        }

        try
        {
            TaskInfo ti;
            ti.exe = array_value["exe"].as_string_view().data();
            ti.args = array_value["args"].as_string_view().data();
            ti.pwd = array_value["pwd"].as_string_view().data();
            ti.timeout = array_value["timeout"].as<uint64_t>();
            ti.extra_args = array_value["extra_args"].as_bool();
            return ti;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "from_json_array_ordered(): unknown exception" << std::endl;
        }

        return TaskInfo();
    }

    jsoncons::ojson to_json_array_object_ordered() const
    {
        jsoncons::ojson task_object(jsoncons::json_object_arg,
        {
            { "exe", exe.toStdString() },
            { "args", args.toStdString() },
            { "pwd", pwd.toStdString() },
            { "timeout", timeout },
            { "extra_args", extra_args }
        });

        return task_object;
    }

    QString exe; /* Путь к исполняемому файлу */
    QString args; /* Аргументы */
    QString pwd; /* Рабочая директория */
    uint64_t timeout = 0; /* Интервал перезапуска */
    bool extra_args = true; /* Признак передачи дополнительных аргументов */
};

#endif // TASKINFO_H
