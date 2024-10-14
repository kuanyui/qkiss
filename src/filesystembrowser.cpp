#include "filesystembrowser.h"

#include <QFile>

FileSystemBrowser::FileSystemBrowser(QWidget *parent):
    QWidget{parent}
{
    setupUi();
}

bool FileSystemBrowser::setDirPathByFileName(QString filePath)
{
    QFileInfo fInfo {filePath};
    if (!fInfo.exists() || !fInfo.isFile()) { return false; }
    QDir dir = fInfo.dir();
    m_model->setRootPath(dir.absolutePath());
    return true;
}


bool FileSystemBrowser::setDirPath(QString dirPath)
{
    QFileInfo info {dirPath};
    if (!info.exists() || !info.isDir()) { return false; }
    m_model->setRootPath(info.absolutePath());
    return true;
}

void FileSystemBrowser::setupUi()
{
    m_model = new QFileSystemModel(this);
    m_model->setRootPath(QDir::rootPath());

    m_view = new QTableView(this);
    m_view->setModel(m_model);
    m_view->setRootIndex(m_model->index(QDir::rootPath()));

    m_view->setColumnHidden(1, true);
    m_view->setColumnHidden(3, true);
    m_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_view->verticalHeader()->hide();

    m_btnGoUp = new QPushButton("Up", this);
    m_btnRefresh = new QPushButton("Refresh", this);
    m_labelCurDirName = new QLabel(this);

    // 創建 layout
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
    connect(m_model, &QFileSystemModel::rootPathChanged, [=](const QString &newPath) {
        QDir dir {newPath};
        m_labelCurDirName->setText(dir.dirName());
    });
}

void FileSystemBrowser::goUp() {
    QModelIndex currentIndex = m_view->rootIndex();
    QModelIndex parentIndex = currentIndex.parent();
    if (parentIndex.isValid()) {
        m_view->setRootIndex(parentIndex);
    }
}

void FileSystemBrowser::refresh() {
    m_model->setRootPath(m_model->filePath(m_view->rootIndex()));
}

void FileSystemBrowser::onItemClicked(const QModelIndex &index) {
    QString chosenPath = m_model->filePath(index);
    if (m_model->isDir(index)) {
        setDirPath(chosenPath);
    } else {

    }
}
