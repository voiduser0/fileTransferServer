#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>

#include "FileTransferServer.h"

FileTransferServer::FileTransferServer(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::FileTransferServerClass())
{
	ui->setupUi(this);

	resize(1200, 800);

	m_tcp = TcpServer::GetTcpServer();
	m_tcp->Listen(9999);
	connect(m_tcp, &TcpServer::SendPlainText, this, &FileTransferServer::AppendPlainText);
	connect(m_tcp, &TcpServer::GetMsg, this, &FileTransferServer::GetMsg);
	connect(this, &FileTransferServer::SigDisConnected, m_tcp, &TcpServer::DisConnected);

	m_userOp = new  UserOperation(this);
	connect(this, &FileTransferServer::GetUserInfo, m_userOp, &UserOperation::GetUserInfo);
	connect(m_userOp, &UserOperation::SigReSend, this, &FileTransferServer::ReSend);
	connect(m_userOp, &UserOperation::SigKickUser, this, &FileTransferServer::KickUser);

	QSettings settings("Path.ini", QSettings::IniFormat);
	m_qsFilePath = settings.value("path").toString();
}

FileTransferServer::~FileTransferServer()
{
	ClearUser();
	delete ui;
	delete m_tcp;
}

void FileTransferServer::on_selFileBtn_clicked()
{
	QString qsFilePath{ QFileDialog::getOpenFileName(this, 
		QStringLiteral("请选择想要发送的文件"), m_qsFilePath, QStringLiteral("任何文件(*)"))};
	ui->selFilePath->setText(qsFilePath);
}

void FileTransferServer::GetMsg(const QString &qsAddr, const quint16 usPort, const QByteArray& bytes)
{
	quint32 ipAddr = QHostAddress(qsAddr).toIPv4Address();
	QByteArrayList lbaBytes = bytes.split('|');
	if (lbaBytes[0].contains("NEWCONNECTION"))
	{
		AddUser(ipAddr, qsAddr, usPort);
	}
	else if (lbaBytes[0].contains("GETFILE"))
	{
		UpdateUser(ipAddr);
	}
	else if (lbaBytes[0].contains("DISCONNECTION"))
	{
		DeleteUser(ipAddr);
	}
}

QMutex gMtUserLocker;
void FileTransferServer::AddUser(const quint32 ip, const QString& qsAddr, const quint16 usPort)
{
	QMutexLocker locker(&gMtUserLocker);
	SUserInfo userInfo{ qsAddr, usPort, new QStandardItem };
	m_hsUserInfo.insert(ip, userInfo);
	SUserInfo &user = m_hsUserInfo[ip];
	user.item->setData(QIcon(":/FileTransferServer/images/no.png"), Qt::DecorationRole);
	user.item->setData(QString("IP Address: %1").arg(user.qsAddr), Qt::DisplayRole);
	user.item->setEditable(false);
	m_model.appendRow(user.item);
	ui->userOpView->setModel(&m_model);
	ui->numberLabel->setText(QStringLiteral("在线人数：") + QString::number(m_hsUserInfo.size()));
}

void FileTransferServer::UpdateUser(const quint32 ip)
{
	m_hsUserInfo.value(ip).item->setData(QIcon(":/FileTransferServer/images/ok.png"), Qt::DecorationRole);
}

void FileTransferServer::DeleteUser(const quint32 ip)
{
	QMutexLocker locker(&gMtUserLocker);
	if (!m_hsUserInfo.contains(ip))
		return;
	emit SigDisConnected(ip);
	auto itUserInfo = m_hsUserInfo.find(ip);
	delete itUserInfo.value().item;
	itUserInfo.value().item = nullptr;
	QString qsAddr = itUserInfo.value().qsAddr;
	quint16 usPort = itUserInfo.value().usPort;
	m_hsUserInfo.erase(itUserInfo);
	m_userOp->close();
	m_model.setRowCount(m_hsUserInfo.size());
	ui->numberLabel->setText(QStringLiteral("在线人数：") + QString::number(m_hsUserInfo.size()));
}

void FileTransferServer::ClearUser()
{
	QMutexLocker locker(&gMtUserLocker);
	auto beg = m_hsUserInfo.begin();
	while (beg != m_hsUserInfo.end())
	{
		emit SigDisConnected(QHostAddress(beg.value().qsAddr).toIPv4Address());
		delete beg.value().item;
		beg.value().item = nullptr;
		beg = m_hsUserInfo.erase(beg);
	}
	m_userOp->close();
}

void FileTransferServer::SendFile(const QString& qsAddr, const quint16 usPort)
{
	QString qsFileName{ ui->selFilePath->text() };
	QFile file(qsFileName);
	if (!file.exists())
		return;
	file.open(QIODevice::ReadOnly);
	QFileInfo fileInfo(qsFileName);
	QString newFlag = "NEWFILE|";
	QByteArray fileArray = newFlag.toLocal8Bit() + fileInfo.fileName().toLocal8Bit() + "|" + file.readAll();
	m_tcp->SendBytes(fileArray, qsAddr, usPort);
	file.close();
}

void FileTransferServer::closeEvent(QCloseEvent* event)
{
	QSettings settings("Path.ini", QSettings::IniFormat);
	if (!ui->selFilePath->text().isEmpty())
		settings.setValue("path", ui->selFilePath->text());
}

void FileTransferServer::on_clearBtn_clicked()
{
	ClearUser();
	m_model.setRowCount(m_hsUserInfo.size());
	ui->numberLabel->setText(QStringLiteral("在线人数：") + QString::number(m_hsUserInfo.size()));
}

void FileTransferServer::on_sendFileBtn_clicked()
{
	SendFile("", 0);
}

void FileTransferServer::on_userOpView_doubleClicked(const QModelIndex& index)
{
	QString qsData = index.data().toString();
	int i = qsData.indexOf(":") + 2;
	QString qsAddr = qsData.mid(i);
	quint32 ipAddr = QHostAddress(qsAddr).toIPv4Address();
	auto user = m_hsUserInfo.value(ipAddr);
	emit GetUserInfo(user.qsAddr, user.usPort);
	m_userOp->show();
}

void FileTransferServer::ReSend(const quint32 ipAddr)
{
	SendFile(m_hsUserInfo.value(ipAddr).qsAddr, m_hsUserInfo.value(ipAddr).usPort);
}

void FileTransferServer::KickUser(const quint32 ipAddr)
{
	DeleteUser(ipAddr);
}

void FileTransferServer::AppendPlainText(const QString &text)
{
	ui->historyInfo->appendPlainText(text);
}