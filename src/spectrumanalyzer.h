#pragma once

#include <QObject>
#include <complex>
#include <valarray>

typedef std::complex<double> complex;
typedef std::valarray<complex> complex_array;
typedef QVector<const QVector<float> *> spectrogram;

class SpectrumAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit SpectrumAnalyzer(QObject *parent);
    ~SpectrumAnalyzer();

    const spectrogram *getFrequencySpectrogram(const QVector<qint16> *pcmAudioData, quint32 sampleRate, quint32 fragmentDurationMs) const;

private:
    const QVector<const qint16 *> *splitByTimeIntervals(const QVector<qint16> *pcmAudioData, quint32 samplesPerFragment) const;
    static complex_array *toComplex(const qint16 *pcmAudioData, int size);
    void fastFourierTransform(complex_array &spectra, int n) const;
    const QVector<float> *toAmplitudeSpectra(complex_array &spectra, const int size) const;
};
