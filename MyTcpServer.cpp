#include "MyTcpServer.h"

#include <QMetaType>

MyTcpServer::MyTcpServer(QTcpServer *parent)
	: QTcpServer(parent)
{
	qRegisterMetaType<qintptr>("qintptr");
}

MyTcpServer::~MyTcpServer()
{}

void MyTcpServer::incomingConnection(qintptr socketDescription)
{
	emit SigGetSocketDescription(socketDescription);
}
