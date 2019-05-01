#ifndef AUDIOSEARCHENGINEEXCEPTION_H
#define AUDIOSEARCHENGINEEXCEPTION_H

#include <QException>

class AudioSearchEngineException : public QException
{
public:
    void raise() const override { throw *this; }
    AudioSearchEngineException *clone() const override { return new AudioSearchEngineException(*this); }
};

#endif // AUDIOSEARCHENGINEEXCEPTION_H
