#ifndef ARCHIVEDFILE_H
#define ARCHIVEDFILE_H

#include "enums.h"
#include <QObject>
#include <QString>
#include <QProcess>
#include <QHash>
#include <QTemporaryDir>
#include "parsedcnf.h"
#include "parsedkcf.h"
#include "parsedcel.h"

class ArchivedFile: public QObject
{
    Q_OBJECT
public:
    ArchivedFile(QObject *parent = nullptr);
    bool loadLzh(const QString &filePath, QtKISS::Case caseSensitivity);
    static bool checkLhaCommandAvailability();
    QStringList getCnfNameList();
    QStringList getOtherFileNameList();
    /** Auto handling case sensitivity. */
    std::optional<ParsedCnf *> getParsedCnf(QString cnfFileName) const;
    std::optional<ParsedKcf *> getParsedKcf(QString kcfFileName) const;
    std::optional<ParsedCel *> getParsedCel(QString celFileName) const;
    std::optional<QByteArray> getOtherFile(QString celFileName) const;
private slots:
    void onProcessReadyRead();
    void onProcessFinished();
signals:
    void newLogLine(QString line);
private:
    QtKISS::Case m_caseSensitivity;
    QProcess *m_process;
    QTemporaryDir *m_tmpDir;
    QMap<QString, ParsedCnf*> *m_parsedCnfFiles;
    QHash<QString, ParsedKcf*> *m_parsedKcfFiles;
    QHash<QString, ParsedCel*> *m_parsedCelFiles;
    QMap<QString, QByteArray> *m_otherFiles;
    QString m_archiveFilePath;
};

#endif // ARCHIVEDFILE_H
