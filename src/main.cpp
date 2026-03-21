/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2026 ono ono (kuanyui) <azazabc123@gmail.com>
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
#include <QStyleFactory>
#include "translationmanager.h"
#include "ztranslator.h"
#include "logging.h"

QtMessageHandler originalMessageHandler = nullptr;
MainWindow *mainWindow = nullptr;

void customizedMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (mainWindow != nullptr && type != QtMsgType::QtDebugMsg) {
        if (!cstrStartsWith(context.category, "qkiss")) { goto end; }   /// @see `logging.h`
        mainWindow->receiveMsgFromCustomQtDebugMessageHandler(type, context, msg);
    }
    end:
    if (originalMessageHandler) {
        // QString fmttedMsg = qFormatLogMessage(type, context, msg);
        (*originalMessageHandler)(type, context, msg);
        // (*originalMessageHandler)(type, context, QString::fromUtf8(context.file) + QString::number(context.line) + msg);
    }
}

bool applyTranslatorsFromConfig()
{
    /// QTranslate
    static QTranslator *currQTranslator = nullptr;
    /// ZTranslator as fallback
    static ZTranslator *currZTranslator = nullptr;

    if (currQTranslator) {
        qApp->removeTranslator(currQTranslator);
        delete currQTranslator;
        currQTranslator = nullptr;
    }
    if (currZTranslator) {
        qApp->removeTranslator(currZTranslator);
        delete currZTranslator;
        currZTranslator = nullptr;
    }

    currQTranslator = new QTranslator{};
    currZTranslator = new ZTranslator{};
    QString langId = ConfigFile::instance().effectedUiLanguage();
    if (currQTranslator->load(":/i18n/qkiss_" + langId + ".qm")) {
        currZTranslator->setLang(langId);
        // NOTE: installed translator is applied in reversed order.
        qApp->installTranslator(currZTranslator);   // 1. fallback
        qApp->installTranslator(currQTranslator);   // 0. primary
        qkDebug << "[DEBUG] ::applyTranslatorsFromConfig() SUCCESS" << langId;
        return true;
    } else {
        qkDebug << "[DEBUG] ::applyTranslatorsFromConfig() FAILED" << langId;
        delete currQTranslator;
        currQTranslator = nullptr;
        delete currZTranslator;
        currZTranslator = nullptr;
        return false;
    }
}


int main(int argc, char *argv[])
{
    // qSetMessagePattern("[%{type}][%{file}:%{line}] %{message}");
    // Or envvar
    // export QT_MESSAGE_PATTERN="[%{type}][%{file}:%{line}] %{message}"
    originalMessageHandler = qInstallMessageHandler(customizedMessageHandler);
    /*
     * Qt doc: "This property holds the **Internet domain** of the organization that wrote this application".
     * For macOS only.
     * - If set to `qkiss.app`, macOS converts it to `app.qkiss.<APP>.plist`
     */
    QCoreApplication::setOrganizationDomain("qkiss.app");
    /*
     * For new project, lowercase is safer.
     * - Windows:
     *    - HKEY_CURRENT_USER\Software\<ORG>\<APP>
     *    - C:\Users\<user>\AppData\{Roaming,Local}\<ORG>\<APP>.ini
     * - macOS:
     *    - (If domain is set) ~/Library/Preferences/<reversed(DOMAIN)>.<APP>.plist
     *    - (If no domain set) ~/Library/Preferences/<ORG>.<APP>.plist
     * - Linux:
     *    - ~/.config/<ORG>/<APP>.conf
     */
    QCoreApplication::setOrganizationName("qkiss");
    QCoreApplication::setApplicationName("qkiss");

    // avoid wrong icon bouncing when startup in KDE
    QGuiApplication::setDesktopFileName("qkiss");

    QApplication app(argc, argv);
#ifdef Q_OS_WIN
    // Windows style + color is too ugly on Windows... So use Fusion on Windows
    app.setStyle(QStyleFactory::create("Fusion"));
#endif
    QFile qssFile(":/styles/main.qss");
    [[maybe_unused]] bool success = qssFile.open(QFile::ReadOnly);
    QString qssContent = QLatin1String(qssFile.readAll());
    app.setStyleSheet(qssContent);
    qApp->installEventFilter(&TranslationManager::instance());
    applyTranslatorsFromConfig();
    mainWindow = new MainWindow();
    QObject::connect(&ConfigFile::instance(), &ConfigFile::uiLanguageChanged, []() {
        applyTranslatorsFromConfig();
        mainWindow->retranslate();
    });
    mainWindow->show();
    if (QCoreApplication::arguments().length() >= 2) {
        mainWindow->openDirOrFile(QCoreApplication::arguments()[1]);
    }
    // qInstallMessageHandler(originalMessageHandler);
    return app.exec();
}
