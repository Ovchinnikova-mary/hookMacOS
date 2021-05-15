#include <QString>
#include <QFile>
#include <QImage>

#include "Client.h"
#include "Event.h"

hookSpace::TClient::TClient(){
    countMessage = 1;
    xmlWriter.setDevice(this);

    connect(this, &QTcpSocket::connected, this, &hookSpace::TClient::onConnected);
    connect(this, &QTcpSocket::readyRead, this, &hookSpace::TClient::onReadyRead);
    connect(this, &QTcpSocket::disconnected, this, &hookSpace::TClient::onDisconnected);
}

void hookSpace::TClient::onConnectionRequest(const QString ip, const int port){
    this->connectToHost(ip, port);
    if (!this->waitForConnected(3000))
        emit errorConnected();
}

void hookSpace::TClient::onConnected(){
    xmlWriter.writeStartElement("connection");
    xmlWriter.writeAttribute("type", "Fineltec.Tutor.Capture");
    xmlWriter.writeEndElement();
    countMessage = 1;
    emit socketConnected();
}

void hookSpace::TClient::onReadyRead(){
    QXmlStreamReader xmlReader(this);
    while(!xmlReader.atEnd() && !xmlReader.hasError()){
        QXmlStreamReader::TokenType token = xmlReader.readNext();
        if (token == QXmlStreamReader::StartElement){
            if(xmlReader.name() == "close"){
                xmlWriter.writeStartElement("close");
                xmlWriter.writeEndElement();
            }
        }
    }
}

void hookSpace::TClient::onDisconnected(){
    emit socketDisconnected();
}

void hookSpace::TClient::onSendMessage(QString *data){
    if (this->state() == QTcpSocket::ConnectedState){
        xmlWriter.writeStartElement("message");
        xmlWriter.writeAttribute("number", QString::number(countMessage++));
        xmlWriter.writeDTD(*data);
        xmlWriter.writeEndElement();
    }
    else
        emit errorConnected();
    delete data;
}

void hookSpace::TClient::onStarted(){
    if (this->state() == QTcpSocket::ConnectedState){
        xmlWriter.writeStartElement("message");
        xmlWriter.writeAttribute("number", QString::number(countMessage++));
        xmlWriter.writeStartElement("content.start");
        xmlWriter.writeEndElement();
        xmlWriter.writeEndElement();
    }
    else
        emit errorConnected();
}

void hookSpace::TClient::onStopped(){
    if (this->state() == QTcpSocket::ConnectedState){
        xmlWriter.writeStartElement("message");
        xmlWriter.writeAttribute("number", QString::number(countMessage++));
        xmlWriter.writeStartElement("content.stop");
        xmlWriter.writeEndElement();
        xmlWriter.writeEndElement();
    }
    else
        emit errorConnected();
}
