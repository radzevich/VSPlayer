#pragma once

#include <QFile>
#include <QAudioFormat>

class WavData : public QObject
{
    Q_OBJECT

public:
    explicit WavData(QObject *parent = nullptr);

    QAudioFormat *audioFormat() const;
    void setAudioFormat(QAudioFormat *audioFormat);

    QByteArray *audioBuffer() const;
    void setAudioBuffer(QByteArray *audioData);

private:
    QAudioFormat *_audioFormat = nullptr;
    QByteArray *_audioBuffer = nullptr;
};
