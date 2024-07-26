#pragma once

#include <QTcpServer>

class MyTcpServer  : public QTcpServer
{
	Q_OBJECT

public:
	MyTcpServer(QTcpServer *parent = Q_NULLPTR);
	~MyTcpServer();

protected:
	virtual void incomingConnection(qintptr socketDescription);

signals:
	void SigGetSocketDescription(qintptr socketDescription);
};
