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

signals:

};

#endif // INNERFILESBROWSER_H
