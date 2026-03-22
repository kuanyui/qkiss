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
    enum ColIdx {
        ColIdxStateIcon,
        ColIdxFileName,
        ColIdxSize,
        ColIdxPatchBtn
    };

public:
    FileSystemBrowser(QWidget *parent = nullptr);
    ~FileSystemBrowser();
    /**
     * Ensure a dir path always ends with a slash `/` (Or `\` on Windows), while fild path should never ends with a slash.
     * If the path is not existed, returns an empty string.
     */
    static QString formalizePath(const QString& dirOrFilePath);
    /// File name, not dir name
    bool setPath(QString filePath);
    /// Set CWD
    bool setDirPath(QString dirPath);
    /// Get CWD
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
    void onItemActivated(const QModelIndex &index);
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
    /// Current working directory (CWD)
    QString m_curDirPath;
    /// Previous CWD
    QString m_prevOpenedDirPath;
    /// Unused currently.
    QLabel *m_labelCurDirName;
    FileIconProvider *m_fileIconProvider;
    /// First element is main archive path, the rests are patches' paths.
    /// Can be a LZH file path, or folder (not supported yet.)
    QList<QString> m_loadedKissFilePaths;

    /**
     * ```
     *                             m_curPathDir
     *    　|______|　                                       |______|
     *   BackwardStack             CurrentDir              ForwardStack
     * ```
     *
     * - When click any new folder   --> Clear `ForwardStack`, push `m_curPathDir` into `BackwardStack`; then `m_curPathDir := newDir`
     * - When click GoUp             --> Clear `ForwardStack`, push `m_curPathDir` into `BackwardStack`; then `m_curPathDir := newDir`
     * - When click GoBackward       --> take one from `BackwardStack` and push it to `ForwardStack`; then `m_curPathDir := takenDir`
     * - When click GoForward        --> take one from `ForwardStack` and push it to `BackwardStack`; then `m_curPathDir := takenDir`.
     */
    QStack<QString> m_navBackwardStack;
    /** When user press Back, item will be push on this. */
    QStack<QString> m_navForwardStack;
    ConfigFile &m_cfg;



    // QObject interface
public:
    /// NOTE: I tried to use `mousePressEvent` before, but it seems that the click event are all catched by QTreeView, so use eventFilter instead.
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

};


#endif // FILESYSTEMBROWSER_H
