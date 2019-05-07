#include "backend/models/wavdata.h"
#include <qaudioformat.h>

WavData::WavData(QObject *parent)
    : QObject(parent)
{
    _audioFormat = new QAudioFormat();
    _audioBuffer = new QByteArray();
}

QAudioFormat *WavData::audioFormat() const
{
    return _audioFormat;
}

void WavData::setAudioFormat(QAudioFormat *audioFormat)
{
    _audioFormat = audioFormat;
}

QByteArray *WavData::audioBuffer() const
{
    return _audioBuffer;
}

void WavData::setAudioBuffer(QByteArray *audioData)
{
    _audioBuffer = audioData;
}
