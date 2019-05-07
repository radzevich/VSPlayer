#include "audiodecoder.h"
#include "audiodecoderexception.h"
#include "backend/utilities/helpers/wavfilereader.h"
#include "pcmaudiodata.h"

#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QtConcurrent>
#include <qendian.h>

AudioDecoder::AudioDecoder(QObject* pobj) : QObject(pobj)
{
}

AudioDecoder::~AudioDecoder()
{
}

void AudioDecoder::decode(const QString &filePath, const QAudioFormat &audioFormat)
{
    WavData wavData;

    const auto audioFilePath = mediaToAudio(filePath, audioFormat);
    const auto rawAudioData = audioToRawAudioData(audioFilePath, &wavData);
    const auto pcmAudioData = rawAudioDataToPcmByChannels(rawAudioData, &audioFormat);

    onDecodingFinished(pcmAudioData);
}

const QString *AudioDecoder::mediaToAudio(const QString &mediaFilePath, const QAudioFormat &audioFormat) const
{
    const auto audioCodec = audioFormat.codec();
    const auto sampleRate = QString::number(audioFormat.sampleRate());
    const auto channelsCount = QString::number(audioFormat.channelCount());
    const auto args = QStringList()
        << "-y"                         // overwrite output file
        << "-i"     << mediaFilePath    // input file
        << "-ar"    << sampleRate       // audio sample rate (Hz)
        << "-ac"    << channelsCount
        << "-c:a"   << audioCodec
        << "-vn"    << _tempWavFile;    // output file

    QProcess process;
    process.setStandardErrorFile("logs/ffmpeg/errors.txt");
    process.setStandardOutputFile("logs/ffmpeg/output.txt");
    process.start("ffmpeg", args);
    process.waitForFinished();

    const auto exitStatus = process.exitStatus();

    process.kill();

    if (exitStatus == QProcess::ExitStatus::CrashExit)
    {
        throw AudioDecoderException("Error converting media file to .wav file");
    }

    return &_tempWavFile;
}

const WavData *AudioDecoder::audioToRawAudioData(const QString *audioFilePath, WavData *wavData)
{
    const WavFileReader wavFileReader(*audioFilePath);
    wavFileReader.readWavData(wavData, true);

    return wavData;
}

const PcmAudioData *AudioDecoder::rawAudioDataToPcmByChannels(
    const WavData *rawAudioData,
    const QAudioFormat *audioFormat) const
{
    auto pcmAudioData = new PcmAudioData();

    const auto audioBuffer = rawAudioData->audioBuffer();
    const auto audioData = const_cast<qint16 *>(reinterpret_cast<const qint16 *>(audioBuffer->constData()));

    const auto sampleSize = audioFormat->sampleSize();
    const auto audioBufferSize = audioBuffer->count();
    const quint64 samplesCount = static_cast<double>(audioBufferSize) / sampleSize * 8;

    const auto channelsCount = audioFormat->channelCount();
    const auto framesCount = samplesCount / audioFormat->channelCount();

    for (auto channelNumber = 0; channelNumber < channelsCount; channelNumber++) {
        auto channelData = new QVector<qint16>(framesCount);

        for (quint64 frameNumber = 0; frameNumber < framesCount; frameNumber++) {
            const qint64 sampleNumber = frameNumber * channelsCount + channelNumber;
            const auto sample = qFromLittleEndian(audioData[sampleNumber]);

            channelData->append(sample);
        }

        if (channelNumber == 0) {
            pcmAudioData->setLeftChannelData(channelData);
        }
        else {
            pcmAudioData->setRightChannelData(channelData);
        }
    }

    return pcmAudioData;
}

void AudioDecoder::decodeAsync(const QString &filePath, const QAudioFormat &audioFormat)
{
//    QFuture<void> future = QtConcurrent::run(this, &decode, filePath, audioFormat);
    decode(filePath, audioFormat);
}

void AudioDecoder::onDecodingFinished(const PcmAudioData *pcmAudioData)
{
    emit decoded(pcmAudioData);
}
