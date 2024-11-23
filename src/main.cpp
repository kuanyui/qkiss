#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>


QtMessageHandler originalMessageHandler = nullptr;
MainWindow *mainWindow = nullptr;

void customizedMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (mainWindow != nullptr && type != QtMsgType::QtDebugMsg) {
        QString message = qFormatLogMessage(type, context, msg);
        mainWindow->receiveMsgFromCustomQtDebugMessageHandler(type, msg);
    }
    if (originalMessageHandler) {
        (*originalMessageHandler)(type, context, msg);
    }
}


int main(int argc, char *argv[])
{
    originalMessageHandler = qInstallMessageHandler(customizedMessageHandler);
    QApplication app(argc, argv);
    QFile qssFile(":/styles/main.qss");
    qssFile.open(QFile::ReadOnly);
    QString qssContent = QLatin1String(qssFile.readAll());
    app.setStyleSheet(qssContent);
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qkiss_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }
    mainWindow = new MainWindow();
    mainWindow->show();
    qInstallMessageHandler(originalMessageHandler);
    return app.exec();
}
