#pragma once

#include <QObject>
#include "backend/models/wavfile.h"

class WavFileReader : public QObject
{
    Q_OBJECT

public:
    WavFileReader(QObject *parent);
    ~WavFileReader();

    const WavFile *readFile(const QString &filePath);

private:
    //template <typename T> T readChunkHeader();

    bool readByChunk(QFile &file, WavFile &wavFile);
    bool readRiffChunk(QFile &file, QAudioFormat *audioFormat);
    bool readFmtChunk(QFile &file, QAudioFormat *audioFormat);
    bool readListHeader(QFile &file);
    bool readDataChunk(QFile &file, QByteArray *audioBuffer);

    void onError(const QString &errorMessage);
};
