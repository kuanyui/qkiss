#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QSet>
#include "filesystembrowser.h"
#include "fileiconprovider.h"
#include "iconbuttondelegate.h"

FileSystemBrowser::FileSystemBrowser(QWidget *parent):
    QWidget{parent},
    m_fileIconProvider{ new FileIconProvider() }
{
    setupUi();
    setDirPath(QDir::currentPath());
}

FileSystemBrowser::~FileSystemBrowser()
{
    delete m_fileIconProvider;
}

bool FileSystemBrowser::setPath(QString filePath)
{
    QFileInfo fInfo {filePath};
    if (!fInfo.exists()) {
        qWarning() << "[ERROR][filesystembrowser] file path not found, aborted.";
        return false;
    }
    if (fInfo.isFile()) {
        QDir dir = fInfo.dir();
        m_curDirPath = dir.absolutePath();
        m_labelCurDirName->setText(dir.dirName());
        updateModel();
        return true;
    }
    if (fInfo.isDir()) {
        // FileInfo::absolutePath() is wrong when `dirPath` in `FileInfo(dirPath)` is not ends with `/`.
        if (!filePath.endsWith("/")) {
            fInfo = QFileInfo(filePath + "/");
        }
        m_curDirPath = fInfo.absolutePath();
        m_labelCurDirName->setText(fInfo.fileName());
        updateModel();
        return true;
    }
    qCritical() << "[ERROR] unhandled error.";
    return false;
}


bool FileSystemBrowser::setDirPath(QString dirPath)
{
    QFileInfo dInfo {dirPath};
    if (!dInfo.exists() || !dInfo.isDir()) {
        qWarning() << "[ERROR][filesystembrowser] Invalid dir path, aborted.";
        return false;
    }
    m_curDirPath = dInfo.absoluteFilePath();
    m_labelCurDirName->setText(dInfo.fileName());
    updateModel();
    return true;
}

void FileSystemBrowser::markPathAsLoaded(QString path, QKiSS::FileLoadMode fileLoadMode)
{
    QStringList oriLoadedKissFilePaths = m_loadedKissFilePaths;
    switch (fileLoadMode) {
    case QKiSS::FileLoadMode::LoadAsIndependent: {
        m_loadedKissFilePaths.clear();
        m_loadedKissFilePaths.append(path);
        break;
    }
    case QKiSS::FileLoadMode::LoadAsPatch: {
        m_loadedKissFilePaths.append(path);
        break;
    }
    }
    QSet<QString> oriPaths = { oriLoadedKissFilePaths.begin(), oriLoadedKissFilePaths.end() };
    QSet<QString> newPaths = { m_loadedKissFilePaths.begin(), m_loadedKissFilePaths.end() };
    QSet<QString> newlyUnmarkedPaths = oriPaths - newPaths;
    QSet<QString> newlyMarkedPaths = newPaths - oriPaths;
    QSet<QString> allChangedPaths = oriPaths + newPaths;
    for (int r = 0; r < m_model->rowCount(); r++) {
        QStandardItem *firstCol = m_model->item(r, 0);
        QStandardItem *stateIconItem = m_model->item(r, ColumnIdx::ColIdxStateIcon);
        QStandardItem *fileNameItem = m_model->item(r, ColumnIdx::ColIdxFileName);
        QString itemFilePath = firstCol->data(ThisRole::FilePathRole).toString();
        if (newlyMarkedPaths.contains(itemFilePath) || newlyUnmarkedPaths.contains(itemFilePath)) {
            fileNameItem->setIcon(getFileIcon(itemFilePath));   // packed / unpacked
        }
        if (allChangedPaths.contains(itemFilePath)) {
            stateIconItem->setIcon(getLoadStateIcon(itemFilePath));    // independent / patch
        }
    }
}

void FileSystemBrowser::setupUi()
{

    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(4);
    m_model->setHorizontalHeaderLabels({ "", tr("File Name"), tr("Size"), tr("Patch") });  // model.clear() will also delete headers... WTF...

    m_view = new QTreeView(this);
    m_view->setModel(m_model);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);  // Make cells read-only
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setRootIsDecorated(false);
    m_view->header()->setStretchLastSection(false);
    m_view->header()->setDefaultSectionSize(30);
    m_view->header()->setMinimumSectionSize(10);
    m_view->header()->setSectionResizeMode(QHeaderView::Interactive);
    m_view->header()->resizeSection(ColumnIdx::ColIdxStateIcon, 22);
    m_view->header()->resizeSection(ColumnIdx::ColIdxFileName, 150);
    m_view->header()->resizeSection(ColumnIdx::ColIdxSize, 50);
    // m_view->resizeColumnToContents(2);
    m_view->header()->setSectionResizeMode(ColumnIdx::ColIdxStateIcon, QHeaderView::ResizeMode::Fixed);
    m_view->header()->setSectionResizeMode(ColumnIdx::ColIdxPatchBtn, QHeaderView::ResizeMode::ResizeToContents);
    m_view->setMouseTracking(true);   // NOTE: Set to true to make QStyledItemDelegate::editorEvent accepts QEvent::MouseMove without QEvent::MouseButtonPress
    // Delegate
    IconButtonDelegate *patchDelegate = new IconButtonDelegate(m_view);
    m_view->setItemDelegateForColumn(ColumnIdx::ColIdxPatchBtn, patchDelegate);
    connect(patchDelegate, &IconButtonDelegate::pressed, [=](const QModelIndex& index){
        QString filePath = m_model->data(index, ThisRole::FilePathRole).toString();
        if (filePath.isEmpty()) { return; }
        QFileInfo fileInfo {filePath};
        qDebug() << "debug" << filePath;
        if (fileInfo.isDir()) {
            // TODO: load folder as patch
        } else {
            emit requestLoadFile(filePath, QKiSS::FileLoadMode::LoadAsPatch);
        }
    });

    // int iconSize = style()->pixelMetric(QStyle::PM_ButtonIconSize);
    m_btnGoUp = new QToolButton(this);
    m_btnGoUp->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    m_btnGoUp->setToolTip(tr("Go Up"));
    m_btnGoUp->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_btnRefresh = new QToolButton(this);
    m_btnRefresh->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    m_btnRefresh->setToolTip(tr("Refresh"));
    m_btnRefresh->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_labelCurDirName = new QLabel(this);
    // m_labelCurDirName->setSizePolicy()

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_btnGoUp);
    btnLayout->addWidget(m_btnRefresh);
    btnLayout->addWidget(m_labelCurDirName);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(m_view);

    connect(m_btnGoUp, &QPushButton::clicked, this, &FileSystemBrowser::goUp);
    connect(m_btnRefresh, &QPushButton::clicked, this, &FileSystemBrowser::refresh);
    connect(m_view, &QTreeView::doubleClicked, this, &FileSystemBrowser::onItemDbClicked);
}

void FileSystemBrowser::updateModel()
{
    // m_model->clear();   // DO NOT USE THIS. This will clear header title, header size...etc
    m_model->removeRows(0, m_model->rowCount());
    QDir dir { m_curDirPath };
    QFileInfoList fInfoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::IgnoreCase);
    foreach (const QFileInfo &fInfo, fInfoList) {
        const QString ext = fInfo.fileName().toUpper();
        QString fileAbsPath = fInfo.absoluteFilePath();
        bool isDir = fInfo.isDir();
        bool isLzh = ext.endsWith(".LZH");
        bool isCnf = ext.endsWith(".CNF");
        if (!isDir && (!isLzh && !isCnf)) {
            continue;
        }
        QList<QStandardItem *> rowItems;
        QStandardItem *stateItem = new QStandardItem();
        stateItem->setData(fileAbsPath, ThisRole::FilePathRole);
        stateItem->setIcon(getLoadStateIcon(fileAbsPath));
        rowItems << stateItem;
        QStandardItem *fileNameItem = new QStandardItem(fInfo.fileName());
        fileNameItem->setIcon(getFileIcon(fInfo));
        rowItems << fileNameItem;
        rowItems << new QStandardItem(formatFileSize(fInfo.size()));
        if (isLzh) {
            QStandardItem *item = new QStandardItem(tr("Patch"));
            QIcon patchIcon = QIcon::fromTheme("addon");
            item->setIcon(patchIcon);
            item->setToolTip(tr("Click to load this file as patch"));
            item->setData(fInfo.filePath(), ThisRole::FilePathRole);    // REFACTOR: Redundant data, consider how to reuse it.
            rowItems << item;
        }
        m_model->appendRow(rowItems);
    }
}

QString FileSystemBrowser::formatFileSize(qint64 bytes) {
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;

    if (bytes >= gb) {
        return QString::number(double(bytes) / gb, 'f', 1) + " GB";
    } else if (bytes >= mb) {
        return QString::number(double(bytes) / mb, 'f', 0) + " MB";
    } else if (bytes >= kb) {
        return QString::number(double(bytes) / kb, 'f', 0) + " KB";
    } else {
        return QString::number(bytes) + " B";
    }
}

QIcon FileSystemBrowser::getFileIcon(const QFileInfo &fileInfo) const
{
    QString absPath = fileInfo.absoluteFilePath();
    if (m_loadedKissFilePaths.contains(absPath)) {
        bool isLzh = absPath.toUpper().endsWith(".LZH");
        if (isLzh) {
            return QIcon::fromTheme("package-unzipped");
        }
        if (fileInfo.isDir()) {
            return QIcon::fromTheme("folder-open");
        }
    }
    return m_fileIconProvider->icon(fileInfo);
}

QIcon FileSystemBrowser::getFileIcon(const QString &filePath) const
{
    return getFileIcon(QFileInfo{filePath});
}

QIcon FileSystemBrowser::getLoadStateIcon(const QFileInfo &fileInfo) const
{
    return getLoadStateIcon(fileInfo.absoluteFilePath());
}

QIcon FileSystemBrowser::getLoadStateIcon(const QString &filePath) const
{
    int idx = m_loadedKissFilePaths.indexOf(filePath);
    if (idx == -1) {
        return QIcon();
    } else if (idx == 0) {
        return QIcon::fromTheme("qkiss");
    } else {
        return QIcon::fromTheme("addon");
    }
}



void FileSystemBrowser::goUp() {
    QDir dir { m_curDirPath };
    dir.cdUp();
    setDirPath(dir.path());
}

void FileSystemBrowser::refresh() {
    updateModel();
}

void FileSystemBrowser::onItemDbClicked(const QModelIndex &index) {
    if (!index.isValid()) return;
    QModelIndex indexOfFirstCol = m_model->index(index.row(), 0);  // choose the first column (col = 0)
    QString filePath = m_model->data(indexOfFirstCol, ThisRole::FilePathRole).toString();
    QFileInfo fileInfo { filePath };
    if (fileInfo.isDir()) {
        setDirPath(filePath);
    } else {
        emit requestLoadFile(filePath, QKiSS::FileLoadMode::LoadAsIndependent);
    }
}
