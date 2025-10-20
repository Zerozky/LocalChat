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
            }
        }
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::refreshUserList() {
    ui->listWidgetUsers->clear();
    for (const auto &user : map) {
        ui->listWidgetUsers->addItem(user.name + " - " + user.ip);
    }
}
