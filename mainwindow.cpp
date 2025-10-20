//
// Created by Zero on 2025/10/20.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainwindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(55555, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    connect(ui->pushButtonLogin, &QPushButton::clicked, [this]() {
        QString name = ui->lineEditUserName->text();
        if (name.isEmpty()) {
            name = "Anonymous";
        }
        QByteArray data = QString("LOGIN:" + name.trimmed()).toUtf8();
        udpSocket->writeDatagram(data, QHostAddress("192.168.0.255"), 55555);
        ui->textEditInfo->append("Logged in as " + name);
    });

    connect(udpSocket, &QUdpSocket::readyRead, [this]() {
        while (udpSocket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(static_cast<int>(udpSocket->pendingDatagramSize()));
            QHostAddress sender;
            quint16 senderPort;

            udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            QString message = QString::fromUtf8(datagram);

            if (message.startsWith("LOGIN:")) {
                QString name = message.mid(6);
                ui->textEditInfo->append(name + " (" + sender.toString() + ") logged in.");
                map[sender.toString()] = {name, sender.toString(), QDateTime::currentDateTime()};
                refreshUserList();
            } else if (message.startsWith("LOGOUT:")) {
                QString name = message.mid(7);
                ui->textEditInfo->append(name + " (" + sender.toString() + ") logged out.");
                map.remove(sender.toString());
                refreshUserList();
            }
        }
    });

    connect(this, &MainWindow::destroyed, [this]() {
        QByteArray data = QString("LOGOUT").toUtf8();
        udpSocket->writeDatagram(data, QHostAddress("192.168.0.255"), 55555);
    });

    connect(ui->listWidgetUsers, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item) {
        QString userInfo = item->text();
        QStringList parts = userInfo.split(" - ");
        if (parts.size() == 2) {
            QString userName = parts[0];
            QString userIp = parts[1];
            ui->textEditInfo->append("Starting chat with " + userName + " at " + userIp);
            // 服务端
            tcpServer = new QTcpServer(this);
            connect(tcpServer, &QTcpServer::newConnection, [this]() {
                QTcpSocket *socket = tcpServer->nextPendingConnection();
                ui->textEditInfo->append("Chat connection established with " + socket->peerAddress().toString());
                connect(socket, &QTcpSocket::readyRead, [this, socket]() {
                    QString clientID = socket->peerAddress().toString();
                    QTime currentTime = QTime::currentTime();
                    QString message = QString("[%1] - %2 : ").arg(clientID).arg(currentTime.toString())
                                      + socket->readAll();
                    ui->textEditDisplayMessage->append(message);
                });
            });
            tcpServer->listen(QHostAddress::Any, 55555);
            // 客户端
            tcpSocket = new QTcpSocket(this);
            tcpSocket->connectToHost(userIp, 55555);
            connect(tcpSocket, &QTcpSocket::connected, [this, userName, userIp]() {
                ui->textEditInfo->append("Connected to " + userName + " at " + userIp);
            });
            tcpSocket->write("你好，这里是:" + ui->lineEditUserName->text().toUtf8());
        }
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::refreshUserList() {
    ui->listWidgetUsers->clear();
    for (const auto &user: map) {
        ui->listWidgetUsers->addItem(user.name + " - " + user.ip);
    }
}
