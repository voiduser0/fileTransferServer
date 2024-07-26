#include "UserOperation.h"

UserOperation::UserOperation(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::UserOperationClass())
{
	ui->setupUi(this);
}

UserOperation::~UserOperation()
{
	delete ui;
}

void UserOperation::on_reSendBtn_clicked()
{
	quint32 ipAddr = QHostAddress(ui->ipLabel->text()).toIPv4Address();
	emit SigReSend(ipAddr);
}

void UserOperation::on_kickBtn_clicked()
{
	quint32 ipAddr = QHostAddress(ui->ipLabel->text()).toIPv4Address();
	emit SigKickUser(ipAddr);
}

void UserOperation::GetUserInfo(QString qsAddr, quint16 usSeqNum)
{
	ui->ipLabel->setText(qsAddr);
	ui->PortLabel->setText(QString::number(usSeqNum));
}
