#include "FileTransferServerHandle.h"

constexpr int kMsgSecLength{ 10 };
struct SMsgHeader
{
	char msgHeader[kMsgSecLength] = { 0 };
	int msgSize{};
	char msgHeaderTail[kMsgSecLength] = { 0 };
};
void SendMsg(QTcpSocket* tcpSocket, const QByteArray& bytes)
{
	SMsgHeader msgHeader{};
	msgHeader.msgSize = bytes.size() + 1;
	tcpSocket->write((char*)&msgHeader, sizeof(msgHeader));
	char* msgData = new char[msgHeader.msgSize];
	memset(msgData, 0, msgHeader.msgSize);
	memcpy_s(msgData, msgHeader.msgSize, bytes.data(), bytes.size());
	for (int i{}; i < msgHeader.msgSize; i += 1024)
	{
		tcpSocket->write(msgData + i, qMin(1024, msgHeader.msgSize - i));
	}
	delete[]msgData;
	msgData = nullptr;
}

struct SMsgRecInfo
{
	quint64 recvNum{};				// 已经接收大小
	quint64 expectNum{};			// 预期接收大小
	QByteArray msgData;

	void Clear()
	{
		recvNum = 0;
		expectNum = 0;
		msgData.clear();
	}
};
using spMsgRecInfo = QSharedPointer<SMsgRecInfo>;
QHash<QTcpSocket*, spMsgRecInfo> gHsMsgRecInfo{};
QMutex gMtHandleMsg;
void HandleMsg(QTcpSocket* tcpSocket, pfunc GetMsg)
{
	QMutexLocker locker(&gMtHandleMsg);

	if (!gHsMsgRecInfo.contains(tcpSocket))
	{
		spMsgRecInfo spMsgRecInfo = QSharedPointer<SMsgRecInfo>::create();
		gHsMsgRecInfo.insert(tcpSocket, spMsgRecInfo);
	}
	spMsgRecInfo msgRecInfo = gHsMsgRecInfo[tcpSocket];
	while (tcpSocket->bytesAvailable() > 0)
	{
		if (msgRecInfo->recvNum == 0)
		{
			if (tcpSocket->bytesAvailable() < sizeof(SMsgHeader))
				return;

			QByteArray byteHeader = tcpSocket->read(sizeof(SMsgHeader));
			SMsgHeader* msgHeader = (SMsgHeader*)(byteHeader.data());
			msgRecInfo->expectNum = msgHeader->msgSize;
		}

		qint64 bytesAvailable = tcpSocket->bytesAvailable();
		if (bytesAvailable > 0)
		{
			//qint64 recvNum = qMin(bytesAvailable, gMsgInfo.expectNum - gMsgInfo.recvNum);
			msgRecInfo->msgData += tcpSocket->read(bytesAvailable);
			msgRecInfo->recvNum += bytesAvailable;

			if (msgRecInfo->expectNum == msgRecInfo->recvNum)
			{
				if (GetMsg != nullptr)
				{
					GetMsg(msgRecInfo->msgData);
				}
				msgRecInfo->Clear();
			}
		}
	}
}

