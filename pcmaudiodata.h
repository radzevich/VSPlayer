#pragma once

#include <QObject>
#include <qvector.h>

class PcmAudioData : public QObject
{
    Q_OBJECT

public:
    explicit PcmAudioData(QObject *parent = nullptr);
    ~PcmAudioData();

    bool isStereo() const;

    const QVector<qint16> *leftChannelData() const { return _leftChannelData; }
    void setLeftChannelData(const QVector<qint16> *data) { _leftChannelData = data; };

    const QVector<qint16> *rightChannelData() const { return _rightChannelData; };
    void setRightChannelData(const QVector<qint16> *data) { _rightChannelData = data; };

private:
    const QVector<qint16> *_leftChannelData = nullptr;
    const QVector<qint16> *_rightChannelData = nullptr;
};
