#include "audiodecoder.h"
#include "exceptions/audiodecoderexception.h"
#include "backend/utilities/helpers/wavfilereader.h"
#include "backend/models/wavfile.h"

#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QtConcurrent>

using namespace std;

AudioDecoder::AudioDecoder(QObject* pobj) : QObject(pobj)
{
    _audioBuffer = new QByteArray();
}

AudioDecoder::~AudioDecoder()
{
}

void AudioDecoder::decode(const QString &filePath, const QAudioFormat &audioFormat)
{
    releaseAudioBuffer();

    QProcess process;
    process.setStandardErrorFile("logs/ffmpeg/errors.txt");
    process.setStandardOutputFile("logs/ffmpeg/outputs.txt");
    process.start("ffmpeg", QStringList() << "-i" << filePath << "-vn" << "soundfile.wav");
    process.waitForFinished();

    QProcess::ExitStatus exitStatus = process.exitStatus();

    process.kill();

    if (exitStatus == QProcess::Crashed)
    {
        qDebug() << "Error converting video to .wav file";
        return;
    }

    WavFileReader wavFileReader(this);

    const WavFile *wavFile = wavFileReader.readFile("soundfile.wav");
    if (!wavFile->isValid()) {
        qDebug("soundfile.wav is not valid");
        return;
    }

    _audioBuffer = wavFile->getAudioBuffer();
    onDecodingFinished();
}

void AudioDecoder::decodeAsync(const QString &filePath, const QAudioFormat &audioFormat)
{
//    QFuture<void> future = QtConcurrent::run(this, &decode, filePath, audioFormat);
    decode(filePath, audioFormat);
}

void AudioDecoder::onDecodingFinished()
{
    emit decoded(*_audioBuffer);
}

void AudioDecoder::releaseAudioBuffer()
{
    _audioBuffer->clear();
}

