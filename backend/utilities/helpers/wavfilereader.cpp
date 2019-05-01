#include "backend/utilities/helpers/wavfilereader.h"
#include "backend/models/wavfile.h"

#include <qfile.h>
#include <qendian.h>

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

WavFileReader::WavFileReader(QObject *parent)
    : QObject(parent)
{
}

WavFileReader::~WavFileReader()
{
}

const WavFile *WavFileReader::readFile(const QString &filePath)
{
    WavFile wavFile(this);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        onError("Error opening the .wav file");
        return &wavFile;
    }

    if (!readByChunk(file, wavFile)) {
        onError("Error readin the .wav file");
        return &wavFile;
    }

    wavFile.setIsValid(true);

    return &wavFile;
}

bool WavFileReader::readByChunk(QFile &file, WavFile &wavFile)
{
    QAudioFormat *audioFormat = wavFile.getAudioFormat();
    QByteArray *audioBuffer = wavFile.getAudioBuffer();

    quint64 filePos = file.pos();
    file.seek(0);

    bool isValid = true;
    bool canRead = true;

    while (canRead && isValid) {
        char descriptorId[4];

        quint64 pos = file.pos();

        if (!file.peek(descriptorId, sizeof(descriptorId)) == sizeof(descriptorId)) {
            onError("Error reading chunk descriptor id of .wav file");
            return false;
        }

        if (memcmp(descriptorId, "RIFF", 4) == 0) {
            isValid = readRiffChunk(file, audioFormat);
        }
        else if (memcmp(descriptorId, "RIFX", 4) == 0) {
            isValid = readRiffChunk(file, audioFormat);
        }
        else if (memcmp(descriptorId, "fmt ", 4) == 0) {
            isValid = readFmtChunk(file, audioFormat);
        }
        else if (memcmp(descriptorId, "LIST", 4) == 0) {
            isValid = readListHeader(file);
        }
        else if (memcmp(descriptorId, "data", 4) == 0) {
            isValid = readDataChunk(file, audioBuffer);
            canRead = false;
        }
    }

    file.seek(filePos);

    return isValid;
}

bool WavFileReader::readRiffChunk(QFile &file, QAudioFormat *audioFormat)
{
    RiffHeader riffHeader;

    if (file.read(reinterpret_cast<char *>(&riffHeader), sizeof(RiffHeader)) != sizeof(RiffHeader)) {
        onError("Error reading RIFF chunk of .wav file");
        return false;
    }

    if (memcmp(riffHeader.type, "WAVE", 4) != 0) {
        onError("'WAVE' file format expected");
        return false;
    }

    if (memcmp(&riffHeader.descriptor.id, "RIFF", 4) == 0) {
        audioFormat->setByteOrder(QAudioFormat::LittleEndian);
    }
    else {
        audioFormat->setByteOrder(QAudioFormat::BigEndian);
    }

    return true;
}

bool WavFileReader::readFmtChunk(QFile &file, QAudioFormat *audioFormat)
{
    FmtHeader fmtHeader;

    if (file.read(reinterpret_cast<char *>(&fmtHeader), sizeof(FmtHeader)) != sizeof(FmtHeader)) {
        onError("Error reading fmt chunk of .wav file");
        return false;
    }

    quint16 waveFormat = qFromLittleEndian<quint16>(fmtHeader.waveFormat);
    if (waveFormat != 1 && waveFormat != 0) {
        onError("Unexpected audio format of .wav file: ");
        return false;
    }

    if (qFromLittleEndian<quint32>(fmtHeader.descriptor.size) > sizeof(FmtHeader)) {
        // Extended data available
        quint16 extraFormatBytes;
        if (file.peek((char*)&extraFormatBytes, sizeof(quint16)) != sizeof(quint16)) {
            onError("Error peeking extended data");
            return false;
        }

        const qint64 throwAwayBytes = sizeof(quint16) + qFromLittleEndian<quint16>(extraFormatBytes);
        if (file.read(throwAwayBytes).size() != throwAwayBytes) {
            onError("Error reading extended data");
            return false;
        }
    }

    int bps = qFromLittleEndian<quint16>(fmtHeader.bitsPerSample);
    audioFormat->setChannelCount(qFromLittleEndian<quint16>(fmtHeader.numChannels));
    audioFormat->setCodec("audio/pcm");
    audioFormat->setSampleRate(qFromLittleEndian<quint32>(fmtHeader.sampleRate));
    audioFormat->setSampleSize(qFromLittleEndian<quint16>(fmtHeader.bitsPerSample));
    audioFormat->setSampleType(bps == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);

    return true;
}

bool WavFileReader::readListHeader(QFile &file)
{
    ListHeader listHeader;

    if (file.read(reinterpret_cast<char *>(&listHeader), sizeof(ListHeader)) != sizeof(ListHeader)) {
        onError("Error reading LIST chunk of .wav file");
        return false;
    }

    quint32 listHeaderSize = qFromLittleEndian<quint32>(listHeader.descriptor.size);
    quint64 offset = file.pos() + listHeaderSize;

    if (!file.seek(offset)) {
        onError("Error seeking LIST chunk of .wav file");
        return false;
    }

    return true;
}

bool WavFileReader::readDataChunk(QFile &file, QByteArray *audioBuffer)
{
    DataHeader dataHeader;
    if (file.read(reinterpret_cast<char *>(&dataHeader), sizeof(DataHeader)) != sizeof(DataHeader)) {
        onError("Error reading DATA chunk of .wav file");
        return false;
    }

    audioBuffer->clear();
    audioBuffer->append(file.readAll());

    if (audioBuffer->count() != qFromLittleEndian(dataHeader.descriptor.size)) {
        onError("Error reading audio data from .wav file");
        return false;
    }

    return true;
}

//template<typename T>
//T WavFileReader::readChunkHeader<T>()
//{
//    T dataHeader;
//    if (file.read(reinterpret_cast<char *>(&dataHeader), sizeof(T)) != sizeof(T)) {
//        onError("Error reading chunk of .wav file");
//    }
//
//    return dataHeader;
//}

void WavFileReader::onError(const QString &errorMessage)
{
    //QDebug << errorMessage.toStdString().c_str();
}