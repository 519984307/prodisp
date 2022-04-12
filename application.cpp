#include "application.h"

#include <QFileInfo>
#include <QThreadPool>
#include <QTranslator>
#include <QLibraryInfo>
#include <QStyleFactory>

#include <iostream>

Application::Application(int &argc, char *argv[])
    : QApplication(argc, argv)
{
    setObjectName("prodisp_application");
    setStyle(QStyleFactory::create("Fusion"));
    setOrganizationName("prodisp");
    setApplicationName("prodisp");
    setApplicationVersion(PRODISPVERSION);

    QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount() > 1 ? QThreadPool::globalInstance()->maxThreadCount() : 2);

    /* Установка перевода */
    QTranslator translator;
    if (translator.load(QString("qt_%1").arg(QLocale::system().name()),
                        QFileInfo::exists(QString("%1/translations/qt_ru.qm").arg(applicationDirPath())) ?
                        QString("%1/translations").arg(applicationDirPath()) :
                        QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    {
        if (!installTranslator(&translator))
        {
            std::cerr << "installTranslator() failed" << std::endl;
        }
    }
}

bool Application::notify(QObject *receiver, QEvent *event)
{
    try
    {
        return QApplication::notify(receiver, event);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "unknown exception" << std::endl;
    }

    return false;
}
