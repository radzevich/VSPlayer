#include "pcmaudiodata.h"

PcmAudioData::PcmAudioData(QObject *parent)
    : QObject(parent)
{
}

PcmAudioData::~PcmAudioData()
{
    delete _leftChannelData;
    delete _rightChannelData;
}

bool PcmAudioData::isStereo() const
{
    return _leftChannelData != nullptr && _rightChannelData != nullptr;
}
