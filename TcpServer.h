#pragma once

#include <QSet>
#include <QHash>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>

#include "FileTransferServerHandle.h"

class TcpServer  : public QObject
{
	Q_OBJECT

private:
	TcpServer(QObject *parent = Q_NULLPTR);
	TcpServer(const TcpServer &other);

public:
	static TcpServer* GetTcpServer();
	
private:
	QTcpServer* m_tcpServer{};
	QHash<quint32, QTcpSocket*> m_hsSocket{};
	QSet<quint32> m_setIp;
	static TcpServer* m_instanceTcpServer;

public:
	~TcpServer();
	void Listen(const quint16 port);
	void Close();

signals:
	void SendPlainText(const QString &text);
	void GetMsg(const QString &qsAddr, const quint16 usPort, const QByteArray &bytes);

public slots:
	void SendBytes(const QByteArray& fileBytes, const QString& qsAddr = "", const quint16 usPort = 0);
	void DisConnected(const quint32 ip);

private slots:
	void HandleNewConnection();
	void ReadMsg();
};
