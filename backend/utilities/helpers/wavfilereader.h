#pragma once

#include <QObject>
#include <QFileInfo>
#include "backend/models/wavdata.h"

class WavFileReader : public QObject
{
    Q_OBJECT

public:
    explicit WavFileReader(const QString &filePath, QObject *parent = nullptr);
    ~WavFileReader();

    void readWavData(WavData* rawAudioData, bool removeWavFileAfterReading = false) const;

private:
    QFile *_file = nullptr;

    void closeFile() const;
    void openFile() const;
    void readFile(WavData *rawAudioData) const;
    void readRiffChunk(QAudioFormat *audioFormat) const;
    void readFmtChunk(QAudioFormat *audioFormat) const;
    void readListHeader() const;
    void readDataChunk(QByteArray *audioBuffer) const;
    void removeFile() const;
};
