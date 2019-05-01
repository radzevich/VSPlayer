#pragma once

#include <QFile>
#include <QAudioFormat>

class WavFile : public QObject
{
    Q_OBJECT

public:
    WavFile(QObject *parent = 0);

    QAudioFormat *getAudioFormat() const;
    void setAudioFormat(QAudioFormat &audioFormat);

    QByteArray *getAudioBuffer() const;
    void setAudioBuffer(QByteArray &audioData);

    bool isValid() const;
    void setIsValid(bool isValid);

private:
    QAudioFormat *_audioFormat = nullptr;
    QByteArray *_audioBuffer = nullptr;

    bool _isValid;
};
