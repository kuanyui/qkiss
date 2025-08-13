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

#ifndef INNERFILESBROWSER_H
#define INNERFILESBROWSER_H

#include <QObject>
#include <QTreeWidget>
#include "archivefile.h"

class InnerFilesBrowser : public QTreeWidget
{
    Q_OBJECT
public:
    enum CustomItemType {
        CustomItemFolderType = QTreeWidgetItem::UserType + 1,
        CustomItemCnfFileType,
        CustomItemDocFileType
    };
    explicit InnerFilesBrowser(QWidget *parent = nullptr);
    void rerenderFileList(ArchiveFile *archiveFile);
    void markItemAsLoaded(QTreeWidgetItem *item);
signals:

private:
    bool isReadme(const QString &fileName) const;
    QString m_curCnfFileName;
    QString m_curDocFileName;
};

#endif // INNERFILESBROWSER_H
