#ifndef JSONCLIENT_H
#define JSONCLIENT_H


#include <QJsonDocument>
#include <QMainWindow>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QList>

namespace hookSpace{
    class JSONClient;
}

class hookSpace::JSONClient : public QObject{
    Q_OBJECT
public:
JSONClient();

public slots:
    void onStarted();
    void onStopped();
    void onWrite(QJsonObject *jsonObject);
signals:

private:
    int countMessage;
    bool hookRun;
    QJsonDocument *document;
    QJsonArray *frameArray;
    QList<QJsonObject *> *jsonObjects;
    void writeScrollEvent(QList<QJsonObject *> *jsonObjects);
};

#endif // JSONCLIENT_H
