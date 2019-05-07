#pragma once

#include <QObject>
#include "audiodecoder.h"
#include "spectrumanalyzer.h"

class AudioSearchEngine : public QObject
{
    Q_OBJECT

public:
    explicit AudioSearchEngine(QObject* pobj = nullptr);
    virtual ~AudioSearchEngine();

    void analyze(const QString &filePath) const;

signals:
    void error(const QString &errorMessage);

private:
    QString _searchRequest = "";
    AudioDecoder *_audioDecoder = nullptr;
    SpectrumAnalyzer *_spectrumAnalyzer = nullptr;
};
