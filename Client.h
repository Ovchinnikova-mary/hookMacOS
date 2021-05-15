#ifndef CLIENT_H
#define CLIENT_H

#include <QtNetwork>
#include <QMainWindow>

namespace hookSpace{
class TClient;
}

class hookSpace::TClient : public QTcpSocket{
    Q_OBJECT
public:
    TClient();

public slots:
    void onConnectionRequest(const QString ip, const int port);
    void onConnected();
    void onReadyRead();
    void onSendMessage(QString *data);
    void onStarted();
    void onStopped();
    void onDisconnected();

signals:
    void errorConnected();
    void socketConnected();
    void socketDisconnected();

private:
    int countMessage;
    bool hookRun;
    QXmlStreamWriter xmlWriter;
};

#endif // CLIENT_H
