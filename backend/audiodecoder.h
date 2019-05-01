#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <QObject>
#include <QAudioDecoder>
#include <QAudioDeviceInfo>

struct AudioSamples
{
    uint samplesCount;
    const quint16* buffer;
};

class AudioDecoder : public QObject
{
    Q_OBJECT

public:
    explicit AudioDecoder(QObject* pobj = nullptr);
    virtual ~AudioDecoder();

    void AudioDecoder::decode(const QString &filePath, const QAudioFormat &audioFormat);

signals:
    void decoded(QByteArray &audioBuffer);

private slots:
    void onAudioBufferReady();
    void onDecodingFinished();
    void onError(QAudioDecoder::Error errorCode);

private:
    QAudioDecoder *_audioDecoder = nullptr;
    QByteArray *_audioBuffer = nullptr;
    QAudioDeviceInfo *_device = nullptr;

    void releaseAudioBuffer();
};

#endif // AUDIODECODER_H
