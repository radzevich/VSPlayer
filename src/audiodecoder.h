#pragma once

#include <QObject>
#include <QAudioDeviceInfo>
#include "pcmaudiodata.h"
#include "wavdata.h"

class AudioDecoder final : public QObject
{
    Q_OBJECT

public:
    static const int CHANNELS_COUNT;
    static const int SAMPLE_RATE_HZ;
    static const int SAMPLE_SIZE_BITS;
    static const QString DEFAULT_CODEC;

    explicit AudioDecoder(QObject* pobj = nullptr);
    virtual ~AudioDecoder();

    const PcmAudioData * decode(const QString &filePath) const;

private:
    static const QString TEMP_WAV_FILE;
    static const QString OUTPUT_LOG;
    static const QString ERROR_LOG;

    static const qint8 LEFT_CHANNEL;
    static const qint8 RIGHT_CHANNEL;

    const QString *mediaToAudio(const QString &mediaFilePath) const;
    static const WavData *audioToRawAudioData(const QString *audioFilePath, WavData *wavData);
    const PcmAudioData *rawAudioDataToPcmByChannels(const WavData *rawAudioData) const;
    QVector<qint16> *readDataForChannel(const qint16* allChannelsData, quint64 framesCount, qint8 channelNumber) const;
};
