#include "filesystembrowser.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QDir>
#include <QFile>


FileSystemBrowser::FileSystemBrowser(QWidget *parent):
    QWidget{parent}
{
    setupUi();
    setDirPath(QDir::currentPath());
}

FileSystemBrowser::~FileSystemBrowser()
{
    delete m_fileIconProvider;
}

bool FileSystemBrowser::setDirPathByFileName(QString filePath)
{
    QFileInfo fInfo {filePath};
    if (!fInfo.exists() || !fInfo.isFile()) {
        qWarning() << "[ERROR][filesystembrowser] Invalid file path, aborted.";
        return false;
    }
    QDir dir = fInfo.dir();
    m_curDirPath = dir.absolutePath();
    m_labelCurDirName->setText(dir.dirName());
    updateModel();
    return true;
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



void FileSystemBrowser::setupUi()
{
    m_fileIconProvider = new QFileIconProvider();

    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(3);
    m_model->setHorizontalHeaderLabels({ tr("File Name"), tr("Size"), tr("Delete") });  // model.clear() will also delete headers... WTF...

    m_view = new QTableView(this);
    m_view->setModel(m_model);
    m_view->verticalHeader()->hide();
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);  // Make cells read-only
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->horizontalHeader()->setDefaultSectionSize(30);
    m_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_view->horizontalHeader()->resizeSection(0, 150);
    m_view->horizontalHeader()->resizeSection(1, 30);
    m_view->horizontalHeader()->resizeSection(2, 30);

    int iconSize = style()->pixelMetric(QStyle::PM_ButtonIconSize);
    m_btnGoUp = new QToolButton(this);
    m_btnGoUp->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    m_btnGoUp->setToolTip("Go Up");
    m_btnGoUp->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_btnRefresh = new QToolButton(this);
    m_btnRefresh->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    m_btnRefresh->setToolTip("Refresh");
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
    connect(m_view, &QTableView::clicked, this, &FileSystemBrowser::onItemClicked);
}

void FileSystemBrowser::updateModel()
{
    // m_model->clear();   // DO NOT USE THIS. This will clear header title, header size...etc
    m_model->removeRows(0, m_model->rowCount());
    QDir dir { m_curDirPath };
    QFileInfoList fInfoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QFileInfo &fInfo, fInfoList) {
        QList<QStandardItem *> rowItems;
        QStandardItem *fileNameItem = new QStandardItem(fInfo.fileName());
        fileNameItem->setData(fInfo.filePath(), ThisRole::FileNameRole);
        fileNameItem->setIcon(m_fileIconProvider->icon(fInfo));
        rowItems << fileNameItem;
        rowItems << new QStandardItem(formatFileSize(fInfo.size()));
        rowItems << new QStandardItem();   // preserved for button
        QPushButton *deleteBtn = new QPushButton("delete");
        m_view->setIndexWidget(m_model->index(m_model->rowCount()-1, 2), deleteBtn);
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

void FileSystemBrowser::goUp() {
    QDir dir { m_curDirPath };
    dir.cdUp();
    setDirPath(dir.path());
}

void FileSystemBrowser::refresh() {
    updateModel();
}

void FileSystemBrowser::onItemClicked(const QModelIndex &index) {
    if (!index.isValid()) return;
    QModelIndex fileNameIndex = m_model->index(index.row(), 0);  // choose the first column (col = 0)
    QString filePath = m_model->data(fileNameIndex, ThisRole::FileNameRole).toString();
    QFileInfo fileInfo { filePath };
    if (fileInfo.isDir()) {
        setDirPath(filePath);
    } else {
        emit selectFile(filePath, false);
    }
}
