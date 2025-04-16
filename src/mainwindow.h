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
    void setupToolBars();
    void setupDockWindows();
    void setupCenterArea();
    void setupShortcuts();
    void saveWindowSettings();
    void restoreWindowSettings();
    void renderRecentFiles();
    void addToRecentFiles(QString filePath);
private slots:
    void clearRecentFiles();
    void clearRecentDirs();
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
    QPlainTextEdit *m_docViewer;
    QStringList m_recentFiles;
    QStringList m_recentDirs;
};



#endif // MAINWINDOW_H
