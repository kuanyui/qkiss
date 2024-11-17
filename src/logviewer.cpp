#include "logviewer.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

LogViewer::LogViewer(QWidget *parent)
    : QWidget{parent},
      m_treeWidget{new QTreeWidget{this}}
{
    setupLayout();
}

void LogViewer::appendLog(QtMsgType type, const QString &msg)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    switch (type) {
        case QtMsgType::QtDebugMsg:
        item->setText(0, "🐛 Debug");
        item->setForeground(0, QBrush(Qt::gray));
        case QtMsgType::QtInfoMsg:
        item->setText(0, "ℹ️ Info");
        item->setForeground(0, QBrush(QColor("#61AFEF")));
        break;
        case QtMsgType::QtWarningMsg:
        item->setText(0, "⚠️ Warning");
        item->setForeground(0, QBrush(Qt::yellow));
        break;
        case QtMsgType::QtCriticalMsg:
        item->setText(0, "⛔ Error");
        item->setForeground(0, QBrush(Qt::red));
        case QtMsgType::QtFatalMsg:
        item->setText(0, "⛔ Fatal");
        item->setForeground(0, QBrush(Qt::red));
        break;
    }
    item->setText(1, msg);
    m_treeWidget->addTopLevelItem(item);
}

void LogViewer::clearAllLogs()
{
    m_treeWidget->clear();
}

void LogViewer::setupLayout()
{
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setColumnCount(2);
    m_treeWidget->setHeaderLabels({"Severity", "Message"});
    m_treeWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);   // Let user can scroll to the middle of a row, if the row contains multiple lines.
    // m_treeWidget->setHeaderLabels({tr""});
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_treeWidget);
}

