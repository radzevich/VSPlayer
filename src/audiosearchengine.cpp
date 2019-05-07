#include "audiosearchengine.h"

#include <QObject>
#include <QAudioOutput>

AudioSearchEngine::AudioSearchEngine(QObject* pobj)
    : QObject(pobj)
{
    _audioDecoder = new AudioDecoder(this);
    _spectrumAnalyzer = new SpectrumAnalyzer(this);
}

AudioSearchEngine::~AudioSearchEngine()
{
}

void AudioSearchEngine::analyze(const QString &filePath) const
{
    const auto pcmAudioData = _audioDecoder->decode(filePath);
    const auto frequencySpectra = _spectrumAnalyzer->getFrequencySpectrogram(pcmAudioData->leftChannelData(), 25600, 20);

    QFile file("frequencies.csv");
    file.open(QIODevice::WriteOnly);

    for (auto spectrum : *frequencySpectra)
    {
        for (auto i = 0; i < 512; i++)
        {
            file.write(QString::number((*spectrum)[i]).toStdString().c_str());

            if (i != 511)
            {
                file.write(", ");
            }
            else
            {
                file.write("\n");
            }
        }
    }

    file.flush();
    file.close();
}
