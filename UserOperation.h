#pragma once

#include <QMainWindow>
#include <QHostAddress>

#include "ui_UserOperation.h"

QT_BEGIN_NAMESPACE
namespace Ui { class UserOperationClass; };
QT_END_NAMESPACE

class UserOperation : public QMainWindow
{
	Q_OBJECT

public:
	UserOperation(QWidget *parent = nullptr);
	~UserOperation();

private:
	Ui::UserOperationClass *ui;

private slots:
	void on_reSendBtn_clicked();
	void on_kickBtn_clicked();

public slots:
	void GetUserInfo(QString qsAddr, quint16 usPort);

signals:
	void SigReSend(const quint32 ipAddr);
	void SigKickUser(const quint32 ipAddr);
};
