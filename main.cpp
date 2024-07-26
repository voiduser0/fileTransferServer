#include "FileTransferServer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FileTransferServer w;
    w.show();
    return a.exec();
}
