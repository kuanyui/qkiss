#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QToolBar>
#include "archivedfile.h"
#include "gkissview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
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
    QComboBox *m_cboxZoomScale;
    ArchivedFile *m_archivedFile;
    ArchiveProcessor *m_archiveProcessor;
    GKissView *m_kissView;
};
#endif // MAINWINDOW_H
