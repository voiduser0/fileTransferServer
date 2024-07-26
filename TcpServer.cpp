#include "TcpServer.h"
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <qDebug>

namespace Addr {
	inline namespace Ipv4 {
		QHostAddress GetFirstIpv4Addr()
		{
			QHostAddress haAddr{};
			bool flag{};
			QList<QHostAddress> lsHostAddr = QNetworkInterface::allAddresses();
			for (const QHostAddress& hostAddr : lsHostAddr)
			{
				if (hostAddr.protocol() == QAbstractSocket::IPv4Protocol && !hostAddr.isNull())
				{
					haAddr = hostAddr;
					flag = true;
					break;
				}
			}
			if (!flag)
			{
				haAddr = QHostAddress::LocalHost;
			}
			return haAddr;
		}
	}
};

TcpServer* TcpServer::m_instanceTcpServer = nullptr;


TcpServer::TcpServer(QObject* parent)
	: QObject(parent)
{
}

TcpServer* TcpServer::GetTcpServer()
{
	if (m_instanceTcpServer == nullptr)
	{
		m_instanceTcpServer = new TcpServer;
	}
	return m_instanceTcpServer;
}

TcpServer::~TcpServer()
{
	Close();
}

void TcpServer::Listen(const quint16 port)
{
	if (m_tcpServer == nullptr)
		m_tcpServer = new QTcpServer(this);

	if (!m_tcpServer->listen(Addr::GetFirstIpv4Addr(), port))
	{
		emit SendPlainText(QStringLiteral("¼àÌý¶Ë¿ÚÊ§°Ü£º") + QString::number(port));
		return;
	}
	connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::HandleNewConnection);
}

void TcpServer::HandleNewConnection()
{
	QTcpSocket* tcpSocket = m_tcpServer->nextPendingConnection();
	QHostAddress haAddr = tcpSocket->peerAddress();
	quint32 ipAddr = haAddr.toIPv4Address();

	if (m_setIp.contains(ipAddr))
		return;
	m_setIp.insert(ipAddr);

	m_hsSocket.insert(ipAddr, tcpSocket);

	connect(m_hsSocket.value(ipAddr), &QTcpSocket::readyRead, this, &TcpServer::ReadMsg);

	SendBytes("SUCCESSFUL|", haAddr.toString(), m_hsSocket.value(ipAddr)->peerPort());
	emit SendPlainText(QString("Connecting: %1").arg(haAddr.toString()));
}


void TcpServer::ReadMsg()
{
	QTcpSocket* tcpSocket = (QTcpSocket*)(sender());

	HandleMsg(tcpSocket, [&](const QByteArray& bytes) {
		emit GetMsg(tcpSocket->peerAddress().toString(), tcpSocket->peerPort(), bytes);
		});
}


void TcpServer::SendBytes(const QByteArray& fileBytes, const QString& qsAddr, const quint16 usPort)
{
	qDebug() << QThread::currentThread();
	for (const auto& socket : m_hsSocket)
	{
		if (!qsAddr.isEmpty() && QHostAddress(qsAddr) != socket->peerAddress())
			continue;

		if (usPort == 0)
		{
			SendMsg(socket, fileBytes);
		}
		else if (usPort == socket->peerPort())
		{
			SendMsg(socket, fileBytes);
			break;
		}
	}
}

void TcpServer::DisConnected(const quint32 ip)
{
	auto it = m_hsSocket.find(ip);
	QTcpSocket* socket = it.value();
	socket->disconnectFromHost();
	if (socket->state() == QAbstractSocket::UnconnectedState
		|| socket->waitForDisconnected(1000))
	{
		emit SendPlainText(QString("Disconnected: %1").arg(socket->peerAddress().toString()));
		m_setIp.remove(ip);
		socket->deleteLater();
		it = m_hsSocket.erase(it);
	}
}

void TcpServer::Close()
{
	if (!m_tcpServer)
		return;

	for (QTcpSocket* socket : m_hsSocket)
	{
		if (socket && socket->state() != QAbstractSocket::UnconnectedState)
		{
			socket->disconnectFromHost();
			socket->waitForDisconnected();
		}
		delete socket;
		socket = nullptr;
	}
	m_hsSocket.clear();
	m_setIp.clear();
	m_tcpServer->close();
	delete m_instanceTcpServer;
	m_instanceTcpServer = nullptr;
}