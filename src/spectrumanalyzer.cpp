#include "spectrumanalyzer.h"
#include <complex>
#include <qmath.h>
#include <valarray>
#include <qvector.h>

SpectrumAnalyzer::SpectrumAnalyzer(QObject *parent)
    : QObject(parent)
{
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
}

const spectrogram *SpectrumAnalyzer::getFrequencySpectrogram(
    const QVector<qint16> *pcmAudioData,
    const quint32 sampleRate,
    const quint32 fragmentDurationMs) const
{
    const quint32 samplesPerFragment = static_cast<double>(sampleRate) * fragmentDurationMs / 1000;
    const auto fragmentsByTime = splitByTimeIntervals(pcmAudioData, samplesPerFragment);

    const auto fragmentsCount = fragmentsByTime->count();
    const auto frequencySpectrogram = new spectrogram(fragmentsCount);

    for (auto i = 0; i < fragmentsCount; i++)
    {
        const auto audioFragment = (*fragmentsByTime)[i];
        const auto complexRepresentation = toComplex(audioFragment, samplesPerFragment);

        fastFourierTransform(*complexRepresentation, samplesPerFragment);

        const auto amplitudeSpectra = toAmplitudeSpectra(*complexRepresentation, samplesPerFragment);
        (*frequencySpectrogram)[i] = amplitudeSpectra;

        delete complexRepresentation;
    }

    return frequencySpectrogram;
}

const QVector<const qint16 *> *SpectrumAnalyzer::splitByTimeIntervals(
    const QVector<qint16> *pcmAudioData,
    const quint32 samplesPerFragment) const
{
    const auto samplesCount = pcmAudioData->count();
    const int fragmentsCount = ceil(static_cast<double>(samplesCount) / samplesPerFragment);

    const auto result = new QVector<const qint16*>(fragmentsCount);

    const auto data = pcmAudioData->constData();

    for (auto i = 0; i < fragmentsCount; i++)
    {
        (*result)[i] = &data[i * samplesPerFragment];
    }

    // All fragments sizes should be powers of 2 for fast Fourier transformation
    const auto lastFragmentSize = fragmentsCount * samplesPerFragment - samplesCount;
    if (lastFragmentSize != 0)
    {
        const auto fullLastFragment = new qint16[samplesPerFragment];

        const auto lastFragment = result->last();
        const auto missedSamplesCount = samplesPerFragment - lastFragmentSize;

        // Copy samples from the last fragment to a new full fragment
        memcpy(fullLastFragment, lastFragment, lastFragmentSize);

        // Set missed values as 0
        memset(&fullLastFragment[lastFragmentSize], 0, missedSamplesCount);

        result->removeLast();
        result->append(fullLastFragment);
    }

    return result;
}

complex_array *SpectrumAnalyzer::toComplex(const qint16 *pcmAudioData, const int size)
{
    const auto complexValues = new complex_array(size);

    for (auto i = 0; i < size; i++)
    {
        (*complexValues)[i] = static_cast<double>(pcmAudioData[i]);
    }

    return complexValues;
}

void SpectrumAnalyzer::fastFourierTransform(complex_array &spectra, int n) const
{
    if (n < 1)
    {
        return;
    }

    n >>= 1;

    complex_array odd = spectra[std::slice(0, n, 2)];
    complex_array even = spectra[std::slice(1, n, 2)];

    fastFourierTransform(even, n);
    fastFourierTransform(odd, n);

    for (auto i = 0; i < n; i++)
    {
        auto t = exp(std::complex<double>(0, -2 * M_PI * i / (n * 2))) * odd[i];
        spectra[i] = even[i] + t;
        spectra[n + i] = even[i] - t;
    }
}

const QVector<float> *SpectrumAnalyzer::toAmplitudeSpectra(complex_array &spectra, const int size) const
{
    const auto amplitudeSpectra = new QVector<float>(size);
    for (auto i = 0; i < size; i++)
    {
        const auto spectrum = spectra[i];
        const auto real = spectrum.real();
        const auto imag = spectrum.imag();

        (*amplitudeSpectra)[i] = sqrt(real * real + imag * imag);
    }

    return amplitudeSpectra;
}
