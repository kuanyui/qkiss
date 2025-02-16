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
    m_archiveFile = new ArchiveFile{this};
    m_kissView = new GKissView(this);
    QIcon::setThemeSearchPaths({ ":/xdg_icon_themes" });
    // QIcon::setFallbackSearchPaths({ ":/icons/light" });
    QIcon::setThemeName("CustomizedTango");    // folder name under any of QIcon::themeSearchPaths (:/src/humanity_0.4.6)
    // setWindowIcon(QIcon::fromTheme("paprefs"));
    setWindowIcon(QIcon::fromTheme("qkiss"));
    ui->action_window_fileSystemBrowser->setIcon(QIcon::fromTheme("system-file-manager"));
    ui->action_window_logViewer->setIcon(QIcon::fromTheme("logviewer", QIcon::fromTheme("utilities-system-monitor")));
    ui->action_window_codeViewer->setIcon(QIcon::fromTheme("applications-development"));
    ui->action_window_innerFilesBrowser->setIcon(QIcon::fromTheme("package-x-generic"));
    ui->action_window_docViewer->setIcon(QIcon::fromTheme("text-x-readme", QIcon::fromTheme("accessories-text-editor")));
    // qDebug() << "Available sizes" << QIcon::fromTheme("paprefs").availableSizes();
    setupToolBars();
    setupDockWindows();
    m_fileSystemBrowser->setDirPath(QDir(QDir::homePath()).absoluteFilePath("tmp/kissdata"));
    connect(m_fileSystemBrowser, &FileSystemBrowser::requestLoadFile, [=](QString filePath, bool loadAsPatch) {
        m_logViewer->clearAllLogs();
        ArchiveProcessor::extractFileAsPromise(filePath).then([this, filePath, loadAsPatch](std::shared_ptr<QMap<QString, QByteArray>> filesMap) {
            tryToLoadFilesAsKissFiles(filePath, filesMap, loadAsPatch);
        });
    });

    connect(m_kissView, &GKissView::highlightedItemChanged, [=](GKissItem *item) {
        if (item == nullptr) {
            statusBar()->clearMessage();
        } else {
            QStringList itemsOfObj;
            itemsOfObj.append(item->celFileName());
            for (auto sibling: *item->siblings()) {
                itemsOfObj.append(sibling->celFileName());
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
    m_logViewer = nullptr;  // NOTE: I don't sure if I can set it to nullptr in dtor, or QObject will relys on it.
    delete ui;
    ui = nullptr;   // To prevent customized QtDebug message handler from segfault.
}

void MainWindow::receiveMsgFromCustomQtDebugMessageHandler(QtMsgType type, const QString &msg)
{
    if (m_logViewer == nullptr) { return; }
    m_logViewer->appendLog(type, msg);
}

void MainWindow::tryToLoadFilesAsKissFiles(QString compressedFilePath, std::shared_ptr<QMap<QString, QByteArray> > extractedRawFiles, bool loadAsPatch) {
    qInfo().noquote() << tr("Start to parse CNF / KCF / CEL files...");
    KissFilesParsingResult parsedKissFiles = ArchiveFile::parseKissFiles(extractedRawFiles, compressedFilePath);
    if (loadAsPatch) {
        bool kissFilesLoadedOk = m_archiveFile->loadKissFilesAsPatch(parsedKissFiles, compressedFilePath);
        if (!kissFilesLoadedOk) {
            parsedKissFiles.deleteAllObjects();  // FIXME: This is too dangerous... If return value contains allocated pointer, should use smart pointer to avoid user from forgotting to delete... But std::unique_ptr cannot be placed in QHash because of CoW...
            qWarning().noquote() << tr("\"%1\" is not a valid KiSS patch file. Aborted.").arg(compressedFilePath);
            return;
        }
    } else {
        bool kissFilesLoadedOk = m_archiveFile->loadKissFiles(parsedKissFiles, compressedFilePath);
        if (!kissFilesLoadedOk) {
            parsedKissFiles.deleteAllObjects();  // FIXME: This is too dangerous... If return value contains allocated pointer, should use smart pointer to avoid user from forgotting to delete... But std::unique_ptr cannot be placed in QHash because of CoW...
            qWarning().noquote() << tr("\"%1\"is not a valid KiSS file. Aborted.").arg(compressedFilePath);
            return;
        }
    }
    m_kissView->deleteAllScenes();
    m_kissView->setArchiveFile(m_archiveFile);
    qInfo().noquote() << tr("Parsing finished. Now rendering scenes...");
    m_innerFilesBrowser->rerenderFileList(m_archiveFile);
    m_codeEditor->setPlainText(m_archiveFile->getCnfFileAsString(m_archiveFile->curCnf()->fileName()));
    if (m_archiveFile->getDocFileNameList().length() == 0) {
        m_docViewer->setPlainText("");
    } else {
        QString docFileName = m_archiveFile->getDocFileNameList()[0];
        m_docViewer->setPlainText(m_archiveFile->getDocFileAsString(docFileName));
        ui->dockWidget_docViewer->raise();
    }
    rerenderSetPaletteButtons();
    qInfo().noquote() << tr("KiSS files rendered successfully: %1").arg(QFileInfo(compressedFilePath).fileName());
    // m_logViewer->appendLog(QtMsgType::QtInfoMsg, QStringLiteral("KiSS files rendered successfully: %1").arg(QFileInfo(compressedFilePath).fileName()));
}


void MainWindow::rerenderSetPaletteButtons()
{
    // =====================================================
    // Page sets
    // =====================================================
    m_toolbarPageSets->clear();   // This will auto delete QPushButton, and QButtonGroup will also remove the deleted buttons from itself.
    for (size_t sceneIdx = 0; sceneIdx < m_kissView->scenesAmount(); sceneIdx++) {
        QToolButton *btn = new QToolButton(m_toolbarPageSets);
        m_btnGroupPageSets->addButton(btn);
        btn->setCheckable(true);   // Make it act like radio button
        if (sceneIdx == 0) { btn->setChecked(true); }
        btn->setText(QString::number(sceneIdx));
        connect(btn, &QAbstractButton::pressed, [=](){
            m_kissView->showKissSceneByIndex(sceneIdx);
            m_archiveFile->setCurCnfSetNo(sceneIdx);   // FIXME: It seems m_archiveFile stores a setNo, but m_kissView itself also stores a setNo (more precisely, sceneIdx). Should keep a singleton here.
            btn->setDown(true);
        });
        connect(m_archiveFile, &ArchiveFile::curCnfSetNoChanged, [=](uint8_t newCurCnfSetNo){
            if (sceneIdx == newCurCnfSetNo) {
                btn->setDown(true);
            }
        });
        m_toolbarPageSets->addWidget(btn);
    }
    // =====================================================
    // Color sets
    // =====================================================
    m_toolbarColorSets->clear();   // This will auto delete QPushButton, and QButtonGroup will also remove the deleted buttons from itself.
    for (size_t paletteIdx = 0; paletteIdx < m_archiveFile->paletteAmountOfFirstKcfInCurCnf(); paletteIdx++) {
        QToolButton *btn = new QToolButton(m_toolbarPageSets);
        m_btnGroupColorSets->addButton(btn);
        btn->setCheckable(true);   // Make it act like radio button
        if (paletteIdx == 0) { btn->setChecked(true); }
        btn->setText(QString::number(paletteIdx));
        connect(btn, &QAbstractButton::pressed, [=](){
            m_kissView->curKissScene()->setPaletteNoInKcf(paletteIdx);
            m_archiveFile->setCurPaletteNo(paletteIdx);    // FIXME: It seems m_archiveFile stores a setNo, but m_kissView->curKissScene() itself also stores a paletteNo. Should keep a singleton here.
        });
        connect(m_archiveFile, &ArchiveFile::curPaletteNoChanged, [=](uint8_t newCurPaletteNo){
            if (paletteIdx == newCurPaletteNo) {
                btn->setDown(true);
            }
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
    m_toolbarWindows->addAction(ui->action_window_innerFilesBrowser);
    m_toolbarWindows->addAction(ui->action_window_logViewer);
    m_toolbarWindows->addAction(ui->action_window_codeViewer);
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
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);  // Make bottom dock cannot stretch to bottom-left corner.
    // tabifyDockWidget(ui->dockWidget_logViewer, ui->dockWidget_textFileViewer);
    // ui->dockWidget_logViewer->raise();  // activate "logs viewer" tab.
    addDockWidget(Qt::BottomDockWidgetArea, ui->dockWidget_cnfCodeViewer);
    addDockWidget(Qt::BottomDockWidgetArea, ui->dockWidget_logViewer);
    // ==================================================
    // File System Browser
    // ==================================================
    m_fileSystemBrowser = new FileSystemBrowser(this);
    ui->dockWidget_fileSystemBrowser->setWidget(m_fileSystemBrowser);
    // ui->dockWidget_fileSystemBrowser->resize(270, ui->dockWidget_fileSystemBrowser->height());
    // ==================================================
    // Inner Files Browser
    // ==================================================
    m_innerFilesBrowser = new InnerFilesBrowser(this);
    ui->dockWidget_cnfFileSelector->setWidget(m_innerFilesBrowser);
    // ==================================================
    // Log Viewer
    // ==================================================
    m_logViewer = new LogViewer(this);
    ui->dockWidget_logViewer->setWidget(m_logViewer);
    // ==================================================
    // Code Editor
    // ==================================================
    m_codeEditor = new CodeEditor(this);
    ui->dockWidget_cnfCodeViewer->setWidget(m_codeEditor);
    // ==================================================
    // Doc Viewer
    // ==================================================
    QFont fontMono {"Monospace"};
    fontMono.setStyleHint(QFont::TypeWriter);
    m_docViewer= new QPlainTextEdit(this);
    m_docViewer->setReadOnly(true);
    m_docViewer->setFont(fontMono);
    ui->dockWidget_docViewer->setWidget(m_docViewer);
    // ==================================================
    // (Signals)
    // ==================================================
    connect(m_innerFilesBrowser, &InnerFilesBrowser::itemActivated, [=](QTreeWidgetItem *item) {
        if (item->type() == InnerFilesBrowser::CustomItemCnfFileType) {
            QString cnfFileName = item->text(0);
            m_kissView->setCurCnfAndRenderScenes(cnfFileName);
            // m_archiveFile->setCurCnfByName(item->text());
            rerenderSetPaletteButtons();
            m_codeEditor->setPlainText(m_archiveFile->getCnfFileAsString(cnfFileName));
        } else if (item->type() == InnerFilesBrowser::CustomItemDocFileType) {
            QString docFileName = item->text(0);
            m_docViewer->setPlainText(m_archiveFile->getDocFileAsString(docFileName));
            ui->dockWidget_docViewer->raise();
        }
    });
    connect(m_archiveFile, &ArchiveFile::curCnfChangedAndLoaded, [=](){
    });
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
                m_logViewer->clearAllLogs();
                ArchiveProcessor::extractFileAsPromise(filePath).then([this, filePath](std::shared_ptr<QMap<QString, QByteArray>> filesMap) {
                    tryToLoadFilesAsKissFiles(filePath, filesMap, false);
                });
                // m_archiveProcessor->extractLzh(filePath);
            }
        }
    }
    event->acceptProposedAction();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_logViewer = nullptr;   // Avoid potential segfault when closing window
    QMainWindow::closeEvent(event);
}
