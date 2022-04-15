#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "taskexecutor.h"
#include "taskscreator.h"
#include "application.h"
#include "taskinfo.h"
#include "task.h"

#include <QTimer>
#include <QDebug>
#include <QLabel>
#include <QTabBar>
#include <QAction>
#include <QKeyEvent>
#include <QSettings>
#include <QGroupBox>
#include <QTextEdit>
#include <QDateTime>
#include <QTabWidget>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextDocument>
#include <QSignalBlocker>

#ifndef _WIN32
#include <csignal>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QWidget *spacer = new QWidget(ui->toolbar);
    m_create_taskslist_action = new QAction(QIcon(":/images/add.png"), tr("Создать список задач"), ui->toolbar);
    m_edit_taskslist_action = new QAction(QIcon(":/images/edit.png"), tr("Редактировать список задач"), ui->toolbar);
    m_choose_path_to_taskslist_action = new QAction(QIcon(":/images/folder.ico"), tr("Выбрать директорию со списком задач"), ui->toolbar);
    m_play_pause_action = new QAction(QIcon(":/images/play.png"), tr("Запустить"), ui->toolbar);
    m_quit_action = new QAction(QIcon(":/images/quit.png"), tr("Выход"), ui->file_menu);

    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_play_pause_action->setCheckable(true);
    m_play_pause_action->setChecked(false);
    m_play_pause_action->setStatusTip(tr("Запуск и остановка задач"));
    m_create_taskslist_action->setStatusTip(tr("Запуск формы для создания списка задач"));
    m_choose_path_to_taskslist_action->setStatusTip(tr("Выбор директории, в которой располагается файл \"%1\"").arg(TaskExecutor::taskslist_name()));

    ui->toolbar->addAction(m_create_taskslist_action);
    ui->toolbar->addAction(m_edit_taskslist_action);
    ui->toolbar->addSeparator();
    ui->toolbar->addAction(m_choose_path_to_taskslist_action);
    ui->toolbar->addSeparator();
    ui->toolbar->addWidget(spacer);
    ui->toolbar->addAction(m_play_pause_action);

    ui->file_menu->addAction(m_create_taskslist_action);
    ui->file_menu->addSeparator();
    ui->file_menu->addAction(m_quit_action);

    m_tabs = new QTabWidget(this);
    m_tabs->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    setCentralWidget(m_tabs);

    QStringList args = TaskExecutor::extra_args();
    if (!args.empty())
    {
        QLabel *args_lbl = new QLabel(tr("<b>Доп. аргументы:</b> %1").arg(TaskExecutor::extra_args().join(' ')), this);
        statusBar()->addPermanentWidget(args_lbl);
    }

    m_label = new QLabel(this);
    statusBar()->addPermanentWidget(m_label);

    QObject::connect(m_tabs->tabBar(), &QTabBar::customContextMenuRequested, this, &MainWindow::tab_bar_custom_context_menu_requested);
    QObject::connect(this, &MainWindow::need_update_permanent_widget, this, &MainWindow::update_permanent_widget, Qt::QueuedConnection);
    QObject::connect(m_create_taskslist_action, &QAction::triggered, this, &MainWindow::create_taskslist_action_triggered);
    QObject::connect(m_edit_taskslist_action, &QAction::triggered, this, &MainWindow::edit_taskslist_action_triggered);
    QObject::connect(m_choose_path_to_taskslist_action, &QAction::triggered, this, &MainWindow::choose_path_to_taskslist_action_triggered);
    QObject::connect(ui->save_output_action, &QAction::toggled, this, &MainWindow::save_output_action_toggled);
    QObject::connect(m_play_pause_action, &QAction::toggled, this, &MainWindow::play_pause_toggled);
    QObject::connect(m_quit_action, &QAction::triggered, this, &MainWindow::quit_action_triggered);
    QObject::connect(ui->autolaunch_action, &QAction::toggled, this, &MainWindow::auto_launch_toggled);
    QObject::connect(ui->reread_taskslist_action, &QAction::triggered, this, &MainWindow::ask_to_restart);

    QTimer::singleShot(0, this, &MainWindow::restore_settings);
}

MainWindow::~MainWindow()
{
    save_settings();
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_F5:
        ask_to_restart();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    save_settings();
    QMainWindow::closeEvent(event);
}

void MainWindow::task_started(const QString &task_name)
{
    for (int i = 0; i < m_tabs->count(); ++i)
    {
        QString task = m_tabs->tabText(i);
        if (prodisp::streq(task, task_name))
        {
            m_tabs->setTabIcon(i, QIcon(":/images/green.png"));
        }
    }
}

void MainWindow::task_finished(const QString &task_name, int exit_code)
{
    for (int i = 0; i < m_tabs->count(); ++i)
    {
        QString task = m_tabs->tabText(i);
        if (prodisp::streq(task, task_name))
        {
            QWidget *w = m_tabs->widget(i);
            if (QTextEdit *out = w->findChild<QTextEdit*>("output"))
            {
                out->append(QString("<b><font color=\"dark blue\">%1</font></b>: <b><font color=\"dark blue\">%2</font></b>")
                            .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                            .arg(tr("Процесс завершён с кодом \"%1\"").arg(exit_code)));
            }

            m_tabs->setTabIcon(i, QIcon(":/images/red.png"));
        }
    }
}

void MainWindow::task_failed_to_start(const QString &task_name)
{
    for (int i = 0; i < m_tabs->count(); ++i)
    {
        QString task = m_tabs->tabText(i);
        if (prodisp::streq(task, task_name))
        {
            QWidget *w = m_tabs->widget(i);
            if (QTextEdit *out = w->findChild<QTextEdit*>("output"))
            {
                out->append(QString("<b><font color=\"dark blue\">%1</font></b>: <b><font color=\"dark blue\">%2</font></b>")
                            .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                            .arg(tr("Невозможно запустить процесс")));
            }

            m_tabs->setTabIcon(i, QIcon(":/images/red.png"));
        }
    }
}

void MainWindow::stdout_msg(const QString &task_name, const QString &msg)
{
    if (!ui->save_output_action->isChecked())
        return;

    for (int i = 0; i < m_tabs->count(); ++i)
    {
        QString task = m_tabs->tabText(i);
        if (prodisp::streq(task, task_name))
        {
            QWidget *w = m_tabs->widget(i);
            if (QTextEdit *out = w->findChild<QTextEdit*>("output"))
            {
                out->append(QString("<b><font color=\"dark blue\">%1</font></b>: %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz")).arg(msg));
            }
        }
    }
}

void MainWindow::stderr_msg(const QString &task_name, const QString &msg)
{
    if (!ui->save_output_action->isChecked())
        return;

    for (int i = 0; i < m_tabs->count(); ++i)
    {
        QString task = m_tabs->tabText(i);
        if (prodisp::streq(task, task_name))
        {
            QWidget *w = m_tabs->widget(i);
            if (QTextEdit *out = w->findChild<QTextEdit*>("output"))
            {
                out->append(QString("<b><font color=\"dark blue\">%1</font></b>: <b><font color=\"red\">%2</font></b>").arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz")).arg(msg));
            }
        }
    }
}

void MainWindow::create_taskslist_action_triggered()
{
    TasksCreator tc(this);
    QObject::connect(&tc, &TasksCreator::ask_to_restart, this, &MainWindow::ask_to_restart, Qt::QueuedConnection);
    tc.exec();

    emit need_update_permanent_widget();
}

void MainWindow::edit_taskslist_action_triggered()
{
    emit need_edit_taskslist();
}

void MainWindow::edit_taskslist(const QList<TaskInfo> &info)
{
    TasksCreator tc(info, this);
    QObject::connect(&tc, &TasksCreator::update_taskslist, this, &MainWindow::update_taskslist, Qt::QueuedConnection);
    tc.exec();
}

void MainWindow::started()
{
    QSignalBlocker lock(m_play_pause_action);
    configure_ui(true);
    m_play_pause_action->setChecked(true);
    statusBar()->showMessage(tr("Задачи запущены..."), 5000);
}

void MainWindow::finished()
{
    QSignalBlocker lock(m_play_pause_action);;
    configure_ui(false);
    m_play_pause_action->setChecked(false);
    statusBar()->showMessage(tr("Задачи остановлены!"), 5000);
}

void MainWindow::play_pause_toggled(bool state)
{
    if (state)
    {
        emit start();
    }
    else
    {
        emit stop();
    }
}

void MainWindow::auto_launch_toggled(bool state)
{
    QSettings sett;
    sett.setValue("autolaunch", state);
}

void MainWindow::save_output_action_toggled(bool state)
{
    QSettings sett;
    sett.setValue("save_output", state);
}

void MainWindow::quit_action_triggered()
{
    save_settings();
    Application::quit();
}

void MainWindow::update_taskslist()
{
    emit restart(false);
    emit ask_for_info();
}

void MainWindow::tab_bar_custom_context_menu_requested(const QPoint &pos)
{
    int index = m_tabs->tabBar()->tabAt(pos);
    if (index < 0)
        return;

    QString task_name = m_tabs->tabText(index);

    QMenu menu(this);
#ifdef _WIN32
    QAction *kill = menu.addAction(tr("Завершить принудительно (TerminateProcess)"));
    if (menu.exec(m_tabs->tabBar()->mapToGlobal(pos)) != nullptr)
        emit send_signal(task_name, 1);
#else
    QMenu *send_signal_menu = menu.addMenu(tr("Отправить сигнал"));

    QAction *sigusr1 = send_signal_menu->addAction("SIGUSR1");
    QAction *sigusr2 = send_signal_menu->addAction("SIGUSR2");
    QAction *sigint = send_signal_menu->addAction("SIGINT");
    QAction *sigalrm = send_signal_menu->addAction("SIGALRM");

    QAction *a = menu.exec(m_tabs->tabBar()->mapToGlobal(pos));
    if (a == sigusr1)
        emit send_signal(task_name, SIGUSR1);
    else if (a == sigusr2)
        emit send_signal(task_name, SIGUSR2);
    else if (a == sigint)
        emit send_signal(task_name, SIGINT);
    else if (a == sigalrm)
        emit send_signal(task_name, SIGALRM);
#endif
}

void MainWindow::choose_path_to_taskslist_action_triggered()
{
    QSettings sett;

    QFileDialog fd;
    fd.setFileMode(QFileDialog::DirectoryOnly);
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setDirectory(sett.value("path", Application::applicationDirPath()).toString());
    fd.setWindowTitle(tr("Выбор директории"));

    if (fd.exec() == QDialog::Accepted)
    {
        QStringList dirs = fd.selectedFiles();
        if (!dirs.empty())
        {
            QString dir = dirs.at(0);
            sett.setValue("path", dir);

            ask_to_restart();
            emit need_update_permanent_widget();
        }
    }
}

void MainWindow::save_settings()
{
    QSettings sett;
    sett.setValue("geometry", saveGeometry());
}

void MainWindow::restore_settings()
{
    QSettings sett;
    if (sett.contains("geometry"))
    {
        restoreGeometry(sett.value("geometry").toByteArray());
    }

    if (sett.value("autolaunch", true).toBool())
    {
        ui->autolaunch_action->setChecked(true);
        m_play_pause_action->setChecked(true);
    }

    ui->save_output_action->setChecked(sett.value("save_output", true).toBool());

    emit ask_for_info();
    emit need_update_permanent_widget();
}

void MainWindow::ask_to_restart()
{
    if (prodisp::confirm_operation(tr("Перезагрузка"), tr("Перечитать файл \"%1\"?").arg(TaskExecutor::taskslist_name())))
    {
        emit restart(m_play_pause_action->isChecked());
        emit ask_for_info();
    }
}

void MainWindow::update_permanent_widget()
{
    QSettings sett;
    m_label->setText(tr("<b>Путь к \"%1\":</b> \"%2\"").arg(TaskExecutor::taskslist_name()).arg(sett.value("path", Application::applicationDirPath()).toString()));
}

void MainWindow::create_tabs(const QList<TaskInfo> &info)
{
    for (const TaskInfo &i : info)
    {
        QWidget *w = new QWidget(m_tabs);
        QGroupBox *box = new QGroupBox(tr("Вывод"), w);
        QTextEdit *output = new QTextEdit(box);
        QVBoxLayout *box_vl = new QVBoxLayout;
        output->setObjectName("output");
        box_vl->addWidget(output);
        box->setLayout(box_vl);
        output->document()->setMaximumBlockCount(1000); // Максимальное количество строк

        QFormLayout *f = new QFormLayout;
        f->setRowWrapPolicy(QFormLayout::DontWrapRows);
        //f->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
        f->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
        f->setLabelAlignment(Qt::AlignLeft);

        f->addRow(tr("<b>Исполняемый файл:</b> "), new QLabel(i.exe, w));
        f->addRow(tr("<b>Аргументы:</b> "), new QLabel(i.args, w));
        f->addRow(tr("<b>Рабочая директория:</b> "), new QLabel(i.pwd, w));
        f->addRow(tr("<b>Таймаут:</b> "), new QLabel(tr("%1 мс").arg(i.timeout), w));

        QVBoxLayout *vl = new QVBoxLayout;
        vl->addItem(f);
        vl->addWidget(box);
        w->setLayout(vl);

        m_tabs->addTab(w, QIcon(":/images/red.png"), i.name());
    }
}

void MainWindow::configure_ui(bool state)
{
    m_create_taskslist_action->setEnabled(!state);
    m_edit_taskslist_action->setEnabled(!state);
    m_choose_path_to_taskslist_action->setEnabled(!state);
    ui->reread_taskslist_action->setEnabled(!state);

    if (state)
    {
        m_play_pause_action->setText(tr("Остановить"));
        m_play_pause_action->setIcon(QIcon(":/images/stop.png"));
    }
    else
    {
        m_play_pause_action->setText(tr("Запустить"));
        m_play_pause_action->setIcon(QIcon(":/images/play.png"));
    }
}

void MainWindow::info_reply(const QList<TaskInfo> &info)
{
    m_tabs->clear();
    create_tabs(info);
}
