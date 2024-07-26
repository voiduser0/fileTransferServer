#pragma once

#include <QTcpsocket>
#include <functional>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>

void SendMsg(QTcpSocket* tcpSocket, const QByteArray& bytes);
using pfunc = std::function<void(const QByteArray&)>;
void HandleMsg(QTcpSocket* tcpSocket, pfunc GetMsg);