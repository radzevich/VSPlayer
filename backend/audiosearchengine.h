#ifndef AUDIOSEARCHENGINE_H
#define AUDIOSEARCHENGINE_H

#include "audiodecoder.h"

#include <QObject>

class AudioSearchEngine : public QObject
{
    Q_OBJECT

public:
    explicit AudioSearchEngine(QObject* pobj = nullptr);
    virtual ~AudioSearchEngine();

    void startProcessing(const QString &filePath);

signals:
    void error(const QString &errorMessage);

private:
    void onAudioDecoded(const QByteArray &audioBuffer);

    AudioDecoder *_audioDecoder = nullptr;
};

#endif // AUDIOSEARCHENGINE_H
