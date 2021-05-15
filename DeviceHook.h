#ifndef DEVICEHOOK_H
#define DEVICEHOOK_H

#include <QString>
#include <QThread>
#include <QTime>
#include <QTimer>
#include "Event.h"

#include <QtMac>
#include <ApplicationServices/ApplicationServices.h>
#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
#endif

#ifndef Q_WS_MAC
namespace hookSpace {
    class THookDevice;
    class THookMouse;
    class THookKeyboard;
    class THookKeyboardThread;
    class THookMouseThread;
    class DelayThread;
    void getScreenCrutch(THookDevice* hookDev, TEvent* ev);
}

class hookSpace::THookDevice : public QObject{
    Q_OBJECT
public:
    THookDevice(TModifier *modif);
    ~THookDevice();
    char fileName[100];
    char hotKey[100];
    char format[100];
    char path[200];
    virtual void getEvent()=0;
public slots:
    void setFileName(const QString name);
    void setHotKey(const QString name);
    void setFormat(const QString name);
    void setPath(const QString name);
    void onHookRun();
    void onHookStop();
    void onGetScreen(hookSpace::TEvent* event);
signals:
    void runThread();
    void hookDeviceError();
    void hookDeviceRunned();
    void hookDeviceStopped();
    void eventReady(TEvent *event);
    void changeStatusEvent(bool isNew);
    void setModifiers(TEvent* ev);
    void getScreen(hookSpace::TEvent* event);
    void pressHotKey();
protected:
    void run();
    void sendEvent();
    virtual QString generateImageName() = 0;
    int fd;
    int countScreen;
    FILE *driverFile;
    TEvent *event;
    TModifier *modifier;
    bool isRun;
    bool eventIsNew;
    bool keyDownModifier;
    bool HotKey;
};

class hookSpace::THookMouse : public hookSpace::THookDevice{
    Q_OBJECT
public:
    THookMouse(TModifier *modif);
    THookMouse(TModifier *modif, QTimer *timer);
    void getEvent();

private slots:
        void runTimer(TEvent *event);
            void runTimer();
    //void checkDelayEvent();
private:
    void signalProcessing(unsigned char x1, int x2, int x3, int x4);
    TMouseEvent* createNewDragEvent(TypesOfEvents typeDrag);
    TMouseEvent* createNewClickEvent(TypesOfMouseButton typeButton);
    TMouseEvent* createNewScrollEvent(Ways way);
    TMouseEvent *previousEvent;
    QString generateImageName();
    int x,y;
    bool timerRun, mouseDown, dragRun;
    CGPoint lastCursorPos;
    QTime timeOfClick;
    void signalProcessing(void *event_data);
};

class hookSpace::THookKeyboard : public hookSpace::THookDevice{
    Q_OBJECT
public:
    THookKeyboard(TModifier *modif);
    void getEvent();

public slots:
    void onChangeStatusEvent(bool isNew);
signals:
    void addModifier(ModifierNames m);
    void removeModifier(ModifierNames m);

private:
    void signalProcessing(int keyCode, int eventCode);
    void signalProcessing(void *event_data);
    ModifierNames modifierName(int keyCode);
    ModifierNames keyModifierName(int keyCode);
    QString searchKey(int keyCode);
    TKeyPressEvent* createNewKeyPressEvent();
    QString generateImageName();
    QTime timeOfClick;
};

class hookSpace::DelayThread:public QThread{
    Q_OBJECT
public:
    explicit DelayThread(TEvent *event, QTime timeOfClick);
    void run();
    TEvent *event;
    QTime timeOfClick;
    THookDevice *dev;
public slots:
    void stop();
signals:
    void send(TEvent *event);
    void send();
};
#endif // Q_WS_MAC
#endif // DEVICEHOOK_H
