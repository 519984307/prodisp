#include "application.h"
#include "mainwindow.h"
#include "taskexecutor.h"

#include <iostream>

#include <QSettings>
#include <QCommandLineParser>

#ifndef _WIN32
#include <csignal>

void signal_handler(int sig)
{
    switch (sig)
    {
    case SIGINT:
    case SIGTERM:
    case SIGTSTP:
        std::cout << "finish (" << sig << ")" << std::endl;
        Application::quit();
        break;
    case SIGSEGV:
        std::cerr << "segmentation fault" << std::endl;
        std::exit(EXIT_FAILURE);
        break;
    }
}

#endif // _WIN32

int main(int argc, char *argv[])
{
#ifndef _WIN32
    if (std::signal(SIGINT, signal_handler) == SIG_ERR)
        std::cerr << "Can not set signal handler for SIGINT!" << std::endl;
    if (std::signal(SIGTERM, signal_handler) == SIG_ERR)
        std::cerr << "Can not set signal handler for SIGTERM!" << std::endl;
    if (std::signal(SIGTSTP, signal_handler) == SIG_ERR)
        std::cerr << "Can not set signal handler for SIGTSTP!" << std::endl;
    if (std::signal(SIGSEGV, signal_handler) == SIG_ERR)
        std::cerr << "Can not set signal handler for SIGSEGV!" << std::endl;
#endif

    Application a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Программное обеспечение диспетчеризации запуска программ. Сборка: %1").arg(PRODISPVERSION));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption taskslist_path_option(QStringList() << "p" << "path", QCoreApplication::translate("main", "<path> to TasksList.json"), QCoreApplication::translate("main", "path"));
    parser.addOption(taskslist_path_option);

    QCommandLineOption extra_args_option(QStringList() << "a" << "arg", QCoreApplication::translate("main", "<arg> passes to each task"), QCoreApplication::translate("main", "pass extra argument to all processes"));
    parser.addOption(extra_args_option);

    QCommandLineOption console_option(QStringList() << "c" << "console", QCoreApplication::translate("main", "run without UI"));
    parser.addOption(console_option);

    parser.process(a);

    QString taskslist_path = parser.value(taskslist_path_option);
    QStringList extra_args = parser.values(extra_args_option);
    bool console = parser.isSet(console_option);

    if (!taskslist_path.isEmpty())
    {
        QSettings sett;
        sett.setValue("path", taskslist_path);
    }

    TaskExecutor executor(extra_args);
    executor.read_file();

    if (console)
    {
        executor.start();
    }
    else
    {
        MainWindow *w = new MainWindow;
        QObject::connect(&executor, &TaskExecutor::task_started, w, &MainWindow::task_started);
        QObject::connect(&executor, &TaskExecutor::task_finished, w, &MainWindow::task_finished);
        QObject::connect(&executor, &TaskExecutor::task_failed_to_start, w, &MainWindow::task_failed_to_start);
        QObject::connect(&executor, &TaskExecutor::stdout_msg, w, &MainWindow::stdout_msg);
        QObject::connect(&executor, &TaskExecutor::stderr_msg, w, &MainWindow::stderr_msg);
        QObject::connect(&executor, &TaskExecutor::info_reply, w, &MainWindow::info_reply);
        QObject::connect(&executor, &TaskExecutor::edit_taskslist, w, &MainWindow::edit_taskslist);
        QObject::connect(&executor, &TaskExecutor::started, w, &MainWindow::started);
        QObject::connect(&executor, &TaskExecutor::finished, w, &MainWindow::finished);
        QObject::connect(w, &MainWindow::start, &executor, &TaskExecutor::start);
        QObject::connect(w, &MainWindow::stop, &executor, &TaskExecutor::stop);
        QObject::connect(w, &MainWindow::restart, &executor, &TaskExecutor::restart);
        QObject::connect(w, &MainWindow::ask_for_info, &executor, &TaskExecutor::ask_for_info);
        QObject::connect(w, &MainWindow::need_edit_taskslist, &executor, &TaskExecutor::need_edit_taskslist);
        QObject::connect(w, &MainWindow::send_signal, &executor, &TaskExecutor::send_signal);
        w->show();
    }

    const int r = a.exec();
    executor.stop();
    return r;
}
