#ifndef AUDIODECODEREXCEPTION_H
#define AUDIODECODEREXCEPTION_H

#include <QException>

class AudioDecoderException : public QException
{
public:
    void raise() const override { throw *this; }
    AudioDecoderException *clone() const override { return new AudioDecoderException(*this); }
};

#endif // AUDIODECODEREXCEPTION_H
