#include "backend/utilities/helpers/wavfilereader.h"
#include "filereaderexception.h"

#include <qfile.h>
#include <qendian.h>
#include "backend/models/wavdata.h"

struct ChunkDescriptor
{
    char        id[4];
    quint32     size;
};

struct RiffHeader
{
    ChunkDescriptor descriptor;
    char            type[4];
};

struct FmtHeader
{
    ChunkDescriptor descriptor;
    quint16         waveFormat;
    quint16         numChannels;
    quint32         sampleRate;
    quint32         byteRate;
    quint16         blockAlign;
    quint16         bitsPerSample;
};

struct ListHeader
{
    ChunkDescriptor descriptor;
};

struct DataHeader
{
    ChunkDescriptor descriptor;
};

WavFileReader::WavFileReader(const QString &filePath, QObject *parent)
    : QObject(parent)
{
    _file = new QFile(filePath);
}

WavFileReader::~WavFileReader()
{
    _file->close();
    delete _file;
}

void WavFileReader::readWavData(WavData* rawAudioData, const bool removeWavFileAfterReading) const
{
    try {
        openFile();
        readFile(rawAudioData);
        closeFile();

        if (removeWavFileAfterReading) {
            removeFile();
        }
    }
    catch (FileReaderException &ex) {
        throw ex;
    }
    catch (std::exception &ex) {
        throw FileReaderException(ex.what());
    }
    catch (...) {
        throw FileReaderException("Unhandled exception");
    }
}

void WavFileReader::readFile(WavData *rawAudioData) const
{
    const auto audioFormat = rawAudioData->audioFormat();
    const auto audioBuffer = rawAudioData->audioBuffer();

    auto canRead = true;
    while (canRead) {
        char descriptorId[4];

        quint64 pos = _file->pos();

        if (_file->peek(descriptorId, sizeof(descriptorId)) != sizeof(descriptorId)) {
            throw FileReaderException("Error reading chunk descriptor id of .wav file");
        }

        if (memcmp(descriptorId, "RIFF", 4) == 0) {
            readRiffChunk(audioFormat);
        }
        else if (memcmp(descriptorId, "RIFX", 4) == 0) {
            readRiffChunk(audioFormat);
        }
        else if (memcmp(descriptorId, "fmt ", 4) == 0) {
            readFmtChunk(audioFormat);
        }
        else if (memcmp(descriptorId, "LIST", 4) == 0) {
            readListHeader();
        }
        else if (memcmp(descriptorId, "data", 4) == 0) {
            readDataChunk(audioBuffer);
            canRead = false;
        }
    }
}

void WavFileReader::readRiffChunk(QAudioFormat *audioFormat) const
{
    RiffHeader riffHeader{};

    if (_file->read(reinterpret_cast<char *>(&riffHeader), sizeof(RiffHeader)) != sizeof(RiffHeader)) {
        throw FileReaderException("Error reading RIFF chunk of .wav file");
    }

    if (memcmp(riffHeader.type, "WAVE", 4) != 0) {
        throw FileReaderException("'WAVE' file format expected");
    }

    if (memcmp(&riffHeader.descriptor.id, "RIFF", 4) == 0) {
        audioFormat->setByteOrder(QAudioFormat::LittleEndian);
    }
    else {
        audioFormat->setByteOrder(QAudioFormat::BigEndian);
    }
}

void WavFileReader::readFmtChunk(QAudioFormat *audioFormat) const
{
    FmtHeader fmtHeader{};

    if (_file->read(reinterpret_cast<char *>(&fmtHeader), sizeof(FmtHeader)) != sizeof(FmtHeader)) {
        throw FileReaderException("Error reading fmt chunk of .wav file");
    }

    const auto waveFormat = qFromLittleEndian<quint16>(fmtHeader.waveFormat);
    if (waveFormat != 1 && waveFormat != 0) {
        throw FileReaderException("Unexpected audio format of .wav file: ");
    }

    if (qFromLittleEndian<quint32>(fmtHeader.descriptor.size) > sizeof(FmtHeader)) {
        // Extended data available
        quint16 extraFormatBytes;
        if (_file->peek(reinterpret_cast<char*>(&extraFormatBytes), sizeof(quint16)) != sizeof(quint16)) {
            throw FileReaderException("Error peeking extended data");
        }

        const qint64 throwAwayBytes = sizeof(quint16) + qFromLittleEndian<quint16>(extraFormatBytes);
        if (_file->read(throwAwayBytes).size() != throwAwayBytes) {
            throw FileReaderException("Error reading extended data");
        }
    }

    const int bps = qFromLittleEndian<quint16>(fmtHeader.bitsPerSample);
    audioFormat->setChannelCount(qFromLittleEndian<quint16>(fmtHeader.numChannels));
    audioFormat->setCodec("audio/pcm");
    audioFormat->setSampleRate(qFromLittleEndian<quint32>(fmtHeader.sampleRate));
    audioFormat->setSampleSize(qFromLittleEndian<quint16>(fmtHeader.bitsPerSample));
    audioFormat->setSampleType(bps == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);
}

void WavFileReader::readListHeader() const
{
    ListHeader listHeader{};

    if (_file->read(reinterpret_cast<char *>(&listHeader), sizeof(ListHeader)) != sizeof(ListHeader)) {
        throw FileReaderException("Error reading LIST chunk of .wav file");
    }

    const auto listHeaderSize = qFromLittleEndian<quint32>(listHeader.descriptor.size);
    const quint64 offset = _file->pos() + listHeaderSize;

    if (!_file->seek(offset)) {
        throw FileReaderException("Error seeking LIST chunk of .wav file");
    }
}

void WavFileReader::readDataChunk(QByteArray *audioBuffer) const
{
    DataHeader dataHeader{};
    if (_file->read(reinterpret_cast<char *>(&dataHeader), sizeof(DataHeader)) != sizeof(DataHeader)) {
        throw FileReaderException("Error reading DATA chunk of .wav file");
    }

    audioBuffer->clear();
    audioBuffer->append(_file->readAll());

    if (static_cast<uint>(audioBuffer->count()) != qFromLittleEndian(dataHeader.descriptor.size)) {
        throw FileReaderException("Error reading audio data from .wav file");
    }
}

void WavFileReader::openFile() const
{
    if (_file->isOpen()) {
        _file->close();
    }

    if (!_file->exists()) {
        throw FileReaderException(".wav file not found");
    }

    if (!_file->open(QIODevice::ReadOnly)) {
        throw FileReaderException("Error opening the .wav file");
    }
}

void WavFileReader::closeFile() const
{
    _file->close();
}

void WavFileReader::removeFile() const
{
    if (_file->exists())
    {
        if (!_file->remove()) {
            throw FileReaderException("Error removing the .wav file");
        };
    }
}
