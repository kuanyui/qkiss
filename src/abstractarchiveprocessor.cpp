#include "abstractarchiveprocessor.h"
#include <QDir>

AbstractArchiveProcessor::AbstractArchiveProcessor(QObject *parent)
    : QObject{parent}
{
    instanceCount++;
    qDebug() << "ctor of AbstractArchiveProcessor" << instanceCount;
}


AbstractArchiveProcessor::~AbstractArchiveProcessor() {
    instanceCount--;
    qDebug() << "dtor of AbstractArchiveProcessor" << instanceCount;
}

QString AbstractArchiveProcessor::generateTempDirPath()
{
    return QDir(QDir::tempPath()).filePath("qtkiss-extracted-XXXXXX");
}

int AbstractArchiveProcessor::instanceCount = 0;
