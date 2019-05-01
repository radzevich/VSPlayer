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
    qDebug() << "Audio buffer size: " << audioBuffer.count();

    FILE *fp;
    fp = fopen("samples.wav", "w");

    int bufferSize = audioBuffer.count();
    fwrite(audioBuffer.constData(), sizeof(qint8), audioBuffer.count(), fp);

    fflush(fp);
    fclose(fp);
}
