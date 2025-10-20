//
// Created by Zero on 2025/10/20.
//

#ifndef LOCALCHAT_MAINWINDOW_H
#define LOCALCHAT_MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QDateTime>

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    private slots:
    void refreshUserList();
private:
    struct userInfo {
        QString name;
        QString ip;
        QDateTime lastConnectTime;
    };

    Ui::MainWindow *ui;
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
    QUdpSocket *udpSocket;
    QMap<QString, userInfo> map;

};


#endif //LOCALCHAT_MAINWINDOW_H
