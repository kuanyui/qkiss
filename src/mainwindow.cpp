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
#include <QToolButton>
#include <QString>
#include <QLabel>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QShortcut>
#include <QFileDialog>
#include <QActionGroup>
#include <QTranslator>
#include <QLocale>
#include "archiveprocessorfactory.h"
#include "dockwidgetproxystyle.h"
#include "iteminspector.h"
#include "configfile.h"
#include "src/ui_mainwindow.h"
#include "preferencedialog.h"
#include "aboutdialog.h"
#include "translationmanager.h"
#include "version.h"
#include "logging.h"

using namespace Qt::Literals::StringLiterals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_cfg{ ConfigFile::instance() }
{
    ui->setupUi(this);
    m_archiveFile = new ArchiveFile{this};
    m_kissView = new GKissView(this);
    m_kissView->scaleAbs(m_cfg.zoom());
    QIcon::setThemeSearchPaths({ ":/xdg_icon_themes" });
    // QIcon::setFallbackSearchPaths({ ":/icons/light" });
    QIcon::setThemeName("CustomizedTango");    // folder name under any of QIcon::themeSearchPaths (:/src/humanity_0.4.6)
    // setWindowIcon(QIcon::fromTheme("paprefs"));
    setWindowIcon(QIcon::fromTheme("qkiss"));
    // setWindowTitle(QCoreApplication::applicationName());
    setWindowTitle(QString("%1 %2").arg(PROJ_DISPLAY_NAME, PROJECT_VERSION));
    // qkDebug << "Available sizes" << QIcon::fromTheme("paprefs").availableSizes();
    setupMenuBar();
    setupToolBars();
    setupDockWindows();
    setupStatusBar();
    setupShortcuts();
    connect(m_fileSystemBrowser, &FileSystemBrowser::requestLoadFile, this, [=, this](QString filePath, QKiSS::FileLoadMode fileLoadMode) {
        tryToLoadKissFilePath(filePath, fileLoadMode);
    });

    connect(m_kissView, &GKissView::highlightedItemChanged, this, [=, this](GKissItem *item) {
        m_itemInspector->setInspectedItem(item);
        if (item == nullptr) {
            statusBar()->clearMessage();
        } else {
            QStringList itemsOfObj;
            itemsOfObj.append(item->celFileName().original());
            foreach (auto sibling, *item->siblings()) {
                itemsOfObj.append(sibling->celFileName().original());
            }
            statusBar()->showMessage(tr("Fix: %1, CEL Name: %2, Comment: %3")
                                     .arg(item->fix())
                                     .arg(item->celFileName().original())
                                     .arg(item->cnfCel()->comment())
                                     );
        }
    });
    connect(m_kissView, &GKissView::requestSwitchingSet, this, &MainWindow::switchSceneBy);
    // Zooming
    // connect(m_cboxZoomScale, &QComboBox::currentTextChanged, [=, this]{
    //     qreal factor = ConfigFile::parseZoomHumanToFactor(m_cboxZoomScale->lineEdit()->text());
    //     m_kissView->scaleAbs(factor);
    // });
    // connect(m_cboxZoomScale->lineEdit(), &QLineEdit::editingFinished, [=, this](){
    //     qreal factor = ConfigFile::parseZoomHumanToFactor(m_cboxZoomScale->lineEdit()->text());
    //     m_cfg.setZoom(factor);
    // });
    connect(&m_cfg, &ConfigFile::zoomChanged, [=, this](){
        m_kissView->scaleAbs(m_cfg.zoom());
        m_cboxZoomScale->setCurrentText(m_cfg.zoomAsPercent());
    });
    connect(m_cboxZoomScale, &QComboBox::currentTextChanged, [=, this](){    // ComboBox triggered by user directly
        qreal factor = ConfigFile::parseZoomHumanToFactor(m_cboxZoomScale->lineEdit()->text());
        m_cfg.setZoom(factor);
    });
    connect(ui->action_zoomOut, &QAction::triggered, &m_cfg, [=, this](){
        m_cfg.zoomOut();
    });
    connect(ui->action_zoomIn, &QAction::triggered, &m_cfg, [=, this](){
        m_cfg.zoomIn();
    });
    connect(ui->action_zoomReset, &QAction::triggered, &m_cfg, [=, this](){
        m_cfg.zoomReset();
    });
    ui->action_zoomIn->setDisabled(!m_cfg.canZoomIn());    // REFACTOR: How to DRY?
    ui->action_zoomOut->setDisabled(!m_cfg.canZoomOut());    // REFACTOR: How to DRY?
    connect(&m_cfg, &ConfigFile::zoomChanged, this, [=, this] () {
        ui->action_zoomIn->setDisabled(!m_cfg.canZoomIn());
        ui->action_zoomOut->setDisabled(!m_cfg.canZoomOut());
    });

    // Zooming via Ctrl + mouse wheel on view
    connect(m_kissView, &GKissView::requestZoomIn, &m_cfg, [=, this](){
        m_cfg.zoomIn();
    });
    connect(m_kissView, &GKissView::requestZoomOut, &m_cfg, [=, this](){
        m_cfg.zoomOut();
    });

    // Other
    connect(ui->action_openFile, &QAction::triggered, this, &MainWindow::openFilePicker);
    connect(ui->action_openDirectory, &QAction::triggered, this, &MainWindow::openDirPicker);
    connect(&m_cfg, &ConfigFile::recentFilesChanged, this, &MainWindow::renderRecentFiles);

    // Preference Dialog
    connect(ui->action_preferences, &QAction::triggered, this, [=, this]() {
        auto dialog = new PreferenceDialog(this);
        dialog->show();
    });
    connect(ui->action_helpAbout, &QAction::triggered, this, [=, this]() {
        auto dialog = new AboutDialog(this);
        dialog->show();
    });

    connect(m_archiveFile, &ArchiveFile::fkissChangedCnfSetNo, this, [=, this](uint8_t newCurSnfSetNo) { switchToScene(newCurSnfSetNo, true); });
    connect(m_archiveFile, &ArchiveFile::fkissChangedPaletteNo, this, [=, this](uint8_t newCurSnfSetNo) { switchToPalette(newCurSnfSetNo, true); });

    setCentralWidget(m_kissView);
    restoreWindowSettings();
}

MainWindow::~MainWindow()
{
    m_logViewer = nullptr;  // NOTE: I don't sure if I can set it to nullptr in dtor, or QObject will relys on it.
    delete ui;
    ui = nullptr;   // To prevent customized QtDebug message handler from segfault.
}

void MainWindow::receiveMsgFromCustomQtDebugMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (m_logViewer == nullptr) { return; }
    m_logViewer->appendLog(type, context, msg);
}

void MainWindow::openDirOrFile(const QString &dirOrFilePath)
{
    ui->dockWidget_fileSystemBrowser->setVisible(true);
    m_fileSystemBrowser->setPath(dirOrFilePath);
    tryToLoadKissFilePath(dirOrFilePath, QKiSS::FileLoadMode::LoadAsIndependent);
}

void MainWindow::tryToLoadKissFilePath(const QString &kissArchiveFilePath, QKiSS::FileLoadMode fileLoadMode)
{
    if (!kissArchiveFilePath.endsWith(".LZH", Qt::CaseInsensitive) && !kissArchiveFilePath.endsWith(".CNF", Qt::CaseInsensitive)) { return; }
    m_logViewer->clearAllLogs();
    std::unique_ptr<AbstractArchiveProcessor> _extractor = ArchiveProcessorFactory::getArchiveProcessor(m_cfg.lzhDecompressorBackend(), kissArchiveFilePath);
    std::shared_ptr<AbstractArchiveProcessor> extractor { _extractor.release() };
    extractor->extractFileAsPromise(kissArchiveFilePath).then([this, extractor, kissArchiveFilePath, fileLoadMode](std::shared_ptr<QMap<QString, QByteArray>> filesMap) {
        qkDebug << "Extract ============================" << filesMap->keys();
        tryToParseAndLoadFilesAsKissFiles(kissArchiveFilePath, filesMap, fileLoadMode);      /// <-- entrypoint of loading a CNF
    }).onFailed([extractor](){
    });
}

void MainWindow::tryToParseAndLoadFilesAsKissFiles(QString compressedFilePath, std::shared_ptr<QMap<QString, QByteArray> > extractedRawFiles,  QKiSS::FileLoadMode fileLoadMode) {
    qkInfo.noquote() << tr("Start to parse CNF / KCF / CEL files...");
    KissFilesParsingResult parsedKissFiles = ArchiveFile::parseKissFiles(extractedRawFiles, compressedFilePath);
    if (fileLoadMode == QKiSS::FileLoadMode::LoadAsPatch) {
        bool kissFilesLoadedOk = m_archiveFile->loadKissFilesAsPatch(parsedKissFiles, compressedFilePath);
        if (!kissFilesLoadedOk) {
            parsedKissFiles.deleteAllObjects();  // FIXME: This is too dangerous... If return value contains allocated pointer, should use smart pointer to avoid user from forgotting to delete... But std::unique_ptr cannot be placed in QHash because of CoW...
            qkCritical.noquote() << tr("\"%1\" is not a valid KiSS patch file. Aborted.").arg(compressedFilePath);
            return;
        }
    } else {
        bool kissFilesLoadedOk = m_archiveFile->loadKissFiles(parsedKissFiles, compressedFilePath);
        if (!kissFilesLoadedOk) {
            parsedKissFiles.deleteAllObjects();  // FIXME: This is too dangerous... If return value contains allocated pointer, should use smart pointer to avoid user from forgotting to delete... But std::unique_ptr cannot be placed in QHash because of CoW...
            qkCritical.noquote() << tr("\"%1\"is not a valid KiSS file. Aborted.").arg(compressedFilePath);
            return;
        }
    }
    m_kissView->deleteAllScenes();
    bool ok = m_kissView->setArchiveFileAndRenderFirstCnf(m_archiveFile);
    if (!ok) { return; }
    m_cfg.appendRecentFile(compressedFilePath);
    qkInfo.noquote() << tr("Parsing finished. Now rendering scenes...");
    m_fileSystemBrowser->markPathAsLoaded(compressedFilePath, fileLoadMode);
    m_innerFilesBrowser->rerenderFileList(m_archiveFile);
    m_codeEditor->setPlainText(m_archiveFile->getCnfFileAsString(m_archiveFile->curCnf()->fileName()));
    CPStringList docFileNameList = m_archiveFile->getDocFileNameList();
    if (docFileNameList.length() == 0) {
        m_docViewer->setPlainText("");
    } else {
        CPString docFileName = docFileNameList[0];
        m_docViewer->setPlainText(m_archiveFile->getDocFileAsString(docFileName));
        ui->dockWidget_docViewer->raise();
    }
    rerenderSetPaletteButtons();
    qkSuccess.noquote() << tr("KiSS files rendered successfully: %1").arg(QFileInfo(compressedFilePath).fileName());
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
    cycleToggleSceneSetBtn->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    cycleToggleSceneSetBtn->setIcon(QIcon::fromTheme("clothes"));
    TranslationManager::instance().registerLanguageChangeCallback(cycleToggleSceneSetBtn, [=, this]() {
        m_toolbarSceneSets->setWindowTitle(tr("Scene Sets"));   // Show in context menu of this ToolBar
        cycleToggleSceneSetBtn->setText(tr("Scene Sets"));
        cycleToggleSceneSetBtn->setToolTip(tr("Switch scene set (F1/F2)"));
    });
    m_toolbarSceneSets->addWidget(cycleToggleSceneSetBtn);
    connect(cycleToggleSceneSetBtn, &QAbstractButton::clicked, this, [this](bool checked){
        Q_UNUSED(checked);
        switchSceneBy(1);
    });
    for (size_t sceneIdx = 0; sceneIdx < m_kissView->scenesAmount(); sceneIdx++) {
        QToolButton *btn = new QToolButton(m_toolbarSceneSets);
        m_btnGroupSceneSets->addButton(btn, sceneIdx);   // Assign sceneIdx as `id` in this group, then can be access via QButtonGroup::button() later directly.
        btn->setCheckable(true);   // Make it act like radio button
        if (sceneIdx == 0) { btn->setChecked(true); }
        btn->setText(QString::number(sceneIdx));
        m_toolbarSceneSets->addWidget(btn);
        connect(btn, &QAbstractButton::clicked, this, [=, this](){ switchToScene(sceneIdx, false); });
    }
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
    cycleTogglePaletteSetBtn->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    cycleTogglePaletteSetBtn->setIcon(QIcon::fromTheme("palettes"));
    TranslationManager::instance().registerLanguageChangeCallback(cycleTogglePaletteSetBtn, [=, this]() {
        m_toolbarColorSets->setWindowTitle(tr("Palette Sets"));   // Show in context menu of this ToolBar
        cycleTogglePaletteSetBtn->setText(tr("Palette Sets"));
        cycleTogglePaletteSetBtn->setToolTip(tr("Switch palette set (F3/F4)"));
    });
    m_toolbarColorSets->addWidget(cycleTogglePaletteSetBtn);
    connect(cycleTogglePaletteSetBtn, &QAbstractButton::clicked, this, [this](bool checked){
        Q_UNUSED(checked);
        switchPaletteBy(1);
    });
    for (size_t paletteIdx = 0; paletteIdx < m_archiveFile->paletteAmountOfFirstKcfInCurCnf(); paletteIdx++) {
        QToolButton *btn = new QToolButton(m_toolbarColorSets);
        m_btnGroupColorSets->addButton(btn, paletteIdx);    // Assign paletteIdx as `id` in this group, then can be access via QButtonGroup::button() later directly.
        btn->setCheckable(true);   // Make it act like radio button
        if (paletteIdx == 0) { btn->setChecked(true); }
        btn->setText(QString::number(paletteIdx));
        m_toolbarColorSets->addWidget(btn);
        connect(btn, &QAbstractButton::clicked, this, [=, this](){ switchToPalette(paletteIdx, false); });
    }
}

void MainWindow::switchToScene(uint8_t sceneIdx, bool requestFromFkissScript)
{
    QAbstractButton *btn = m_btnGroupSceneSets->button(sceneIdx);
    if (btn == nullptr) { return; }
    btn->setChecked(true);   // NOTE: DO NOT USE `setDown()` here, it's merely a visual state, and doesn't effect `checked` state. May cause unexpected behavior when programatically triggering `click` event.
    m_archiveFile->setCnfSetNo(sceneIdx);   // REFACTOR: Disastrous logic. Only used by set/palette toolbar currently... I guess storing in GKissView may be enough?
    m_kissView->showKissSceneByIndex(sceneIdx);
    auto paletteNoInKcf = m_kissView->curKissScene()->paletteNoInKcf();
    if (!requestFromFkissScript) {   // triggered by user (from QWidget)
        m_archiveFile->setCnfSetNoFromMainWindow(sceneIdx);
    }
    switchToPalette(paletteNoInKcf, requestFromFkissScript);
}

void MainWindow::switchToPalette(uint8_t paletteIdx, bool requestFromFkissScript)
{
    QAbstractButton *btn = m_btnGroupColorSets->button(paletteIdx);
    if (btn == nullptr) { return; }
    btn->setChecked(true);   // NOTE: DO NOT USE `setDown()` here, it's merely a visual state, and doesn't effect `checked` state. May cause unexpected behavior when programatically triggering `click` event.
    m_archiveFile->setPaletteNo(paletteIdx);
    if (!requestFromFkissScript) {   // triggered by user (from QWidget)
        m_archiveFile->setPaletteNoFromMainWindow(paletteIdx);
    }
    m_kissView->curKissScene()->setPaletteNoInKcfAndRender(paletteIdx);
}

void MainWindow::switchSceneBy(int steps)
{
    if (m_archiveFile->curCnfSetsAmount() == 0) { return; }
    uint8_t curSceneNo = m_archiveFile->curCnfSetNo();
    int max = m_archiveFile->curCnfSetsAmount();
    int nextSceneNo = (curSceneNo + steps) % max;
    if (nextSceneNo < 0) {
        nextSceneNo += max;
    }
    if (curSceneNo == nextSceneNo) { return; }
    qkDebug << "[DEBUG] switchSceneBy()" << nextSceneNo;
    QAbstractButton *btn = m_btnGroupSceneSets->button(nextSceneNo);
    btn->click();
}

void MainWindow::switchPaletteBy(int steps)
{
    if (m_archiveFile->paletteAmountOfFirstKcfInCurCnf() == 0) { return; }
    uint8_t curPaletteNo = m_archiveFile->curPaletteNo();
    int nextPaletteNo = (curPaletteNo + steps) % m_archiveFile->paletteAmountOfFirstKcfInCurCnf();
    if (nextPaletteNo < 0) {
        nextPaletteNo = m_archiveFile->paletteAmountOfFirstKcfInCurCnf() - nextPaletteNo;
    }
    if (curPaletteNo == nextPaletteNo) { return; }
    QAbstractButton *btn = m_btnGroupColorSets->button(nextPaletteNo);
    btn->click();
}

void MainWindow::setupMenuBar()
{
    const QStringList &allEncodingNames = QKiSS::encodingNames();
    QActionGroup *encodingsGroup = new QActionGroup {ui->menu_encoding};    // NOTE: `new` is required. A local instance will act not as you've expected.
    for (int i = 0; i < allEncodingNames.length(); i++) {
        QKiSS::Encoding enumVal = static_cast<QKiSS::Encoding>(i);
        const QString &encName = allEncodingNames[i];
        QAction * act = ui->menu_encoding->addAction(encName);
        encodingsGroup->addAction(act);
        act->setCheckable(true);
        if (m_cfg.encoding() == enumVal) {
            act->setChecked(true);
        }
        connect(act, &QAction::toggled, this, [=, this] () { m_cfg.setEncoding(enumVal); });
    }
    if (ConfigFile::engOnly()) {
        ui->menu_uiLanguage->menuAction()->setVisible(false);
    }
    const auto &allLangs = m_cfg.allLanguages();
    QActionGroup *langsGroup = new QActionGroup {ui->menu_uiLanguage};
    for (const auto &lang : allLangs) {
        // const QString langText = QStringLiteral("%1 %2").arg(lang.flagEmoji).arg(lang.label);
        const QString langText = lang.label;
        QIcon langIcon { lang.flagSvgFile };
        QAction * act = ui->menu_uiLanguage->addAction(langIcon, langText);
        langsGroup->addAction(act);
        act->setCheckable(true);
        if (m_cfg.uiLanguage() == lang.id) {
            act->setChecked(true);
        }
        connect(act, &QAction::toggled, this, [=, this] () { m_cfg.setUiLanguage(lang.id); });
    }

    const auto &allCaseSensitivity= QKiSS::caseSensitivityNames();
    QActionGroup *caseSensitivityGroup = new QActionGroup {ui->menu_caseSensitivity};
    for (const auto &x : allCaseSensitivity) {
        QKiSS::Case key = x.first;
        const QString &label = x.second;
        QAction *act = ui->menu_caseSensitivity->addAction(label);
        caseSensitivityGroup->addAction(act);
        act->setCheckable(true);
        if (m_cfg.caseSensitivity() == key) {
            act->setChecked(true);
        }
        connect(act, &QAction::toggled, this, [=, this] () { m_cfg.setCaseSensitivity(key); });
    }
    TranslationManager::instance().registerLanguageChangeCallback(this, [this]() {
        const QStringList &allEncodingNames = QKiSS::encodingNames();
        for (int i = 0; i < allEncodingNames.length(); i++) {
            QAction *act = ui->menu_encoding->actions().at(i);
            const auto &enc = allEncodingNames.at(i);
            act->setText(enc);
        }
        const auto &allLangs = m_cfg.allLanguages();
        for (int i = 0; i < allLangs.length(); i++) {
            QAction *act = ui->menu_uiLanguage->actions().at(i);
            const auto &lang = allLangs.at(i);
            // const QString langLabel = QStringLiteral("%1 %2").arg(lang.flagEmoji).arg(lang.label);
            const QString langText = lang.label;
            act->setText(langText);
            QIcon langIcon { lang.flagSvgFile };   // The flag of ja_JP also changes with the language.
            act->setIcon(langIcon);
        }
        const auto &allCaseSensitivity= QKiSS::caseSensitivityNames();
        for (int i = 0; i < allCaseSensitivity.length(); i++) {
            QAction *act = ui->menu_caseSensitivity->actions().at(i);
            const auto &cs = allCaseSensitivity.at(i);
            act->setText(cs.second);
        }
    });
}

void MainWindow::setupToolBars()
{
    // File
    m_toolbarFiles = new QToolBar(this);
    m_toolbarFiles->setWindowTitle(tr("Files"));   // The text in context menu when you right-click on toolbar.
    m_toolbarFiles->setObjectName("MainWindow_ToolBar_Files");
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarFiles);
    m_toolbarFiles->addAction(ui->action_openFile);
    m_toolbarFiles->addAction(ui->action_openDirectory);
    // Zoom
    ui->action_zoomReset->setIcon(QIcon::fromTheme("zoom-original"));
    m_toolbarZooms = new QToolBar(this);
    m_toolbarZooms->setWindowTitle(tr("Zooming Tools"));
    m_toolbarZooms->setObjectName("MainWindow_ToolBar_Zooms");
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarZooms);
    m_cboxZoomScale = new QComboBox(m_toolbarZooms);
    m_cboxZoomScale->installEventFilter(this);  // When cbox received an event, pass it to `this` first (to let `this->eventFilter()` handle it).
    m_cboxZoomScale->setEditable(true);
    m_cboxZoomScale->setCurrentText("100%");
    for (qreal factor : m_cfg.zoomFactorList()) {
        QString percentStr = ConfigFile::stringifyZoomFactorToPercent(factor);
        m_cboxZoomScale->addItem(percentStr, QVariant(percentStr));
    }
    m_toolbarZooms->addAction(ui->action_zoomOut);
    m_toolbarZooms->addAction(ui->action_zoomIn);
    m_toolbarZooms->addWidget(m_cboxZoomScale);
    // Dock Window
    m_toolbarWindows = new QToolBar(this);
    m_toolbarWindows->setWindowTitle(tr("Windows"));
    m_toolbarWindows->setObjectName("MainWindow_ToolBar_Windows");   // objectName is required when `QMainWindow::saveState()`
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarWindows);
    m_toolbarWindows->addAction(ui->dockWidget_fileSystemBrowser->toggleViewAction());
    m_toolbarWindows->addAction(ui->dockWidget_innerFileSelector->toggleViewAction());
    m_toolbarWindows->addAction(ui->dockWidget_docViewer->toggleViewAction());
    m_toolbarWindows->addAction(ui->dockWidget_cnfCodeViewer->toggleViewAction());
    m_toolbarWindows->addAction(ui->dockWidget_logViewer->toggleViewAction());
    m_toolbarWindows->addAction(ui->dockWidget_itemInspector->toggleViewAction());
    ui->menu_Window->addAction(ui->dockWidget_fileSystemBrowser->toggleViewAction());
    ui->menu_Window->addAction(ui->dockWidget_innerFileSelector->toggleViewAction());
    ui->menu_Window->addAction(ui->dockWidget_docViewer->toggleViewAction());
    ui->menu_Window->addAction(ui->dockWidget_cnfCodeViewer->toggleViewAction());
    ui->menu_Window->addAction(ui->dockWidget_logViewer->toggleViewAction());
    ui->menu_Window->addAction(ui->dockWidget_itemInspector->toggleViewAction());
    TranslationManager::instance().registerLanguageChangeCallback(this, [=, this]() {
        m_toolbarFiles->setWindowTitle(tr("Files"));   // The text in context menu when you right-click on toolbar.
        m_toolbarZooms->setWindowTitle(tr("Zooming Tools"));
        m_toolbarWindows->setWindowTitle(tr("Windows"));
    });
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
    qApp->setStyle(new DockWidgetProxyStyle{});
    setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);  // Make bottom dock cannot stretch to bottom-left corner.
    // tabifyDockWidget(ui->dockWidget_logViewer, ui->dockWidget_textFileViewer);
    // ui->dockWidget_logViewer->raise();  // activate "logs viewer" tab.
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, ui->dockWidget_cnfCodeViewer);
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, ui->dockWidget_logViewer);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, ui->dockWidget_itemInspector);
    // ==================================================
    // File System Browser
    // ==================================================
    m_fileSystemBrowser = new FileSystemBrowser(this);
    ui->dockWidget_fileSystemBrowser->setWidget(m_fileSystemBrowser);
    ui->dockWidget_fileSystemBrowser->toggleViewAction()->setIcon(QIcon::fromTheme("system-file-manager"));
    // ui->dockWidget_fileSystemBrowser->resize(270, ui->dockWidget_fileSystemBrowser->height());

    // ==================================================
    // Inner Files Browser
    // ==================================================
    m_innerFilesBrowser = new InnerFilesBrowser(this);
    ui->dockWidget_innerFileSelector->setWidget(m_innerFilesBrowser);
    ui->dockWidget_innerFileSelector->toggleViewAction()->setIcon(QIcon::fromTheme("package-unzipped"));

    // ==================================================
    // Log Viewer
    // ==================================================
    m_logViewer = new LogViewer(this);
    ui->dockWidget_logViewer->setWidget(m_logViewer);
    ui->dockWidget_logViewer->toggleViewAction()->setIcon(QIcon::fromTheme("logviewer", QIcon::fromTheme("utilities-system-monitor")));

    // ==================================================
    // Code Editor
    // ==================================================
    m_codeEditor = new CodeEditor(this);
    ui->dockWidget_cnfCodeViewer->setWidget(m_codeEditor);
    ui->dockWidget_cnfCodeViewer->toggleViewAction()->setIcon(QIcon::fromTheme("applications-development"));

    // ==================================================
    // Doc Viewer
    // ==================================================
    QFont fontMono {"Monospace"};
    fontMono.setStyleHint(QFont::TypeWriter);
    m_docViewer= new QPlainTextEdit(this);
    m_docViewer->setReadOnly(true);
    m_docViewer->setFont(fontMono);
    ui->dockWidget_docViewer->setWidget(m_docViewer);
    ui->dockWidget_docViewer->toggleViewAction()->setIcon(QIcon::fromTheme("text-x-readme", QIcon::fromTheme("accessories-text-editor")));

    // ==================================================
    // Item Inspector
    // ==================================================
    m_itemInspector = new ItemInspector(this);
    ui->dockWidget_itemInspector->setWidget(m_itemInspector);
    ui->dockWidget_itemInspector->toggleViewAction()->setIcon(QIcon::fromTheme("inspector"));
    // ==================================================
    // (Signals)
    // ==================================================
    connect(m_innerFilesBrowser, &InnerFilesBrowser::itemActivated, this, [=, this](QTreeWidgetItem *item) {
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

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(tr("QKiSS ready."));

    appendPermanentStatusBarIcon("keyboard-ctrl", 22);
    appendPermanentStatusBarIcon("mouse-left-click", 22);
    appendPermanentStatusBarText([](){ return tr("Ctrl+Drag: Force split CEL from Object"); } );
    appendPermanentStatusBarSeparator();

    appendPermanentStatusBarIcon("keyboard-shift", 22);
    appendPermanentStatusBarIcon("mouse-left-click", 22);
    appendPermanentStatusBarText([](){ return tr("Shift+Drag: Disable FKiSS events"); } );
    appendPermanentStatusBarSeparator();


    appendPermanentStatusBarIcon("mouse-middle-click", 22);
    appendPermanentStatusBarText([](){ return tr("Middle Click: Unfix"); } );
    appendPermanentStatusBarSeparator();


    appendPermanentStatusBarIcon("mouse-right-click", 22);
    appendPermanentStatusBarText([](){ return tr("Right Click: More Actions"); } );
    appendPermanentStatusBarSeparator();

}

QLabel *MainWindow::appendPermanentStatusBarIcon(QString iconName, uint size)
{
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon::fromTheme(iconName).pixmap(size, size));
    statusBar()->addPermanentWidget(iconLabel);
    return iconLabel;
}

ZLabel *MainWindow::appendPermanentStatusBarText(const std::function<QString()> &textGetter)
{
    ZLabel *label = new ZLabel(this);
    label->setText(textGetter);
    statusBar()->addPermanentWidget(label);
    return label;
}

QFrame *MainWindow::appendPermanentStatusBarSeparator()
{
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    statusBar()->addPermanentWidget(separator);
    return separator;
}



void MainWindow::setupShortcuts()
{
    QShortcut *f1 = new QShortcut(QKeySequence(Qt::Key_F1), this);
    QShortcut *f2 = new QShortcut(QKeySequence(Qt::Key_F2), this);
    QShortcut *f3 = new QShortcut(QKeySequence(Qt::Key_F3), this);
    QShortcut *f4 = new QShortcut(QKeySequence(Qt::Key_F4), this);
    connect(f1, &QShortcut::activated, this, [this](){ this->switchSceneBy(-1); });
    connect(f2, &QShortcut::activated, this, [this](){ this->switchSceneBy(1); });
    connect(f3, &QShortcut::activated, this, [this](){ this->switchPaletteBy(-1); });
    connect(f4, &QShortcut::activated, this, [this](){ this->switchPaletteBy(1); });
}

void MainWindow::saveWindowSettings()
{
    QSettings set;
    m_cfg.setMainWindowGeometry(saveGeometry());
    m_cfg.setMainWindowState(saveState());
    qreal factor = ConfigFile::parseZoomHumanToFactor(m_cboxZoomScale->currentText());
    m_cfg.setZoom(factor);
}

void MainWindow::restoreWindowSettings()
{
    restoreGeometry(m_cfg.mainWindowGeometry());
    restoreState(m_cfg.mainWindowState());
    m_cboxZoomScale->setCurrentText(m_cfg.zoomAsPercent());
    renderRecentFiles();
}

void MainWindow::retranslate()
{
    ui->retranslateUi(this);
}

void MainWindow::renderRecentFiles()
{
    // Files
    auto menuRecentFiles = ui->menu_openRecentFile;
    // qkDebug << "[Render] Recent File = " << m_recentFiles;
    menuRecentFiles->setIcon(QIcon::fromTheme("recents"));
    menuRecentFiles->clear();
    foreach (const QString &path, m_cfg.recentFiles()) {
        QAction *act = new QAction(menuRecentFiles);
        act->setText(path);
        connect(act, &QAction::triggered, this, [=, this]() { openDirOrFile(path); });
        menuRecentFiles->addAction(act);
    }
    menuRecentFiles->addSeparator();
    QAction *actClearFile = new QAction(menuRecentFiles);
    actClearFile->setText(tr("Clear All"));
    actClearFile->setIcon(QIcon::fromTheme("edit-clear"));
    connect(actClearFile, &QAction::triggered, &m_cfg, &ConfigFile::clearRecentFiles);
    menuRecentFiles->addAction(actClearFile);
    // Dirs
    // qkDebug << "[Render] Recent Dirs = " << m_recentDirs;
    auto menuRecentDirs = ui->menu_openRecentDirectory;
    // menuRecentDirs->setIcon(QIcon::fromTheme("recents"));
    menuRecentDirs->clear();
    foreach (const QString &path, m_cfg.recentDirs()) {
        QAction *act = new QAction(menuRecentDirs);
        act->setText(path);
        connect(act, &QAction::triggered, this, [=, this]() { openDirOrFile(path); });
        menuRecentDirs->addAction(act);
    }
    menuRecentDirs->addSeparator();
    QAction *actClearDir = new QAction(menuRecentDirs);
    actClearDir->setText(tr("Clear All"));
    actClearDir->setIcon(QIcon::fromTheme("edit-clear"));
    connect(actClearDir, &QAction::triggered, &m_cfg, &ConfigFile::clearRecentDirs);
    menuRecentDirs->addAction(actClearDir);
}



void MainWindow::openFilePicker()
{
    QString filter = tr("LZH Archive Files (*.lzh)");
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Compressed KiSS File"),
                                                    m_fileSystemBrowser->curDirPath(),
                                                    filter,
                                                    nullptr
                                                    // QFileDialog::Option::DontUseNativeDialog
                                                    );
    if (!fileName.isEmpty()) {
        qkDebug << "===============" << fileName;
        openDirOrFile(fileName);
    }
}

void MainWindow::openDirPicker()
{
    QString dirPath = QFileDialog::getExistingDirectory(this,
                                                    tr("Open Directory"),
                                                    m_fileSystemBrowser->curDirPath(),
                                                    QFileDialog::ShowDirsOnly // | QFileDialog::DontUseNativeDialog
                                                    );
    if (!dirPath.isEmpty()) {
        qkDebug << "===============" << dirPath;
        openDirOrFile(dirPath);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_cboxZoomScale && event->type() == QEvent::Wheel) {
        // if dropdown menu of this combobox is expanded, do default behavior.
        if (m_cboxZoomScale->view()->isVisible()) {
            return QMainWindow::eventFilter(watched, event);
        }
        QWheelEvent *ev = static_cast<QWheelEvent *>(event);
        if (ev->angleDelta().y() > 0) {  // scroll up
            m_cfg.zoomInByPredefinedSteps();
        } else {  // scroll down
            m_cfg.zoomOutByPredefinedSteps();
        }
        return true;  // Capture this event.
    }
    return QMainWindow::eventFilter(watched, event);
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
                qkDebug << "Dropped file:" << filePath;
                tryToLoadKissFilePath(filePath, QKiSS::FileLoadMode::LoadAsIndependent);
                // m_archiveProcessor->extractLzh(filePath);
            }
        }
    }
    event->acceptProposedAction();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveWindowSettings();
    m_cfg.saveAll();
    m_logViewer = nullptr;   // Avoid potential segfault when closing window
    QMainWindow::closeEvent(event);
}



