#pragma once

#include "audiodecoder.h"
#include "pcmaudiodata.h"

#include <QObject>

class AudioSearchEngine : public QObject
{
    Q_OBJECT

public:
    explicit AudioSearchEngine(QObject* pobj = nullptr);
    virtual ~AudioSearchEngine();

    void startProcessing(const QString &filePath) const;

signals:
    void error(const QString &errorMessage);

private:
    static void onAudioDecoded(const PcmAudioData *pcmAudioData);

    AudioDecoder *_audioDecoder = nullptr;
};
