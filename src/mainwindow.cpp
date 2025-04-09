#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QToolButton>
#include <QString>
#include <QLabel>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QSettings>
#include "archiveprocessorfactory.h"

using namespace Qt::Literals::StringLiterals;

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
    setWindowTitle(QCoreApplication::applicationName());
    // qDebug() << "Available sizes" << QIcon::fromTheme("paprefs").availableSizes();
    setupToolBars();
    setupDockWindows();
    connect(m_fileSystemBrowser, &FileSystemBrowser::requestLoadFile, this, [=](QString filePath, QKiSS::FileLoadMode fileLoadMode) {
        m_logViewer->clearAllLogs();
        AbstractArchiveProcessor *extractor = ArchiveProcessorFactory::newArchiveProcessor(ArchiveProcessorFactory::Backend::Libarchive);
        extractor->extractFileAsPromise(filePath).then([this, extractor, filePath, fileLoadMode](std::shared_ptr<QMap<QString, QByteArray>> filesMap) {
            qDebug() << "Extract ============================" << filesMap->keys();
            tryToParseAndLoadFilesAsKissFiles(filePath, filesMap, fileLoadMode);      /// <-- entrypoint of loading a CNF
            delete extractor;
        }).onFailed([extractor](){
            delete extractor;
        });
    });

    connect(m_kissView, &GKissView::highlightedItemChanged, this, [=](GKissItem *item) {
        if (item == nullptr) {
            statusBar()->clearMessage();
        } else {
            QStringList itemsOfObj;
            itemsOfObj.append(item->celFileName());
            foreach (auto sibling, *item->siblings()) {
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
    connect(m_cboxZoomScale, &QComboBox::currentTextChanged, this, [=](const QString &text) {
        bool ok;
        qreal targetScale = text.toFloat(&ok);
        if (!ok) {
            targetScale = 1;
        }
        m_kissView->scaleAbs(targetScale);
    });

    setCentralWidget(m_kissView);
    restoreWindowSettings();
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

void MainWindow::loadPath(const QString &dirOrFilePath)
{
    m_fileSystemBrowser->setPath(dirOrFilePath);
}

void MainWindow::tryToParseAndLoadFilesAsKissFiles(QString compressedFilePath, std::shared_ptr<QMap<QString, QByteArray> > extractedRawFiles,  QKiSS::FileLoadMode fileLoadMode) {
    qInfo().noquote() << tr("Start to parse CNF / KCF / CEL files...");
    KissFilesParsingResult parsedKissFiles = ArchiveFile::parseKissFiles(extractedRawFiles, compressedFilePath);
    if (fileLoadMode == QKiSS::FileLoadMode::LoadAsPatch) {
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
    bool ok = m_kissView->setArchiveFileAndRenderFirstCnf(m_archiveFile);
    if (!ok) { return; }
    qInfo().noquote() << tr("Parsing finished. Now rendering scenes...");
    m_fileSystemBrowser->markPathAsLoaded(compressedFilePath, fileLoadMode);
    m_innerFilesBrowser->rerenderFileList(m_archiveFile);
    m_codeEditor->setPlainText(m_archiveFile->getCnfFileAsString(m_archiveFile->curCnf()->fileName()));
    QStringList docFileNameList = m_archiveFile->getDocFileNameList();
    if (docFileNameList.length() == 0) {
        m_docViewer->setPlainText("");
    } else {
        QString docFileName = docFileNameList[0];
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
    // Page sets (scenes)
    // =====================================================
    foreach (auto *btn, m_btnGroupSceneSets->buttons()) {
        m_btnGroupSceneSets->removeButton(btn);   // NOTE: This is crucial here and not clearly documented in Qt. Without it, cycleToggleSceneSetBtn acts wrongly.
    }
    m_toolbarSceneSets->clear();   // This will auto delete QPushButton, and QButtonGroup will also remove the deleted buttons from itself.
    m_toolbarSceneSets->setWindowTitle(tr("Scene Sets"));   // Show in context menu of this ToolBar
    // m_toolbarSceneSets->setWindowIcon(QIcon::fromTheme("clothes"));
    QToolButton *cycleToggleSceneSetBtn = new QToolButton(m_toolbarSceneSets);
    cycleToggleSceneSetBtn->setIcon(QIcon::fromTheme("clothes"));
    cycleToggleSceneSetBtn->setText(tr("Scene Sets"));
    cycleToggleSceneSetBtn->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    m_toolbarSceneSets->addWidget(cycleToggleSceneSetBtn);
    connect(cycleToggleSceneSetBtn, &QAbstractButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        if (m_archiveFile->curCnfSetsAmount() == 0) { return; }
        uint8_t curSceneNo = m_archiveFile->curCnfSetNo();
        uint8_t nextSceneNo = (curSceneNo + 1) % m_archiveFile->curCnfSetsAmount();
        QAbstractButton *btn = m_btnGroupSceneSets->button(nextSceneNo);
        btn->click();
    });
    for (size_t sceneIdx = 0; sceneIdx < m_kissView->scenesAmount(); sceneIdx++) {
        QToolButton *btn = new QToolButton(m_toolbarSceneSets);
        m_btnGroupSceneSets->addButton(btn, sceneIdx);   // Assign sceneIdx as `id` in this group, then can be access via QButtonGroup::button() later directly.
        btn->setCheckable(true);   // Make it act like radio button
        if (sceneIdx == 0) { btn->setChecked(true); }
        btn->setText(QString::number(sceneIdx));
        m_toolbarSceneSets->addWidget(btn);
        connect(btn, &QAbstractButton::clicked, this, [=](){ switchScene(sceneIdx); });
    }
    connect(m_archiveFile, &ArchiveFile::fkissChangedCnfSetNo, this, &MainWindow::switchScene);
    // =====================================================
    // Color sets (palettes)
    // =====================================================
    foreach (auto *btn, m_btnGroupColorSets->buttons()) {
        m_btnGroupColorSets->removeButton(btn);   // NOTE: This is crucial here and not clearly documented in Qt. Without it, cycleTogglePaletteSetBtn acts wrongly.
    }
    m_toolbarColorSets->clear();   // This will auto delete QPushButton, and QButtonGroup will also remove the deleted buttons from itself.
    m_toolbarColorSets->setWindowTitle(tr("Palette Sets"));   // Show in context menu of this ToolBar
    // m_toolbarColorSets->setWindowIcon(QIcon::fromTheme("palettes"));
    QToolButton *cycleTogglePaletteSetBtn = new QToolButton(m_toolbarColorSets);
    cycleTogglePaletteSetBtn->setIcon(QIcon::fromTheme("palettes"));
    cycleTogglePaletteSetBtn->setText(tr("Palette Sets"));
    cycleTogglePaletteSetBtn->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    m_toolbarColorSets->addWidget(cycleTogglePaletteSetBtn);
    connect(cycleTogglePaletteSetBtn, &QAbstractButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        if (m_archiveFile->paletteAmountOfFirstKcfInCurCnf() == 0) { return; }
        uint8_t curPaletteNo = m_archiveFile->curPaletteNo();
        uint8_t nextPaletteNo = (curPaletteNo + 1) % m_archiveFile->paletteAmountOfFirstKcfInCurCnf();
        QAbstractButton *btn = m_btnGroupColorSets->button(nextPaletteNo);
        btn->click();
    });
    for (size_t paletteIdx = 0; paletteIdx < m_archiveFile->paletteAmountOfFirstKcfInCurCnf(); paletteIdx++) {
        QToolButton *btn = new QToolButton(m_toolbarColorSets);
        m_btnGroupColorSets->addButton(btn, paletteIdx);    // Assign paletteIdx as `id` in this group, then can be access via QButtonGroup::button() later directly.
        btn->setCheckable(true);   // Make it act like radio button
        if (paletteIdx == 0) { btn->setChecked(true); }
        btn->setText(QString::number(paletteIdx));
        m_toolbarColorSets->addWidget(btn);
        connect(btn, &QAbstractButton::clicked, this, [=](){ switchPalette(paletteIdx); });
    }
    connect(m_archiveFile, &ArchiveFile::fkissChangedPaletteNo, this, &MainWindow::switchPalette);
}

void MainWindow::switchScene(uint8_t sceneIdx)
{
    QAbstractButton *btn = m_btnGroupSceneSets->button(sceneIdx);
    if (btn == nullptr) { return; }
    btn->setChecked(true);   // NOTE: DO NOT USE `setDown()` here, it's merely a visual state, and doesn't effect `checked` state. May cause unexpected behavior when programatically triggering `click` event.
    m_archiveFile->setCnfSetNo(sceneIdx);   // REFACTOR: Disastrous logic. Only used by set/palette toolbar currently... I guess storing in GKissView may be enough?
    m_kissView->showKissSceneByIndex(sceneIdx);
    auto paletteNoInKcf = m_kissView->curKissScene()->paletteNoInKcf();
    switchPalette(paletteNoInKcf);
}

void MainWindow::switchPalette(uint8_t paletteIdx)
{
    QAbstractButton *btn = m_btnGroupColorSets->button(paletteIdx);
    if (btn == nullptr) { return; }
    btn->setChecked(true);   // NOTE: DO NOT USE `setDown()` here, it's merely a visual state, and doesn't effect `checked` state. May cause unexpected behavior when programatically triggering `click` event.
    m_archiveFile->setPaletteNo(paletteIdx);
    m_kissView->curKissScene()->setPaletteNoInKcfAndRender(paletteIdx);
}

void MainWindow::setupToolBars()
{
    // Action Icons
    ui->action_window_fileSystemBrowser->setIcon(QIcon::fromTheme("system-file-manager"));
    ui->action_window_logViewer->setIcon(QIcon::fromTheme("logviewer", QIcon::fromTheme("utilities-system-monitor")));
    ui->action_window_codeViewer->setIcon(QIcon::fromTheme("applications-development"));
    ui->action_window_innerFilesBrowser->setIcon(QIcon::fromTheme("package-unzipped"));
    ui->action_window_docViewer->setIcon(QIcon::fromTheme("text-x-readme", QIcon::fromTheme("accessories-text-editor")));
    // File
    m_toolbarFiles = new QToolBar(this);
    m_toolbarFiles->setWindowTitle(tr("Files"));   // The text in context menu when you right-click on toolbar.
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarFiles);
    m_toolbarFiles->addAction(ui->action_openFile);
    m_toolbarFiles->addAction(ui->action_openDirectory);
    // Zoom
    ui->action_resetScale->setIcon(QIcon::fromTheme("zoom-original"));
    m_toolbarZooms = new QToolBar(this);
    m_toolbarZooms->setWindowTitle(tr("Zooming Tools"));
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarZooms);
    m_cboxZoomScale = new QComboBox(m_toolbarZooms);
    for (float s : { 4.0f, 3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f, 0.5f }) {
        m_cboxZoomScale->addItem(QString::number(s, 'g', 2), QVariant(s));
    }
    m_cboxZoomScale->setCurrentIndex(m_cboxZoomScale->count() - 2);
    m_toolbarZooms->addAction(ui->action_zoomOut);
    m_toolbarZooms->addAction(ui->action_zoomIn);
    m_toolbarZooms->addWidget(m_cboxZoomScale);
    // Dock Window
    m_toolbarWindows = new QToolBar(this);
    m_toolbarWindows->setWindowTitle(tr("Windows"));
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarWindows);
    m_toolbarWindows->addAction(ui->action_window_fileSystemBrowser);
    m_toolbarWindows->addAction(ui->action_window_innerFilesBrowser);
    m_toolbarWindows->addAction(ui->action_window_docViewer);
    m_toolbarWindows->addAction(ui->action_window_codeViewer);
    m_toolbarWindows->addAction(ui->action_window_logViewer);
    m_toolbarWindows->setObjectName("MainWindowDocksToolbar");   // objectName is required when `QMainWindow::saveState()`
    // Page Sets
    m_toolbarSceneSets = new QToolBar(this);
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarSceneSets);
    m_btnGroupSceneSets = new QButtonGroup(m_toolbarSceneSets);
    m_btnGroupSceneSets->setExclusive(true);   // Radio-button liked
    // Color Sets
    m_toolbarColorSets = new QToolBar(this);
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarColorSets);
    m_btnGroupColorSets = new QButtonGroup(m_toolbarColorSets);
    m_btnGroupColorSets->setExclusive(true);   // Radio-button liked
    rerenderSetPaletteButtons();
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
    connect(m_innerFilesBrowser, &InnerFilesBrowser::itemActivated, this, [=](QTreeWidgetItem *item) {
        m_innerFilesBrowser->markItemAsLoaded(item);
        if (item->type() == InnerFilesBrowser::CustomItemCnfFileType) {
            QString cnfFileName = item->text(0);
            m_kissView->setCurCnfAndRenderScenes(cnfFileName);    /// <-- entrypoint of loading a CNF
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

void MainWindow::saveWindowSettings()
{
    QSettings set;
    set.setValue("MainWindow/Geometry", saveGeometry());
    set.setValue("MainWindow/State", saveState());
    foreach (QAction *act , m_toolbarWindows->actions()) {
        QString key = "MainWindow/ToolbarDockButtons/%1"_L1.arg(act->objectName());
        bool checked = act->isChecked();
        set.setValue(key, checked);
    }
    set.setValue("View/Zoom", m_cboxZoomScale->currentText());
}

void MainWindow::restoreWindowSettings()
{
    QSettings set;
    restoreGeometry(set.value("MainWindow/Geometry").toByteArray());
    restoreState(set.value("MainWindow/State").toByteArray());
    foreach (QAction *act , m_toolbarWindows->actions()) {
        QString key = "MainWindow/ToolbarDockButtons/%1"_L1.arg(act->objectName());
        bool toChecked = set.value(key, true).toBool();
        emit act->toggled(toChecked);
        act->setChecked(toChecked);
    }
    m_cboxZoomScale->setCurrentText(set.value("View/Zoom", "1").toString());

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

        foreach (const QUrl &url , urlList) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                qDebug() << "Dropped file:" << filePath;
                if (!filePath.endsWith(".LZH", Qt::CaseInsensitive)) { return; }
                m_logViewer->clearAllLogs();
                auto *extractor = ArchiveProcessorFactory::newArchiveProcessor(ArchiveProcessorFactory::Backend::Libarchive);
                extractor->extractFileAsPromise(filePath).then([this, extractor, filePath](std::shared_ptr<QMap<QString, QByteArray>> filesMap) {
                    tryToParseAndLoadFilesAsKissFiles(filePath, filesMap, QKiSS::FileLoadMode::LoadAsIndependent);      /// <-- entrypoint of loading a CNF
                    delete extractor;
                }).onFailed([extractor](){
                    delete extractor;
                });
                // m_archiveProcessor->extractLzh(filePath);
            }
        }
    }
    event->acceptProposedAction();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveWindowSettings();
    m_logViewer = nullptr;   // Avoid potential segfault when closing window
    QMainWindow::closeEvent(event);
}
