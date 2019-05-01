#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <QObject>
#include <QAudioDecoder>
#include <QAudioDeviceInfo>

class AudioDecoder : public QObject
{
    Q_OBJECT

public:
    explicit AudioDecoder(QObject* pobj = nullptr);
    virtual ~AudioDecoder();

    void AudioDecoder::decode(const QString &filePath, const QAudioFormat &audioFormat);
    void AudioDecoder::decodeAsync(const QString &filePath, const QAudioFormat &audioFormat);

signals:
    void decoded(QByteArray &audioBuffer);

private:
    QByteArray *_audioBuffer = nullptr;

    void onDecodingFinished();
    void releaseAudioBuffer();
};

#endif // AUDIODECODER_H
