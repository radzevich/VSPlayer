#include "audiosearchengine.h"

#include <cstdio>
#include <QObject>
#include <QAudioOutput>
#include <QIODevice>

AudioSearchEngine::AudioSearchEngine(QObject* pobj) : QObject(pobj)
{
    _audioDecoder = new AudioDecoder(this);

    connect(_audioDecoder, &AudioDecoder::decoded, this, &AudioSearchEngine::onAudioDecoded);
}

AudioSearchEngine::~AudioSearchEngine()
{
}

void AudioSearchEngine::startProcessing(const QString &filePath)
{
    QAudioFormat *audioFormat = new QAudioFormat();
    audioFormat->setSampleRate(44800);
    audioFormat->setChannelCount(1);
    audioFormat->setSampleSize(16);
    audioFormat->setSampleType(QAudioFormat::SignedInt);
    //audioFormat->setByteOrder(QAudioFormat::LittleEndian);
    audioFormat->setCodec("audio/pcm");

    _audioDecoder->decode(filePath, *audioFormat);
}

void AudioSearchEngine::onAudioDecoded(const QByteArray &audioBuffer)
{
    //if (audioBuffer.isValid() && audioBuffer.format().isValid()) {
    //    qInfo() << "AudioSearchEngine codec:" << audioBuffer.format().codec();
    //    qInfo() << "AudioSearchEngine byte order:" << audioBuffer.format().byteOrder();
    //    qInfo() << "AudioSearchEngine sample rate:" << audioBuffer.format().sampleRate();
    //    qInfo() << "AudioSearchEngine sample size:" << audioBuffer.format().sampleSize();
    //    qInfo() << "AudioSearchEngine sample type:" << audioBuffer.format().sampleType();
    //    qInfo() << "AudioSearchEngine channel count:" << audioBuffer.format().channelCount();
    //    qInfo() << "AudioSearchEngine bytes per frame:" << audioBuffer.format().bytesPerFrame();
    //    qInfo() << "AudioSearchEngine byte count:" << audioBuffer.byteCount();
    //    qInfo() << "AudioSearchEngine frame count:" << audioBuffer.frameCount();
    //    qInfo() << "AudioSearchEngine sample count:" << audioBuffer.sampleCount();
    //} else {
    //    qInfo() << "NOT VALID";
    //}

    QAudioFormat *audioFormat = new QAudioFormat();
    audioFormat->setSampleRate(44100);
    audioFormat->setChannelCount(2);
    audioFormat->setSampleSize(16);
    audioFormat->setSampleType(QAudioFormat::SignedInt);
    //audioFormat->setByteOrder(QAudioFormat::LittleEndian);
    audioFormat->setCodec("audio/pcm");

    int bytesCount = audioBuffer.count();
    qDebug() << "Duration: " << audioFormat->durationForBytes(bytesCount);

    const qint16 * data16 = reinterpret_cast<const qint16 *>(audioBuffer.constData());

    qDebug() << audioBuffer.count() / 2;

    FILE *fp;
    fp = fopen("samples.wav", "w");

    int bufferSize = audioBuffer.count() / 2;
    fwrite(audioBuffer.constData(), sizeof(qint16), audioBuffer.count(), fp);

    fflush(fp);
    fclose(fp);
}
