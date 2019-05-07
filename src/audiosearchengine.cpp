#include "audiosearchengine.h"

#include <QObject>
#include <QAudioOutput>

AudioSearchEngine::AudioSearchEngine(QObject* pobj) : QObject(pobj)
{
    _audioDecoder = new AudioDecoder(this);

    connect(_audioDecoder, &AudioDecoder::decoded, this, &AudioSearchEngine::onAudioDecoded);
}

AudioSearchEngine::~AudioSearchEngine()
{
}

void AudioSearchEngine::startProcessing(const QString &filePath) const
{
    const auto audioFormat = new QAudioFormat();
    audioFormat->setSampleRate(16000);
    audioFormat->setChannelCount(2);
    audioFormat->setSampleSize(16);
    audioFormat->setSampleType(QAudioFormat::SignedInt);
    audioFormat->setByteOrder(QAudioFormat::LittleEndian);
    audioFormat->setCodec("pcm_s16le"); // Not compressed, signed int16, little endian

    _audioDecoder->decode(filePath, *audioFormat);

    delete audioFormat;
}

void AudioSearchEngine::onAudioDecoded(const PcmAudioData *pcmAudioData)
{
    const auto sizeL = pcmAudioData->leftChannelData()->count();
    const auto sizeR = pcmAudioData->rightChannelData()->count();

    delete pcmAudioData;
}
