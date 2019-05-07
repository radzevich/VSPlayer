#pragma once

#include <QObject>
#include <QAudioDeviceInfo>
#include "pcmaudiodata.h"
#include "wavdata.h"

class AudioDecoder final : public QObject
{
    Q_OBJECT

public:
    explicit AudioDecoder(QObject* pobj = nullptr);
    virtual ~AudioDecoder();

    void decode(const QString &filePath, const QAudioFormat &audioFormat);
    void decodeAsync(const QString &filePath, const QAudioFormat &audioFormat);

signals:
    void decoded(const PcmAudioData *pcmAudioData);

private:
    const QString _tempWavFile = "soundfile.wav";
    const QString _outputLog = "logs/ffmpeg/outputs.txt";
    const QString _errorLog = "logs/ffmpeg/errors.txt";

    void onDecodingFinished(const PcmAudioData *pcmAudioData);
    const QString *mediaToAudio(const QString &mediaFilePath, const QAudioFormat &audioFormat) const;
    static const WavData *audioToRawAudioData(const QString *audioFilePath, WavData *wavData);
    const PcmAudioData *rawAudioDataToPcmByChannels(const WavData *rawAudioData, const QAudioFormat *audioFormat) const;
};
