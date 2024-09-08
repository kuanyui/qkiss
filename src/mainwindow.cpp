#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QToolButton>
#include <QString>
#include <QLabel>
#include <QDragEnterEvent>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupToolBars();
    qDebug() << ArchivedFile::checkLhaCommandAvailability();
    // NOTE: `new` is required. Otherwise, the life-cycle of it is not long enough to extract archived file.
    m_archivedFile = new ArchivedFile{this};
    // m_archivedFile->loadLzh("samples/MIO_H/frkismi4.lzh", QtKISS::Case::Insensitive);     // 4bits modern palatte
    // m_archivedFile->loadLzh("samples/tyouda_mise/RJ047634_trial.lzh", QtKISS::Case::Insensitive);    // 32bits full-color
    // m_archivedFile->loadLzh("samples/tyouda_mise/FA1280B.lzh", QtKISS::Case::Insensitive);     // 32bits full-color
    // m_archivedFile->loadLzh("samples/Yuki.lzh", QtKISS::Case::Insensitive);     // 4bits legacy palatte
    // m_archivedFile->loadLzh("samples/kb-ran.lzh", QtKISS::Case::Insensitive);     // 24bits modern palatte
    m_archivedFile->loadLzh("samples/pk_ask33.lzh", QtKISS::Case::Insensitive);     // 24bits modern palatte

    m_kissView = new GKissView(this);
    connect(m_archivedFile, &ArchivedFile::archivedFilesUncompressAndParseFinished, [=]() {
        m_kissView->unloadArchivedFile();
        m_kissView->loadArchivedFile(m_archivedFile);
        m_kissView->setCurCnf(m_archivedFile->getCnfNameList().first());
        m_toolbarPageSets->clear();
        for (size_t sceneIdx = 0; sceneIdx < m_kissView->scenesAmount(); sceneIdx++) {
            QToolButton *btn = new QToolButton(m_toolbarPageSets);
            btn->setText(QString::number(sceneIdx));
            connect(btn, &QAbstractButton::pressed, [=](){
                m_kissView->showSceneByIndex(sceneIdx) ;
            });
            m_toolbarPageSets->addWidget(btn);
        }
    });
    connect(m_kissView, &GKissView::hoveredItemChanged, [=](GKissItem *item, GKissScene *scene) {
        if (item == nullptr) {
            statusBar()->clearMessage();
        } else {
            QStringList itemsOfObj;
            for (auto sibling: *item->siblings()) {
                itemsOfObj.append(item->celFileName());
            }
            statusBar()->showMessage(QString("File: %1, ObjId: %2, Pos: (%3, %4), Size: (%5, %6), Fix: %7, Comment: %8     Object items (%9): [ %10 ]")
                                     .arg(item->celFileName())
                                     .arg(item->objId())
                                     .arg(item->cel()->x())
                                     .arg(item->cel()->y())
                                     .arg(item->cel()->width())
                                     .arg(item->cel()->height())
                                     .arg(item->fix())
                                     .arg(item->cnfCel()->comment())
                                     .arg(itemsOfObj.length())
                                     .arg(itemsOfObj.join(", "))
                                     );
        }
    });
    // m_kissView->scale(2, 2);
    connect(m_cboxZoomScale, &QComboBox::currentTextChanged, [=](const QString &text) {
       int targetScale = text.toFloat();
       m_kissView->scaleAbs(targetScale);
    });

    setCentralWidget(m_kissView);
    // resize(920, 560);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupToolBars()
{
    // File
    m_toolbarFiles = new QToolBar(this);
    m_toolbarFiles->setWindowTitle(tr("Files"));   // The text in context menu when you right-click on toolbar.
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarFiles);
    m_toolbarFiles->addAction(ui->action_openFile);
    m_toolbarFiles->addAction(ui->action_openDirectory);
    // Zoom
    m_toolbarZooms = new QToolBar(this);
    m_toolbarZooms->setWindowTitle(tr("Zooming Tools"));
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarZooms);
    m_cboxZoomScale = new QComboBox(m_toolbarZooms);
    m_cboxZoomScale->addItem("1", QVariant(1));
    m_cboxZoomScale->addItem("2", QVariant(2));
    m_cboxZoomScale->addItem("3", QVariant(3));
    m_cboxZoomScale->addItem("4", QVariant(4));
    m_cboxZoomScale->addItem("5", QVariant(5));
    m_toolbarZooms->addAction(ui->action_zoomOut);
    m_toolbarZooms->addAction(ui->action_zoomIn);
    m_toolbarZooms->addWidget(m_cboxZoomScale);
    // Dock Window
    m_toolbarWindows = new QToolBar(this);
    m_toolbarWindows->setWindowTitle(tr("Windows"));
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarWindows);
    m_toolbarWindows->addAction(ui->action_window_fileSystemBrowser);
    m_toolbarWindows->addAction(ui->action_window_cnfSelector);
    m_toolbarWindows->addAction(ui->action_window_logs);
    // Page Sets
    m_toolbarPageSets = new QToolBar(this);
    m_toolbarPageSets->setWindowTitle(tr("Page Sets"));
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarPageSets);
    m_toolbarPageSets->addWidget(new QLabel(tr("Page Set"), m_toolbarPageSets));
    for (int i = 0; i < 9; i++) {
        QToolButton *btn = new QToolButton(m_toolbarPageSets);
        btn->setText(QString::number(i+1));
        btn->setCheckable(true);
        m_toolbarPageSets->addWidget(btn);
    }
    // Color Sets
    m_toolbarColorSets = new QToolBar(this);
    m_toolbarColorSets->setWindowTitle(tr("Color Sets"));
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarColorSets);
    m_toolbarColorSets->addWidget(new QLabel(tr("Color Set"), m_toolbarColorSets));
    for (int i = 0; i < 9; i++) {
        QToolButton *btn = new QToolButton(m_toolbarColorSets);
        btn->setText(QString::number(i+1));
        btn->setCheckable(true);
        m_toolbarColorSets->addWidget(btn);
    }
}

void MainWindow::setupDockWindows()
{

}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();

        for (const QUrl &url : urlList) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                qDebug() << "Dropped file:" << filePath;
                if (!filePath.toUpper().endsWith(".LZH")) { return; }
                m_archivedFile->loadLzh(filePath, QtKISS::Case::Insensitive);
            }
        }
    }
    event->acceptProposedAction();
}
