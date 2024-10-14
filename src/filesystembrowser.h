#ifndef FILESYSTEMBROWSER_H
#define FILESYSTEMBROWSER_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QLabel>

class FileSystemBrowser : public QWidget
{
    Q_OBJECT
public:
    FileSystemBrowser(QWidget *parent = nullptr);
    bool setDirPathByFileName(QString filePath);
    bool setDirPath(QString dirPath);
private:
    void setupUi();
private slots:
    void goUp();
    void refresh();
    void onItemClicked(const QModelIndex &index);

private:
    QFileSystemModel *m_model;
    QTableView *m_view;
    QPushButton *m_btnGoUp;
    QPushButton *m_btnRefresh;
    QLabel *m_labelCurDirName;
};

#endif // FILESYSTEMBROWSER_H
