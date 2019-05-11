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

// TODO: add samplesPerFragment parameter instead of fragmentDurationMs
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

        const auto amplitudeSpectrum = toAmplitudeSpectra(*complexRepresentation, samplesPerFragment);
        const auto energySpectrum = calculateEnergySpectra(amplitudeSpectrum);

        (*frequencySpectrogram)[i] = energySpectrum;

        delete amplitudeSpectrum;
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

const QVector<float> *SpectrumAnalyzer::toAmplitudeSpectra(complex_array &spectra, int size) const
{
    // According to Nyquist–Shannon sampling theorem,
    // the second half of spectra sequence is a mirror reflection of the first one.
    // So we can use only the half of data for analysis
    size = size >> 1;

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

const QVector<quint16> *SpectrumAnalyzer::calculateEnergySpectra(const QVector<float> *amplitudeSpectrum)
{
    const auto energySpectra = new QVector<quint16>(ENERGY_SPECTRA_SIZE, 0);

    for (auto spectrum : *amplitudeSpectrum)
    {
        if (spectrum < UPPER_ANALYZED_FREQUENCY)
        {
            const quint16 spectrumIntervalIndex = spectrum / FREQUENCY_STEP_HZ;
            (*energySpectra)[spectrumIntervalIndex]++;
        }
    }

    return energySpectra;
}
