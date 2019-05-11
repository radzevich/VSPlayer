#pragma once

#include <QObject>
#include <complex>
#include <valarray>

typedef std::complex<double> complex;
typedef std::valarray<complex> complex_array;
typedef QVector<const QVector<quint16> *> spectrogram;

class SpectrumAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit SpectrumAnalyzer(QObject *parent);
    ~SpectrumAnalyzer();

    const spectrogram *getFrequencySpectrogram(const QVector<qint16> *pcmAudioData, quint32 sampleRate, quint32 fragmentDurationMs) const;

private:
    static const int UPPER_ANALYZED_FREQUENCY = 8000;
    static const int FREQUENCY_STEP_HZ = 50;
    static const int ENERGY_SPECTRA_SIZE = UPPER_ANALYZED_FREQUENCY / FREQUENCY_STEP_HZ;

    const QVector<const qint16 *> *splitByTimeIntervals(const QVector<qint16> *pcmAudioData, quint32 samplesPerFragment) const;
    static complex_array *toComplex(const qint16 *pcmAudioData, int size);
    void fastFourierTransform(complex_array &spectra, int n) const;
    const QVector<float> *toAmplitudeSpectra(complex_array &spectra, int size) const;
    static const QVector<quint16> *calculateEnergySpectra(const QVector<float> *amplitudeSpectrum);
};
