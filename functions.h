#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <cstring>

#include <QObject>
#include <QString>

namespace prodisp
{

bool streq(const QString &s1, const QString &s2);
bool streq(const std::string &s1, const std::string &s2);
bool streq(const char *s1, const char *s2);

bool confirm_operation(const QString &title, const QString &text = QObject::tr("Вы уверены?"), QWidget *parent = nullptr) noexcept;

}

#endif // FUNCTIONS_H
