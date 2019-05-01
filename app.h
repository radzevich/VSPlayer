#ifndef APP_H
#define APP_H

#include <QApplication>

class App : public QApplication
{
    Q_OBJECT

public:
    explicit App(int& argc, char** argv);
    ~App();
};

#endif // APP_H
