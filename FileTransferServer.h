#pragma once

#include <QtWidgets/QWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QListView>
#include <QSettings>
#include <QThread>
#include <QMutexLocker>
#include <QMutex>

#include "ui_FileTransferServer.h"
#include "UserOperation.h"
#include "TcpServer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FileTransferServerClass; };
QT_END_NAMESPACE

struct SUserInfo
{
    QString qsAddr{};
    quint16 usPort{};
    QStandardItem* item{};
};

class FileTransferServer : public QWidget
{
    Q_OBJECT

public:
    FileTransferServer(QWidget *parent = nullptr);
    ~FileTransferServer();

private:
    Ui::FileTransferServerClass* ui;
    TcpServer* m_tcp;
    QString m_qsFilePath{};
    QHash<quint16, SUserInfo> m_hsUserInfo{};
    QStandardItemModel m_model{};
    UserOperation* m_userOp;

private:
    void AddUser(const quint32 ip, const QString& qsAddr, const quint16 usPort);
    void UpdateUser(const quint32 ip);
    void DeleteUser(const quint32 ip);
    void ClearUser();
    void SendFile(const QString& qsAddr, const quint16 usPort);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

public slots:
    void AppendPlainText(const QString &text);
    void GetMsg(const QString &qsAddr, const quint16 usPort, const QByteArray& bytes);

private slots:
    void on_selFileBtn_clicked();
    void on_sendFileBtn_clicked();
    void on_clearBtn_clicked();
    void on_userOpView_doubleClicked(const QModelIndex& index);

    void ReSend(const quint32 ipAddr);
    void KickUser(const quint32 ipAddr);

signals:
    void GetUserInfo(QString qsAddr, quint16 usPort);
    void SigDisConnected(const quint32 ipAddr);
};
