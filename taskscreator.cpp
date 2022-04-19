#include "taskscreator.h"
#include "application.h"
#include "taskexecutor.h"
#include "task.h"

#include <QSet>
#include <QLabel>
#include <QTimer>
#include <QSpinBox>
#include <QTimeLine>
#include <QLineEdit>
#include <QSettings>
#include <QCheckBox>
#include <QTabWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QMessageBox>

TasksCreator::TasksCreator(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , edit_mode(false)
{
    create_ui();
    setWindowTitle(tr("Создание списка задач"));

    QTimer::singleShot(0, this, &TasksCreator::restore_settings);
}

TasksCreator::TasksCreator(const QList<TaskInfo> &info, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , edit_mode(true)
{
    create_ui();
    setWindowTitle(tr("Редактирование списка задач"));

    for (const TaskInfo &t : info)
    {
        m_task_tabs->addTab(create_tab(t), t.name);
    }

    QTimer::singleShot(0, this, &TasksCreator::restore_settings);
}

TasksCreator::~TasksCreator()
{
    save_settings();
}

void TasksCreator::add_task_button_clicked()
{
    int index = m_task_tabs->addTab(create_tab(), tr("Процесс %1").arg(m_task_tabs->count() + 1));
    m_task_tabs->setCurrentIndex(index);
}

void TasksCreator::remove_task_button_clicked()
{
    if (m_task_tabs->count() < 1)
        return;

    if (!prodisp::confirm_operation(tr("Удаление задачи"), tr("Удалить вкладку \"%1\"?").arg(m_task_tabs->tabText(m_task_tabs->currentIndex())), this))
        return;

    m_task_tabs->removeTab(m_task_tabs->currentIndex());
}

void TasksCreator::clone_task_button_clicked()
{
    if (m_task_tabs->count() < 1)
        return;

    TaskInfo info = get_taskinfo(m_task_tabs->currentIndex());
    int index = m_task_tabs->addTab(create_tab(info), info.name);

    m_task_tabs->setCurrentIndex(index);
    flash_edit(m_task_tabs->widget(index), "name");
}

void TasksCreator::save_button_clicked()
{
    if (m_task_tabs->count() < 1)
    {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не созданы задачи"));
        return;
    }

    QSet<QString> unique_names;

    QList<TaskInfo> tasks;
    for (int i = 0; i < m_task_tabs->count(); ++i)
    {
        TaskInfo info = get_taskinfo(i);

        QWidget *w = m_task_tabs->widget(i);
        if (check_taskinfo(info, unique_names, w))
        {
            tasks.push_back(info);
            unique_names.insert(info.name);
        }
        else
        {
            m_task_tabs->setCurrentIndex(i);
            return;
        }
    }

    if (save_to_file(tasks))
    {
        if (edit_mode)
            emit update_taskslist();

        accept();
    }
    else
    {
        QMessageBox::warning(this, tr("Ошибка"), tr("Невозможно записать файл"));
    }
}

void TasksCreator::cancel_button_clicked()
{
    reject();
}

void TasksCreator::choose_exe_file(QLineEdit *exe_edit, QLineEdit *pwd_edit, QLineEdit *name_edit)
{
    QString pwd = pwd_edit->text();

    QFileDialog fd;
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setDirectory(pwd.isEmpty() ? Application::applicationDirPath() : pwd);
    fd.setWindowTitle(tr("Выбор исполняемого файла"));

    if (fd.exec() == QDialog::Accepted)
    {
        QStringList dirs = fd.selectedFiles();
        if (!dirs.empty())
        {
            QString path = dirs.at(0);
            exe_edit->setText(path);

            if (name_edit->text().isEmpty())
                name_edit->setText(QFileInfo(path).baseName());
        }
    }
}

void TasksCreator::choose_pwd_dir(QLineEdit *pwd_edit, QLineEdit *exe_edit)
{
    QString exe = exe_edit->text();

    QFileDialog fd;
    fd.setFileMode(QFileDialog::DirectoryOnly);
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setDirectory(exe.isEmpty() ? Application::applicationDirPath() : QFileInfo(exe).absolutePath());
    fd.setWindowTitle(tr("Выбор рабочей директории"));

    if (fd.exec() == QDialog::Accepted)
    {
        QStringList dirs = fd.selectedFiles();
        if (!dirs.empty())
        {
            pwd_edit->setText(dirs.at(0));
        }
    }
}

void TasksCreator::name_text_changed(const QString &text, int index)
{
    m_task_tabs->setTabText(index, text);
}

void TasksCreator::create_ui()
{
    if (layout())
        delete layout();

    QSize icon_size(20, 20);

    m_task_tabs = new QTabWidget(this);
    m_add_task_button = new QToolButton(this);
    m_remove_task_button = new QToolButton(this);
    m_clone_task_button = new QToolButton(this);
    m_save_button = new QPushButton(tr("Сохранить"), this);
    m_cancel_button = new QPushButton(tr("Отмена"), this);

    m_clone_task_button->setIcon(QIcon(":/images/clone.png"));
    m_clone_task_button->setText(tr("Клонировать задачу"));
    m_clone_task_button->setToolTip(tr("Клонировать задачу"));
    m_clone_task_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_clone_task_button->setArrowType(Qt::NoArrow);
    m_clone_task_button->setIconSize(icon_size);

    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_add_task_button->setIcon(QIcon(":/images/add.png"));
    m_add_task_button->setText(tr("Добавить задачу"));
    m_add_task_button->setToolTip(tr("Добавить задачу"));
    m_add_task_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_add_task_button->setArrowType(Qt::NoArrow);
    m_add_task_button->setIconSize(icon_size);
    m_remove_task_button->setIcon(QIcon(":/images/remove.png"));
    m_remove_task_button->setText(tr("Удалить задачу"));
    m_remove_task_button->setToolTip(tr("Удалить задачу"));
    m_remove_task_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_remove_task_button->setArrowType(Qt::NoArrow);
    m_remove_task_button->setIconSize(icon_size);

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(m_clone_task_button);
    hl->addWidget(spacer);
    hl->addWidget(m_add_task_button);
    hl->addWidget(m_remove_task_button);

    QHBoxLayout *hl_btn = new QHBoxLayout;
    QWidget *btn_spacer = new QWidget(this);
    btn_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hl_btn->setSpacing(3);
    hl_btn->addWidget(btn_spacer);
    hl_btn->addWidget(m_save_button);
    hl_btn->addWidget(m_cancel_button);

    QVBoxLayout *vl = new QVBoxLayout;
    vl->addItem(hl);
    vl->addWidget(m_task_tabs);
    vl->addItem(hl_btn);

    setLayout(vl);
    setMinimumSize(240, 320);
    setMaximumSize(768, 1024);

    QObject::connect(m_add_task_button, &QToolButton::clicked, this, &TasksCreator::add_task_button_clicked);
    QObject::connect(m_remove_task_button, &QToolButton::clicked, this, &TasksCreator::remove_task_button_clicked);
    QObject::connect(m_clone_task_button, &QToolButton::clicked, this, &TasksCreator::clone_task_button_clicked);
    QObject::connect(m_save_button, &QPushButton::clicked, this, &TasksCreator::save_button_clicked);
    QObject::connect(m_cancel_button, &QPushButton::clicked, this, &TasksCreator::cancel_button_clicked);
}

QWidget *TasksCreator::create_tab()
{
    QWidget *w = new QWidget;

    QFormLayout *form = new QFormLayout;
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    form->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    form->setLabelAlignment(Qt::AlignLeft);

    QWidget *name_widget = new QWidget;
    QLineEdit *name_edit = new QLineEdit(name_widget);
    QHBoxLayout *name_hl = new QHBoxLayout;
    name_edit->setObjectName("name");
    name_edit->setMinimumHeight(20);
    name_hl->addWidget(name_edit);
    name_widget->setLayout(name_hl);
    form->addRow(tr("Название: "), name_widget);

    QWidget *exe_widget = new QWidget;
    QLineEdit *exe_edit = new QLineEdit(exe_widget);
    QToolButton *exe_choose = new QToolButton(exe_widget);
    QHBoxLayout *exe_hl = new QHBoxLayout;
    exe_hl->setSpacing(3);
    exe_edit->setObjectName("exe");
    exe_edit->setMinimumHeight(20);
    exe_choose->setMinimumHeight(20);
    exe_choose->setToolButtonStyle(Qt::ToolButtonIconOnly);
    exe_choose->setIcon(QIcon(":/images/folder.ico"));
    exe_choose->setToolTip(tr("Указать путь к исполняемому файлу"));
    exe_hl->addWidget(exe_edit);
    exe_hl->addWidget(exe_choose);
    exe_widget->setLayout(exe_hl);
    form->addRow(tr("Исполняемый файл: "), exe_widget);

    QWidget *args_widget = new QWidget;
    QLineEdit *args_edit = new QLineEdit(args_widget);
    QHBoxLayout *args_hl = new QHBoxLayout;
    args_edit->setObjectName("args");
    args_edit->setMinimumHeight(20);
    args_hl->addWidget(args_edit);
    args_widget->setLayout(args_hl);
    form->addRow(tr("Аргументы: "), args_widget);

    QWidget *pwd_widget = new QWidget;
    QLineEdit *pwd_edit = new QLineEdit(pwd_widget);
    QToolButton *pwd_choose = new QToolButton(pwd_widget);
    QHBoxLayout *pwd_hl = new QHBoxLayout;
    pwd_edit->setObjectName("pwd");
    pwd_edit->setMinimumHeight(20);
    pwd_choose->setMinimumHeight(20);
    pwd_choose->setToolButtonStyle(Qt::ToolButtonIconOnly);
    pwd_choose->setIcon(QIcon(":/images/folder.ico"));
    pwd_choose->setToolTip(tr("Указать рабочую директорию"));
    pwd_hl->setSpacing(3);
    pwd_hl->addWidget(pwd_edit);
    pwd_hl->addWidget(pwd_choose);
    pwd_widget->setLayout(pwd_hl);
    form->addRow(tr("Рабочая директория: "), pwd_widget);

    QWidget *timeout_widget = new QWidget;
    QSpinBox *timeout_spin = new QSpinBox(timeout_widget);
    QHBoxLayout *timeout_hl = new QHBoxLayout;
    timeout_spin->setObjectName("timeout");
    timeout_spin->setMinimumHeight(20);
    timeout_spin->setToolTip(tr("Интервал перезапуска задачи при завершении"));
    timeout_spin->setRange(10, 3600 * 1000); // Миллисекунды
    timeout_spin->setValue(1000);
    timeout_spin->setSuffix(tr(" мс"));
    timeout_hl->addWidget(timeout_spin);
    timeout_widget->setLayout(timeout_hl);
    form->addRow(tr("Таймаут: "), timeout_widget);

    QWidget *extra_args_widget = new QWidget;
    QCheckBox *extra_args_checkbox = new QCheckBox(extra_args_widget);
    QHBoxLayout *extra_args_hl = new QHBoxLayout;
    extra_args_checkbox->setObjectName("extra_args");
    extra_args_checkbox->setToolTip(tr("Признак передачи доп. аргументов"));
    extra_args_checkbox->setChecked(true);
    extra_args_hl->addWidget(extra_args_checkbox);
    extra_args_widget->setLayout(extra_args_hl);
    form->addRow(tr("Доп. аргументы: "), extra_args_widget);

    QObject::connect(name_edit, &QLineEdit::textChanged, this, std::bind(&TasksCreator::name_text_changed, this, std::placeholders::_1, m_task_tabs->count()));
    QObject::connect(exe_choose, &QToolButton::clicked, this, std::bind(&TasksCreator::choose_exe_file, this, exe_edit, pwd_edit, name_edit));
    QObject::connect(pwd_choose, &QToolButton::clicked, this, std::bind(&TasksCreator::choose_pwd_dir, this, pwd_edit, exe_edit));

    if (w->layout())
        delete w->layout();

    w->setLayout(form);
    return w;
}

QWidget *TasksCreator::create_tab(const TaskInfo &info)
{
    QWidget *w = create_tab();
    if (QLineEdit *l = w->findChild<QLineEdit*>("name"))
        l->setText(info.name);
    if (QLineEdit *l = w->findChild<QLineEdit*>("exe"))
        l->setText(info.exe);
    if (QLineEdit *l = w->findChild<QLineEdit*>("args"))
        l->setText(info.args);
    if (QLineEdit *l = w->findChild<QLineEdit*>("pwd"))
        l->setText(info.pwd);
    if (QSpinBox *s = w->findChild<QSpinBox*>("timeout"))
        s->setValue(info.timeout);
    if (QCheckBox *c = w->findChild<QCheckBox*>("extra_args"))
        c->setChecked(info.extra_args);
    return w;
}

QString TasksCreator::create_json(const QList<TaskInfo> &tasks)
{
    try
    {
        jsoncons::ojson tasks_array(jsoncons::json_array_arg);
        for (const TaskInfo &t : tasks)
        {
            tasks_array.push_back(t.to_json_array_object_ordered());
        }

        jsoncons::ojson doc;
        doc["tasks"] = std::move(tasks_array);

        std::ostringstream os;
        os << jsoncons::pretty_print(doc) << std::endl;

        return QString::fromStdString(os.str());
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "create_json(): unknown exception" << std::endl;
    }

    return QString();
}

TaskInfo TasksCreator::get_taskinfo(int index)
{
    TaskInfo info;
    QWidget *w = m_task_tabs->widget(index);
    if (QLineEdit *l = w->findChild<QLineEdit*>("name"))
        info.name = l->text().simplified();
    if (QLineEdit *l = w->findChild<QLineEdit*>("exe"))
        info.exe = l->text().simplified();
    if (QLineEdit *l = w->findChild<QLineEdit*>("args"))
        info.args = l->text().simplified();
    if (QLineEdit *l = w->findChild<QLineEdit*>("pwd"))
        info.pwd = l->text().simplified();
    if (QSpinBox *s = w->findChild<QSpinBox*>("timeout"))
        info.timeout = static_cast<uint64_t>(s->value());
    if (QCheckBox *c = w->findChild<QCheckBox*>("extra_args"))
        info.extra_args = c->isChecked();
    return info;
}

bool TasksCreator::check_taskinfo(const TaskInfo &info, const QSet<QString> &unique_names, QWidget *w)
{
    if (info.name.isEmpty() || unique_names.contains(info.name))
    {
        flash_edit(w, "name");
        return false;
    }

    if (info.exe.isEmpty())
    {
        flash_edit(w, "exe");
        return false;
    }

    return true;
}

void TasksCreator::flash_edit(QWidget *w, const QString &name)
{
    QLineEdit *edit = w->findChild<QLineEdit*>(name);
    if (!edit)
    {
        std::cerr << "findChild(" << name.toStdString() << ") failed" << std::endl;
        return;
    }

    edit->setStyleSheet("background-color:rgb(255, 0, 0)");

    QTimeLine *tl = new QTimeLine(1000);
    tl->setFrameRange(0, 255);
    QObject::connect(tl, &QTimeLine::finished, tl, &QTimeLine::deleteLater);
    QObject::connect(tl, &QTimeLine::frameChanged, [edit](int frame)
    {
        edit->setStyleSheet(QStringLiteral("QLineEdit { background-color:rgb(255, %1, %1); }").arg(frame));
    });
    tl->start();
}

void TasksCreator::save_settings()
{
    QSettings sett;
    sett.setValue("taskcreator_geometry", saveGeometry());
}

void TasksCreator::restore_settings()
{
    QSettings sett;
    if (sett.contains("taskcreator_geometry"))
        restoreGeometry(sett.value("taskcreator_geometry").toByteArray());
}

bool TasksCreator::save_to_file(const QList<TaskInfo> &tasks)
{
    if (edit_mode)
        return save_editted_file(tasks);
    else
        return save_new_file(tasks);
}

bool TasksCreator::save_new_file(const QList<TaskInfo> &tasks)
{
    QFileDialog fd;
    fd.setFileMode(QFileDialog::DirectoryOnly);
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setDirectory(Application::applicationDirPath());
    fd.setWindowTitle(tr("Выбор директории сохранения"));

    if (fd.exec() == QDialog::Accepted)
    {
        QStringList dirs = fd.selectedFiles();
        if (!dirs.empty())
        {
            QString dir = dirs.at(0);
            if (!dir.isEmpty())
            {
                QSettings sett;
                if (!prodisp::streq(sett.value("path", Application::applicationDirPath()).toString(), dir))
                {
                    if (prodisp::confirm_operation(tr("Вопрос"), tr("Задать выбранную директорию директорией поиска по умолчанию?"), this))
                    {
                        sett.setValue("path", dir);
                        emit ask_to_restart();
                    }
                }

                QString json = create_json(tasks);
                if (json.isEmpty())
                {
                    std::cerr << "save_new_file(): empty json" << std::endl;
                }
                else
                {
                    QString path = QString("%1/%2").arg(dir).arg(TaskExecutor::taskslist_name());
                    return write_to_file(path, json.toLocal8Bit());
                }
            }
        }
    }

    return false;
}

bool TasksCreator::save_editted_file(const QList<TaskInfo> &tasks)
{
    QString path = TaskExecutor::taskslist_path();
    QFileInfo fi(path);

    if (fi.exists())
    {
        if (!fi.isWritable())
        {
            std::cerr << "isWritable() == false" << std::endl;
            return false;
        }
    }

    QString json = create_json(tasks);
    if (json.isEmpty())
    {
        std::cerr << "save_editted_file(): empty json" << std::endl;
        return false;
    }

    return write_to_file(path, json.toLocal8Bit());
}

bool TasksCreator::write_to_file(const QString &path, const QByteArray &data)
{
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
    {
        qint64 r = f.write(data);
        return r != -1; // Успешное завершение
    }

    return false;
}
