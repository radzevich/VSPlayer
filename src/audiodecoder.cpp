#include "audiodecoder.h"
#include "audiodecoderexception.h"
#include "wavfilereader.h"
#include "pcmaudiodata.h"

#include <QAudioDeviceInfo>
#include <QtConcurrent>
#include <qendian.h>

const QString AudioDecoder::TEMP_WAV_FILE = "soundfile.wav";
const QString AudioDecoder::OUTPUT_LOG = "logs/ffmpeg/outputs.txt";
const QString AudioDecoder::ERROR_LOG = "logs/ffmpeg/errors.txt";

const int AudioDecoder::SAMPLE_RATE_HZ = 25600;
const int AudioDecoder::SAMPLE_SIZE_BITS = 16;
const int AudioDecoder::CHANNELS_COUNT = 2;
const QString AudioDecoder::DEFAULT_CODEC = "pcm_s16le";

const qint8 AudioDecoder::LEFT_CHANNEL = 0;
const qint8 AudioDecoder::RIGHT_CHANNEL = 1;

AudioDecoder::AudioDecoder(QObject* pobj) : QObject(pobj)
{
}

AudioDecoder::~AudioDecoder()
{
}

const PcmAudioData *AudioDecoder::decode(const QString &filePath) const
{
    WavData wavData;

    const auto audioFilePath = mediaToAudio(filePath);
    const auto rawAudioData = audioToRawAudioData(audioFilePath, &wavData);
    const auto pcmAudioData = rawAudioDataToPcmByChannels(rawAudioData);

    return pcmAudioData;
}

const QString *AudioDecoder::mediaToAudio(const QString &mediaFilePath) const
{
    const auto sampleRate = QString::number(SAMPLE_RATE_HZ);
    const auto channelsCount = QString::number(CHANNELS_COUNT);
    const auto args = QStringList()
        << "-y"                         // overwrite output file
        << "-i"     << mediaFilePath    // input file
        << "-ar"    << QString::number(SAMPLE_RATE_HZ)
        << "-ac"    << QString::number(CHANNELS_COUNT)
        << "-c:a"   << DEFAULT_CODEC
        << "-vn"    << TEMP_WAV_FILE;    // output file

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

    return &TEMP_WAV_FILE;
}

const WavData *AudioDecoder::audioToRawAudioData(const QString *audioFilePath, WavData *wavData)
{
    const WavFileReader wavFileReader(*audioFilePath);
    wavFileReader.readWavData(wavData, true);

    return wavData;
}

const PcmAudioData *AudioDecoder::rawAudioDataToPcmByChannels(const WavData *rawAudioData) const
{
    const auto audioBuffer = rawAudioData->audioBuffer();
    const auto audioData = const_cast<qint16 *>(reinterpret_cast<const qint16 *>(audioBuffer->constData()));

    const auto audioBufferSize = audioBuffer->count();
    const quint64 samplesCount = static_cast<double>(audioBufferSize) / SAMPLE_SIZE_BITS * 8;
    const auto framesCount = samplesCount / CHANNELS_COUNT;

    auto pcmAudioData = new PcmAudioData();

    const auto leftChannelData = readDataForChannel(audioData, framesCount, LEFT_CHANNEL);
    pcmAudioData->setLeftChannelData(leftChannelData);

    const auto rightChannelData = readDataForChannel(audioData, framesCount, LEFT_CHANNEL);
    pcmAudioData->setRightChannelData(rightChannelData);

    return pcmAudioData;
}

QVector<qint16> *AudioDecoder::readDataForChannel(
    const qint16* allChannelsData,
    const quint64 framesCount,
    const qint8 channelNumber) const
{
    const auto dataForChannel = new QVector<qint16>(framesCount);

    for (quint64 frameNumber = 0; frameNumber < framesCount; frameNumber++) {
        const qint64 sampleNumber = frameNumber * CHANNELS_COUNT + channelNumber;
        const auto sample = qFromLittleEndian(allChannelsData[sampleNumber]);

        (*dataForChannel)[frameNumber] = sample;
    }

    return dataForChannel;
}
