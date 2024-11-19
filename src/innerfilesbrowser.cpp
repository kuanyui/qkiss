#include <QTreeWidgetItem>
#include "innerfilesbrowser.h"
using namespace Qt::Literals::StringLiterals;

InnerFilesBrowser::InnerFilesBrowser(QWidget *parent)
    : QTreeWidget{parent}
{

}

void InnerFilesBrowser::rerenderFileList(ArchiveFile *archiveFile)
{
    clear();
    QTreeWidgetItem *rootCnfFolder = new QTreeWidgetItem(this, CustomItemType::CustomItemFolderType);
    rootCnfFolder->setText(0, tr("CNF files"));
    rootCnfFolder->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    QTreeWidgetItem *rootDocFolder = new QTreeWidgetItem(this, CustomItemType::CustomItemFolderType);
    rootDocFolder->setText(0, tr("Document files"));
    rootDocFolder->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    addTopLevelItems({ rootCnfFolder, rootDocFolder });
    foreach (const QString &cnfFileName, archiveFile->getCnfNameList()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(rootCnfFolder, CustomItemType::CustomItemCnfFileType);
        item->setText(0, cnfFileName);
        item->setIcon(0, style()->standardIcon(QStyle::SP_FileLinkIcon));
    }
    rootCnfFolder->setText(0, rootCnfFolder->text(0) + " (%1)"_L1.arg(QString::number(rootCnfFolder->childCount())));
    foreach (const QString &docFileName, archiveFile->getDocFileNameList()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(rootDocFolder, CustomItemType::CustomItemCnfFileType);
        item->setText(0, docFileName);
        item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
    }
    rootDocFolder->setText(0, rootDocFolder->text(0) + " (%1)"_L1.arg(QString::number(rootDocFolder->childCount())));
    QString curCnfFileName = archiveFile->curCnf() ? archiveFile->curCnf()->fileName() : "";
}
