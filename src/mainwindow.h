#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QToolBar>
#include <QListWidget>
#include <QButtonGroup>

#include "archivedfile.h"
#include "gkissview.h"
#include "filesystembrowser.h"
#include "logviewer.h"

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

private:
    void tryToLoadFilesAsKissFiles(QString compressedFilePath, std::shared_ptr<QMap<QString, QByteArray>> extractedRawFiles, bool loadAsPatch);
    void reloadCnfFileBrowser();
    void highlightItemInCnfFileBrowser(QString cnfFileName);
    /// Note: this will activate the first button.
    void rerenderSetPaletteButtons();
    void setupToolBars();
    void setupDockWindows();
    void setupCenterArea();
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    Ui::MainWindow *ui;
    QToolBar *m_toolbarFiles;
    QToolBar *m_toolbarZooms;
    QToolBar *m_toolbarWindows;
    QToolBar *m_toolbarPageSets;
    QToolBar *m_toolbarColorSets;
    QButtonGroup *m_btnGroupPageSets;
    QButtonGroup *m_btnGroupColorSets;
    QComboBox *m_cboxZoomScale;
    ArchivedFile *m_archivedFile;
    GKissView *m_kissView;
    FileSystemBrowser *m_fileSystemBrowser;
    LogViewer *m_logViewer;
    QListWidget *m_cnfFileBrowser;
};
#endif // MAINWINDOW_H
