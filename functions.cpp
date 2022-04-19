#include "functions.h"

#include <QMessageBox>
#include <QPushButton>

#if defined(_MSC_VER)
    #define strncasecmp _strnicmp
    #define strcasecmp _stricmp
#endif

bool prodisp::streq(const QString &s1, const QString &s2)
{
    return (s1.compare(s2, Qt::CaseInsensitive) == 0);
}

bool prodisp::streq(const std::string &s1, const std::string &s2)
{
    return streq(s1.c_str(), s2.c_str());
}

bool prodisp::streq(const char *s1, const char *s2)
{
    return (strcasecmp(s1, s2) == 0);
}

bool prodisp::confirm_operation(const QString &title, const QString &text, QWidget *parent) noexcept
{
    QMessageBox mbox(parent);
    mbox.setWindowTitle(title);
    mbox.setText(text);
    mbox.setIcon(QMessageBox::Question);
    mbox.addButton(QObject::tr("Да"), QMessageBox::YesRole);
    QPushButton *no = mbox.addButton(QObject::tr("Нет"), QMessageBox::NoRole);
    mbox.setDefaultButton(no);
    mbox.exec();

    return (mbox.clickedButton() != no);
}
