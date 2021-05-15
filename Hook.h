#ifndef THOOK_H
#define THOOK_H

#include <QString>
#include <QTime>
#include <QThread>

//#include "Event.h"
#include "Client.h"
#include "DeviceHook.h"
#include "jsonclient.h"
#include <QJsonObject>

namespace hookSpace {
    class THook;
    enum hookErrors {connectionFailed, noConnection, hookIsRunned, hookIsStopped, errorStart, convertError};
}

class hookSpace::THook : public QObject{
Q_OBJECT
//Диспетчер, обеспечивает взаимодействие между всеми объектами
public:
    THook();
    ~THook();
    bool isOnline;
public slots:
    void onConnectionRequest(QString ip, int port);
    void onSocketConnected();
    void onDisconnected();
    void onErrorConnected();
    void runHook();
    void stopHook();
    void pauseHook();
    void resumeHook();
    void onHookDeviceRunned();
    void onHookDeviceStopped();
    void onHookDeviceError();
    void onChangeFileKeyName(QString name);
    void onChangeFileMouseName(QString name);
    void onChangeHotKeyName(QString name);
    void onChangeFormatName(QString name);
    void onChangePathName(QString name);
    void onEventReady(TEvent* event);
    void onConvertionDone(QString* event);
    void onConvertionToJsonDone(QJsonObject *event);
    void onConvertionError();
//    void onOnlineModeChanged(bool online);
signals:
// сигналы клиенту
    void requestConnection(const QString ip, const int port);
    void sendData(QString* data);
// сигналы mainwindow
    void connectionDone();
    void socketDisconnected();
    void hookStarted();
    void hookStopped();
    void hookPaused();
    void hookResumed();
// сигналы перехватчикам
    void run();
    void resume();
    void pause();
    void stop();
//установление файлов
    void setFileKeyName(const QString name);
    void setFileMouseName(const QString name);
    void setHotKeyName(const QString name);
    void setFormatName(const QString name);
    void setPathName(const QString name);
// сигналы конвертору
    void convertionRequest(TEvent* event);
    void convertionJsonRequest(TEvent* event);
// сигнал об ошибке
    void error(hookErrors err);
// сигнал в json
    void writeJson(QJsonObject* event);
private:
    void checkState();
    //THookMouseThread *mouseThread ;
    //THookKeyboardThread *keyboardThread;
    QThread threadConvertor, threadClient, threadHookDevices;
    TModifier modifier;
    bool isRun, isPaused;
    int countActiveHookDevice, countHookDeviceError;
    int countDevice;
    QTime timeOfClick;
    THookDevice *device;
};

#endif // THOOK_H
