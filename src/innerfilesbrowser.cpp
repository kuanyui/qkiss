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

#include <QTreeWidgetItem>
#include "innerfilesbrowser.h"
using namespace Qt::Literals::StringLiterals;

InnerFilesBrowser::InnerFilesBrowser(QWidget *parent)
    : QTreeWidget{parent}
{
    setHeaderHidden(true);
}

void InnerFilesBrowser::rerenderFileList(ArchiveFile *archiveFile)
{
    clear();
    QTreeWidgetItem *rootCnfFolder = new QTreeWidgetItem(this, CustomItemType::CustomItemFolderType);
    rootCnfFolder->setExpanded(true);
    rootCnfFolder->setText(0, tr("CNF files"));
    rootCnfFolder->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    QTreeWidgetItem *rootDocFolder = new QTreeWidgetItem(this, CustomItemType::CustomItemFolderType);
    rootDocFolder->setExpanded(true);
    rootDocFolder->setText(0, tr("Document files"));
    rootDocFolder->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    addTopLevelItems({ rootCnfFolder, rootDocFolder });
    foreach (const CPString &cnfFileName, archiveFile->getCnfNameList()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(rootCnfFolder, CustomItemType::CustomItemCnfFileType);
        item->setText(0, cnfFileName.original());
        item->setIcon(0, QIcon::fromTheme("text-x-cnf"));
        // item->setIcon(0, style()->standardIcon(QStyle::SP_FileLinkIcon));
    }
    rootCnfFolder->setText(0, rootCnfFolder->text(0) + " (%1)"_L1.arg(QString::number(rootCnfFolder->childCount())));
    foreach (const CPString &docFileName, archiveFile->getDocFileNameList()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(rootDocFolder, CustomItemType::CustomItemDocFileType);
        item->setText(0, docFileName.original());
        if (isReadme(docFileName.original())) {
            item->setIcon(0, QIcon::fromTheme("text-x-readme"));
        } else {
            item->setIcon(0, QIcon::fromTheme("text-x-generic"));
        }
        // item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
    }
    rootDocFolder->setText(0, rootDocFolder->text(0) + " (%1)"_L1.arg(QString::number(rootDocFolder->childCount())));
    QString curCnfFileName = archiveFile->curCnf() ? archiveFile->curCnf()->fileName().original() : "";
    if (!curCnfFileName.isEmpty()) {
        for (int i = 0; i < rootCnfFolder->childCount(); i++) {
            auto item = rootCnfFolder->child(i);
            if (item->text(0) == curCnfFileName) {
                setCurrentItem(item);
                break;
            }
        }
    }
    if (rootCnfFolder->childCount() != 0) {
        markItemAsLoaded(rootCnfFolder->child(0));
    }
    if (rootDocFolder->childCount() != 0) {
        markItemAsLoaded(rootDocFolder->child(0));
    }
}

void InnerFilesBrowser::markItemAsLoaded(QTreeWidgetItem *item)
{
    QTreeWidgetItem *folder = item->parent();
    switch (item->type()) {
    case CustomItemType::CustomItemCnfFileType: {
        m_curCnfFileName = item->text(0);
        for (int i = 0 ; i < folder->childCount(); i++) {
            QTreeWidgetItem *x = folder->child(i);
            if (x == item) {
                x->setIcon(0, QIcon::fromTheme("play"));
            } else {
                x->setIcon(0, QIcon::fromTheme("text-x-cnf"));
            }
        }
        break;
    }
    case CustomItemType::CustomItemDocFileType: {
        m_curDocFileName = item->text(0);
        for (int i = 0 ; i < folder->childCount(); i++) {
            QTreeWidgetItem *x = folder->child(i);
            if (x == item) {
                x->setIcon(0, QIcon::fromTheme("preview-file"));
            } else {
                x->setIcon(0, QIcon::fromTheme(isReadme(m_curDocFileName) ? "text-x-readme" : "text-x-generic"));  // TODO
            }
        }
        break;
    }
    default:
        break;
    }
}

bool InnerFilesBrowser::isReadme(const QString &fileName) const
{
    QString upper = fileName.toUpper();
    return upper.contains("README") || upper.endsWith("DOC");
}
