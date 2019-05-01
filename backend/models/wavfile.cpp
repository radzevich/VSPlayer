#include "backend/models/wavfile.h"
#include <qaudioformat.h>

WavFile::WavFile(QObject *parent)
    : QObject(parent)
    , _isValid(false)
{
    _audioFormat = new QAudioFormat();
    _audioBuffer = new QByteArray();
}

QAudioFormat *WavFile::getAudioFormat() const
{
    return _audioFormat;
}

void WavFile::setAudioFormat(QAudioFormat &audioFormat)
{
    _audioFormat = &audioFormat;
}

QByteArray *WavFile::getAudioBuffer() const
{
    return _audioBuffer;
}

void WavFile::setAudioBuffer(QByteArray &audioBuffer)
{
    _audioBuffer = &audioBuffer;
}

bool WavFile::isValid() const
{
    return _isValid;
}

void WavFile::setIsValid(bool isValid)
{
    _isValid = isValid;
}
