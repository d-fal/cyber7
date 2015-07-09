#include "home.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    home w;
    w.show();
    w.setWindowTitle("Cyber7");
    return a.exec();
}
