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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QToolBar>
#include <QButtonGroup>

#include "archivefile.h"
#include "gkissview.h"
#include "filesystembrowser.h"
#include "logviewer.h"
#include "innerfilesbrowser.h"
#include "codeeditor.h"
#include "iteminspector.h"
#include "configfile.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void receiveMsgFromCustomQtDebugMessageHandler(QtMsgType type, const QString &msg);
    /// If it's a directory, open it in file browser. If a LZH file, try to extract it and load it.
    void openDirOrFile(const QString &dirOrFilePath);
    void tryToLoadKissFilePath(const QString &kissArchiveFilePath, QKiSS::FileLoadMode fileLoadMode);
private:
    void tryToParseAndLoadFilesAsKissFiles(QString compressedFilePath, std::shared_ptr<QMap<QString, QByteArray>> extractedRawFiles, QKiSS::FileLoadMode fileLoadMode);
    /// Note: this will activate the first button.
    void rerenderSetPaletteButtons();
    void switchToScene(uint8_t sceneIdx);
    void switchToPalette(uint8_t paletteIdx);
    void switchSceneBy(int steps);
    void switchPaletteBy(int steps);
    void setupMenuBar();
    void setupToolBars();
    void setupDockWindows();
    void setupStatusBar();
    QLabel *appendPermanentStatusBarIcon(QString iconName, uint size = 16);
    QLabel *appendPermanentStatusBarText(QString text);
    QFrame *appendPermanentStatusBarSeparator();
    void setupCenterArea();
    void setupShortcuts();
    void saveWindowSettings();
    void restoreWindowSettings();
    bool applyUiLanguageFromConfig();

private slots:
    void renderRecentFiles();
    void openFilePicker();
    void openDirPicker();
    void applyViewZoomByText(QString zoomText);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    Ui::MainWindow *ui;
    QToolBar *m_toolbarFiles;
    QToolBar *m_toolbarZooms;
    QToolBar *m_toolbarWindows;
    QToolBar *m_toolbarSceneSets;
    QToolBar *m_toolbarColorSets;
    QButtonGroup *m_btnGroupSceneSets;
    QButtonGroup *m_btnGroupColorSets;
    QComboBox *m_cboxZoomScale;
    ArchiveFile *m_archiveFile;
    GKissView *m_kissView;
    FileSystemBrowser *m_fileSystemBrowser;
    LogViewer *m_logViewer;
    InnerFilesBrowser *m_innerFilesBrowser;
    CodeEditor *m_codeEditor;
    ItemInspector *m_itemInspector;
    QPlainTextEdit *m_docViewer;
    ConfigFile &m_cfg;
};



#endif // MAINWINDOW_H
