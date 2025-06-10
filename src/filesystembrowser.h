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
#include <QStack>
#include "configfile.h"

class FileIconProvider;
class QFileInfo;

class FileSystemBrowser : public QWidget
{
    Q_OBJECT
    enum ThisRole {
        FilePathRole = Qt::UserRole + 1
    };
private:
    enum ColumnIdx {
        ColIdxStateIcon,
        ColIdxFileName,
        ColIdxSize,
        ColIdxPatchBtn
    };

public:
    FileSystemBrowser(QWidget *parent = nullptr);
    ~FileSystemBrowser();
    /// File name, not dir name
    bool setPath(QString filePath);
    bool setDirPath(QString dirPath);
    QString curDirPath() const;
    /// Add icon to mark this file is loaded, for UX.
    void markPathAsLoaded(QString path, QKiSS::FileLoadMode fileLoadMode);
private:
    void setupUi();
    void updateModel();
    static QString formatFileSize(qint64 bytes);
    QIcon getFileIcon(const QFileInfo &fileInfo) const;
    QIcon getFileIcon(const QString &filePath) const;
    QIcon getLoadStateIcon(const QFileInfo &fileInfo) const;
    QIcon getLoadStateIcon(const QString &filePath) const;
    void updateAllButtonState();
private slots:
    void goUp();
    void goForward();
    void goBackward();
    void refresh();
    void goHome();
    void setCurDirAsHome();
    void setHome(const QString &dirPath);
    void onItemDbClicked(const QModelIndex &index);
signals:
    void requestLoadFile(QString filePath, QKiSS::FileLoadMode fileLoadMode);
private:
    QStandardItemModel *m_model;
    QTreeView *m_view;
    QToolButton *m_btnGoBackward;
    QToolButton *m_btnGoForward;
    QToolButton *m_btnGoUp;
    QToolButton *m_btnRefresh;
    QToolButton *m_btnGoHome;
    QLineEdit *m_lineEditUrl;
    QString m_curDirPath;
    /// Unused currently.
    QLabel *m_labelCurDirName;
    FileIconProvider *m_fileIconProvider;
    /// First element is main archive path, the rests are patches' paths.
    /// Can be a LZH file path, or folder (not supported yet.)
    QList<QString> m_loadedKissFilePaths;
    QStack<QString> m_navBackwardStack;
    QStack<QString> m_navForwardStack;
    ConfigFile &m_cfg;



    // QObject interface
public:
    /// NOTE: I tried to use `mousePressEvent` before, but it seems that the click event are all catched by QTreeView, so use eventFilter instead.
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

};


#endif // FILESYSTEMBROWSER_H
