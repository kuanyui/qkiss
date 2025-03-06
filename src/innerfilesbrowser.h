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
