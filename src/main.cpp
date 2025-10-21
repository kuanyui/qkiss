/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2025 ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include "translationmanager.h"

template <size_t N>
inline bool strStartsWith(const char* str, const char (&prefix)[N]) noexcept {
    // str cannot be nullptr
    return str && std::memcmp(str, prefix, N - 1) == 0;
}


QtMessageHandler originalMessageHandler = nullptr;
MainWindow *mainWindow = nullptr;

void customizedMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (mainWindow != nullptr && type != QtMsgType::QtDebugMsg) {
        if (strStartsWith(context.category, "kf.")) { goto end; }    // KDE messages
        mainWindow->receiveMsgFromCustomQtDebugMessageHandler(type, context, msg);
    }
    end:
    if (originalMessageHandler) {
        // QString fmttedMsg = qFormatLogMessage(type, context, msg);
        (*originalMessageHandler)(type, context, msg);
        // (*originalMessageHandler)(type, context, QString::fromUtf8(context.file) + QString::number(context.line) + msg);
    }
}


int main(int argc, char *argv[])
{
    // qSetMessagePattern("[%{type}][%{file}:%{line}] %{message}");
    // Or envvar
    // export QT_MESSAGE_PATTERN="[%{type}][%{file}:%{line}] %{message}"
    originalMessageHandler = qInstallMessageHandler(customizedMessageHandler);
    QCoreApplication::setOrganizationDomain("qkiss.io");
    QCoreApplication::setOrganizationName("QKiSS");
    QCoreApplication::setApplicationName("QKiSS");
    QApplication app(argc, argv);
    QFile qssFile(":/styles/main.qss");
    qssFile.open(QFile::ReadOnly);
    QString qssContent = QLatin1String(qssFile.readAll());
    app.setStyleSheet(qssContent);
    qApp->installEventFilter(&TranslationManager::instance());
    /*
    // TODO: redundant `qApp,installTranslator()`.
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qkiss_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }
    */
    mainWindow = new MainWindow();
    mainWindow->show();
    if (QCoreApplication::arguments().length() >= 2) {
        mainWindow->openDirOrFile(QCoreApplication::arguments()[1]);
    }
    // qInstallMessageHandler(originalMessageHandler);
    return app.exec();
}
