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
    m_archivedFile = new ArchivedFile{this};
    m_kissView = new GKissView(this);
    setupToolBars();
    setupDockWindows();
    // NOTE: `new` is required. Otherwise, the life-cycle of it is not long enough to extract archived file.
    // m_fileSystemBrowser->setDirPath(QDir(QDir::currentPath()).absoluteFilePath("samples"));
    m_fileSystemBrowser->setDirPath(QDir(QDir::homePath()).absoluteFilePath("tmp/kissdata"));
    // m_archiveProcessor->extractLzh("samples/MIO_H/frkismi4.lzh");     // 4bits modern palatte
    // m_archiveProcessor->extractLzh("samples/KKRSV071.LZH");     // 4bits modern palatte,  // LOTS of challenges for tolerance
    // m_archiveProcessor->extractLzh("samples/tyouda_mise/RJ047634_trial.lzh");    // 32bits full-color
    // m_archiveProcessor->extractLzh("samples/tyouda_mise/FA1280B.lzh");     // 32bits full-color
    // m_archiveProcessor->extractLzh("samples/Yuki.lzh");     // 4bits legacy palatte
    // m_archiveProcessor->extractLzh("samples/kb-ran.lzh");     // 24bits modern palatte
    // m_archiveProcessor->extractLzh("samples/pk_ask33.lzh");     // 24bits modern palatte
    // m_archiveProcessor->extractLzh("samples/kis_aki.lzh");     // Debugging for wrong object positions
    connect(m_fileSystemBrowser, &FileSystemBrowser::requestLoadFile, [=](QString filePath, bool loadAsPatch) {
        ArchiveProcessor::extractFileAsPromise(filePath).then([this, filePath, loadAsPatch](std::shared_ptr<QMap<QString, QByteArray>> filesMap) {
            m_logViewer->clearAllLogs();
            tryToLoadFilesAsKissFiles(filePath, filesMap, loadAsPatch);
        });
    });

    connect(m_kissView, &GKissView::highlightedItemChanged, [=](GKissItem *item) {
        if (item == nullptr) {
            statusBar()->clearMessage();
        } else {
            QStringList itemsOfObj;
            for (auto sibling: *item->siblings()) {
                itemsOfObj.append(item->celFileName());
            }
            statusBar()->showMessage(QString("File: %1, ObjId: %2, CelPos: (%3, %4), Size: (%5, %6), Fix: %7, Comment: %8     Object items (%9): [ %10 ]")
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
    connect(m_cboxZoomScale, &QComboBox::currentTextChanged, [=](const QString &text) {
       int targetScale = text.toFloat();
       m_kissView->scaleAbs(targetScale);
    });

    setCentralWidget(m_kissView);
    m_kissView->scale(0.3, 0.3);
    resize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::receiveMsgFromCustomQtDebugMessageHandler(QtMsgType type, const QString &msg)
{
    m_logViewer->appendLog(type, msg);
}

void MainWindow::tryToLoadFilesAsKissFiles(QString compressedFilePath, std::shared_ptr<QMap<QString, QByteArray> > extractedRawFiles, bool loadAsPatch) {
    KissFilesParsingResult parsedKissFiles = ArchivedFile::parseKissFiles(extractedRawFiles, compressedFilePath);
    if (loadAsPatch) {
        bool kissFilesLoadedOk = m_archivedFile->loadKissFilesAsPatch(parsedKissFiles, compressedFilePath);
        if (!kissFilesLoadedOk) {
            parsedKissFiles.deleteAllObjects();  // FIXME: This is too dangerous... If return value contains allocated pointer, should use smart pointer to avoid user from forgotting to delete... But std::unique_ptr cannot be placed in QHash because of CoW...
            qWarning() << "Not a valid KiSS file. Aborted.";
            return;
        }
    } else {
        bool kissFilesLoadedOk = m_archivedFile->loadKissFiles(parsedKissFiles, compressedFilePath);
        if (!kissFilesLoadedOk) {
            parsedKissFiles.deleteAllObjects();  // FIXME: This is too dangerous... If return value contains allocated pointer, should use smart pointer to avoid user from forgotting to delete... But std::unique_ptr cannot be placed in QHash because of CoW...
            qWarning() << "Not a valid KiSS file. Aborted.";
            return;
        }
    }
    m_kissView->deleteAllScenes();
    m_kissView->setArchiveFile(m_archivedFile);
    reloadCnfFileBrowser();
    rerenderSetPaletteButtons();
    m_logViewer->appendLog(QtMsgType::QtInfoMsg, QStringLiteral("File loaded: %1").arg(compressedFilePath));
}

void MainWindow::reloadCnfFileBrowser()
{
    m_cnfFileBrowser->clear();
    foreach (const QString &cnfFileName, m_archivedFile->getCnfNameList()) {
        QListWidgetItem *item = new QListWidgetItem(cnfFileName);
        m_cnfFileBrowser->addItem(item);
    }
    QString curCnfFileName = m_archivedFile->curCnf() ? m_archivedFile->curCnf()->fileName() : "";
    highlightItemInCnfFileBrowser(curCnfFileName);
}

void MainWindow::highlightItemInCnfFileBrowser(QString cnfFileName)
{
    QListWidgetItem *curItem = m_cnfFileBrowser->currentItem();
    if (curItem) {
        if (curItem->text() == cnfFileName) { return; } // prevent loop
    }
    for (int i = 0; i < m_cnfFileBrowser->count(); i++) {
        QListWidgetItem *item = m_cnfFileBrowser->item(i);
        if (item->text() == cnfFileName) {
            m_cnfFileBrowser->setCurrentItem(item);
            break;
        }
    }
}

void MainWindow::rerenderSetPaletteButtons()
{
    // Page sets
    m_toolbarPageSets->clear();   // This will auto delete QPushButton, and QButtonGroup will also remove the deleted buttons from itself.
    for (size_t sceneIdx = 0; sceneIdx < m_kissView->scenesAmount(); sceneIdx++) {
        QToolButton *btn = new QToolButton(m_toolbarPageSets);
        m_btnGroupPageSets->addButton(btn);
        btn->setCheckable(true);   // Make it act like radio button
        if (sceneIdx == 0) { btn->setChecked(true); }
        btn->setText(QString::number(sceneIdx));
        connect(btn, &QAbstractButton::pressed, [=](){
            m_kissView->showKissSceneByIndex(sceneIdx) ;
            btn->setDown(true);
        });
        m_toolbarPageSets->addWidget(btn);
    }
    // Color sets
    m_toolbarColorSets->clear();   // This will auto delete QPushButton, and QButtonGroup will also remove the deleted buttons from itself.
    for (size_t paletteIdx = 0; paletteIdx < m_archivedFile->paletteAmountOfFirstKcfInCurCnf(); paletteIdx++) {
        QToolButton *btn = new QToolButton(m_toolbarPageSets);
        m_btnGroupColorSets->addButton(btn);
        btn->setCheckable(true);   // Make it act like radio button
        if (paletteIdx == 0) { btn->setChecked(true); }
        btn->setText(QString::number(paletteIdx));
        connect(btn, &QAbstractButton::pressed, [=](){
            m_kissView->curKissScene()->setPaletteNoInKcf(paletteIdx);
        });
        m_toolbarColorSets->addWidget(btn);
    }
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
    ui->dockWidget_cnfFileSelector->setMaximumHeight(300);   // TODO: instead of a fixed height, remember its height during QMainWindow resizing.
    // Page Sets
    m_toolbarPageSets = new QToolBar(this);
    m_btnGroupPageSets = new QButtonGroup(m_toolbarPageSets);
    m_btnGroupPageSets->setExclusive(true);   // Radio-button liked
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
    m_btnGroupColorSets = new QButtonGroup(m_toolbarColorSets);
    m_btnGroupColorSets->setExclusive(true);   // Radio-button liked
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
    m_fileSystemBrowser = new FileSystemBrowser(this);
    ui->dockWidget_fileSystemBrowser->setWidget(m_fileSystemBrowser);
    // ui->dockWidget_fileSystemBrowser->resize(270, ui->dockWidget_fileSystemBrowser->height());
    m_cnfFileBrowser = new QListWidget(this);
    ui->dockWidget_cnfFileSelector->setWidget(m_cnfFileBrowser);
    m_logViewer = new LogViewer(this);
    ui->dockWidget_logOutput->setWidget(m_logViewer);
    connect(m_cnfFileBrowser, &QListWidget::itemActivated, [=](QListWidgetItem *item) {
        m_kissView->setCurCnf(item->text());
        // m_archivedFile->setCurCnfByName(item->text());
        rerenderSetPaletteButtons();
    });
    connect(m_archivedFile, &ArchivedFile::curCnfChanged, this, &MainWindow::highlightItemInCnfFileBrowser);
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
                ArchiveProcessor::extractFileAsPromise(filePath).then([this, filePath](std::shared_ptr<QMap<QString, QByteArray>> filesMap) {
                    tryToLoadFilesAsKissFiles(filePath, filesMap, false);
                });
                // m_archiveProcessor->extractLzh(filePath);
            }
        }
    }
    event->acceptProposedAction();
}
