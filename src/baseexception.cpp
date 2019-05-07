#include "baseexception.h"

BaseException::BaseException(const char *errorMessage) : QException()
{
    _errorMessage = errorMessage;
}

BaseException::BaseException(BaseException *exception) : QException()
{
    _errorMessage = exception->what();
}

void BaseException::raise() const
{
    throw *this;
}

BaseException *BaseException::clone() const
{
    return new BaseException(*this);
}

const char* BaseException::what() const
{
    return _errorMessage;
}
