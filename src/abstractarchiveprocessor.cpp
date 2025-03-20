#include "abstractarchiveprocessor.h"


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

int AbstractArchiveProcessor::instanceCount = 0;
