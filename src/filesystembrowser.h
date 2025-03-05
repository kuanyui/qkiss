#ifndef FILESYSTEMBROWSER_H
#define FILESYSTEMBROWSER_H

#include "enums.h"
#include <QWidget>
#include <QTreeView>
#include <QPushButton>
#include <QToolButton>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QLabel>

class FileIconProvider;

class FileSystemBrowser : public QWidget
{
    Q_OBJECT
    enum ThisRole {
        FileNameRole = Qt::UserRole + 1
    };
public:
    FileSystemBrowser(QWidget *parent = nullptr);
    ~FileSystemBrowser();
    /// File name, not dir name
    bool setPath(QString filePath);
    bool setDirPath(QString dirPath);
private:
    void setupUi();
    void updateModel();
    static QString formatFileSize(qint64 bytes);

private slots:
    void goUp();
    void refresh();
    void onItemDbClicked(const QModelIndex &index);
signals:
    void requestLoadFile(QString filePath, QKiSS::FileLoadMode fileLoadMode);
private:
    QStandardItemModel *m_model;
    QTreeView *m_view;
    QToolButton *m_btnGoUp;
    QToolButton *m_btnRefresh;
    QString m_curDirPath;
    QLabel *m_labelCurDirName;
    FileIconProvider *m_fileIconProvider;
};


#endif // FILESYSTEMBROWSER_H
