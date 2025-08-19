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

#include "logviewer.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include "translationmanager.h"
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
        item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        item->setText(0, tr("Debug"));  // 🐛
        item->setForeground(0, QBrush(Qt::gray));
        break;
        case QtMsgType::QtInfoMsg:
        item->setIcon(0, style()->standardIcon(QStyle::SP_MessageBoxInformation));
        item->setText(0, tr("Info"));  // ℹ️
        item->setForeground(0, QBrush(QColor("#61AFEF")));
        break;
        case QtMsgType::QtWarningMsg:
        item->setIcon(0, style()->standardIcon(QStyle::SP_MessageBoxWarning));
        item->setText(0, tr("Warning"));   // ⚠️
        item->setForeground(0, QBrush(Qt::yellow));
        break;
        case QtMsgType::QtCriticalMsg:
        item->setIcon(0, style()->standardIcon(QStyle::SP_MessageBoxCritical));
        item->setText(0, tr("Error"));   // ⛔
        item->setForeground(0, QBrush(Qt::red));
        break;
        case QtMsgType::QtFatalMsg:
        item->setIcon(0, style()->standardIcon(QStyle::SP_MessageBoxCritical));
        item->setText(0, tr("Fatal"));  // ⛔
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
    TranslationManager::instance().registerLanguageChangeCallback(this, [this](){
        m_treeWidget->setHeaderLabels({ tr("Severity"), tr("Message") });
    });
    m_treeWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);   // Let user can scroll to the middle of a row, if the row contains multiple lines.
    // m_treeWidget->setHeaderLabels({tr""});
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_treeWidget);
}

