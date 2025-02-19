#ifndef FILESYSTEMBROWSER_H
#define FILESYSTEMBROWSER_H

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
    bool setDirPathByFileName(QString filePath);
    bool setDirPath(QString dirPath);
private:
    void setupUi();
    void updateModel();
    static QString formatFileSize(qint64 bytes);

private slots:
    void goUp();
    void refresh();
    void onItemClicked(const QModelIndex &index);
signals:
    void requestLoadFile(QString filePath, bool loadAsPatch);
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
