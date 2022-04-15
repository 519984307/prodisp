#ifndef PRESETCREATOR_H
#define PRESETCREATOR_H

#include <QDialog>

class Task;
class QLabel;
class QLineEdit;
class QTabWidget;
class QToolButton;
struct TaskInfo;

class TasksCreator : public QDialog
{
    Q_OBJECT
public:
    TasksCreator(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    TasksCreator(const QList<TaskInfo> &info, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~TasksCreator();

public slots:

private slots:
    void add_task_button_clicked();
    void remove_task_button_clicked();

    void save_button_clicked();
    void cancel_button_clicked();

    void choose_exe_file(QLineEdit *exe_edit, QLineEdit *pwd_edit);
    void choose_pwd_dir(QLineEdit *pwd_edit, QLineEdit *exe_edit);

    void exe_text_changed(const QString &text, int index);

private:
    void create_ui();

    QWidget *create_tab();
    QWidget *create_tab(const TaskInfo &info);

    QString create_json(const QList<TaskInfo> &tasks);

    void save_settings();
    void restore_settings();

    QToolButton *m_add_task_button = nullptr;
    QToolButton *m_remove_task_button = nullptr;

    QTabWidget *m_task_tabs = nullptr;
    QPushButton *m_save_button = nullptr;
    QPushButton *m_cancel_button = nullptr;

    const bool edit_mode;

    bool save_to_file(const QList<TaskInfo> &tasks);
    bool save_new_file(const QList<TaskInfo> &tasks);
    bool save_editted_file(const QList<TaskInfo> &tasks);
    bool write_to_file(const QString &path, const QByteArray &data);

signals:
    void ask_to_restart();
    void update_taskslist();
};

#endif // PRESETCREATOR_H
