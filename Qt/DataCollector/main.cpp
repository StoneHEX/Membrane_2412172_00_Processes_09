#include "datacollector.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DataCollector w;
    w.show();
    return a.exec();
}
