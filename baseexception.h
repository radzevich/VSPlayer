#pragma once

#include <QException>
#include <QString>

class BaseException : public QException
{
public:
    BaseException(const char *errorMessage);
    BaseException(BaseException *exception);

    void raise() const override;
    BaseException *clone() const override;
    const char *what() const override;

private:
    const char*_errorMessage;
};
