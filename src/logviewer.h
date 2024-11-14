#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QTreeWidget>

class LogViewer : public QWidget
{
    Q_OBJECT
public:
    explicit LogViewer(QWidget *parent = nullptr);
    void appendLog(QtMsgType type, const QString &msg);
    void clearAllLogs();
signals:

private:
    void setupLayout();
    QTreeWidget *m_treeWidget;
};

#endif // LOGVIEWER_H
