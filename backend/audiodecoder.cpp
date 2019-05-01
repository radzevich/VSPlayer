#include "audiodecoder.h"
#include "exceptions/audiodecoderexception.h"

#include <QAudioDeviceInfo>
#include <QAudioOutput>

AudioDecoder::AudioDecoder(QObject* pobj) : QObject(pobj)
{
    _audioDecoder = new QAudioDecoder(this);
    _audioBuffer = new QByteArray();
    _device = &QAudioDeviceInfo::defaultOutputDevice();

    connect(_audioDecoder, SIGNAL(bufferReady()), this, SLOT(onAudioBufferReady()));
    connect(_audioDecoder, SIGNAL(finished()), this, SLOT(onDecodingFinished()));
    connect(_audioDecoder, SIGNAL(error(QAudioDecoder::Error)), this, SLOT(onError(QAudioDecoder::Error)));
}

AudioDecoder::~AudioDecoder()
{
}

void AudioDecoder::decode(const QString &filePath, const QAudioFormat &audioFormat)
{
    releaseAudioBuffer();

   // _audioDecoder->setAudioFormat(audioFormat);
    _audioDecoder->setSourceFilename(filePath);
    _audioDecoder->start();
}

void AudioDecoder::onAudioBufferReady()
{
    if (_audioDecoder->bufferAvailable()) {
        QAudioBuffer audioBuffer = _audioDecoder->read();
        qDebug() << "Sample count: " << audioBuffer.sampleCount();
        qDebug() << "Byte count:" << audioBuffer.sampleCount();
        _audioBuffer->append(audioBuffer.constData<char>());
    } else {
        throw new AudioDecoderException();
    }
}

void AudioDecoder::onDecodingFinished()
{
    emit decoded(*_audioBuffer);
}

void AudioDecoder::onError(QAudioDecoder::Error errorCode)
{
    const QString error = _audioDecoder->errorString();

    qDebug() << error;
}

void AudioDecoder::releaseAudioBuffer()
{
    _audioBuffer->clear();
}

