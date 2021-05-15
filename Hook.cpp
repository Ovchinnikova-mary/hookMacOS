#include <QFile>
#include <QCursor>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Hook.h"

hookSpace::THook::THook(){
    isOnline=false;
    isRun = false;
    isPaused = false;
    countActiveHookDevice = 0;
    countHookDeviceError = 0;
    countDevice = 2;

    TClient* client = new TClient;
    client->moveToThread(&threadClient);
    threadClient.start(QThread::TimeCriticalPriority);

    TEventsConvertor *convertor = new TEventsConvertor;
    convertor->moveToThread(&threadConvertor);
    threadConvertor.start();
    THookKeyboard* hookKeyboard = new THookKeyboard(&modifier);
    THookMouse* hookMouse = new THookMouse(&modifier);
    hookKeyboard->moveToThread(&threadHookDevices);
    hookMouse->moveToThread(&threadHookDevices);
    threadHookDevices.start();
    JSONClient* jsonClient = new JSONClient;
    jsonClient->moveToThread(&threadClient);



    // Связывание сигналов и слотов
    // Все объекты общаются между собой не через вызов функций,
    // А через механизм сигналов и слотов
    // Функция слота объекта выполняется в потоке, в котором находится этот объект

    connect(this, &hookSpace::THook::requestConnection, client, &hookSpace::TClient::onConnectionRequest);
    connect(this, &hookSpace::THook::sendData, client, &hookSpace::TClient::onSendMessage);
    connect(this, &hookSpace::THook::run, client, &hookSpace::TClient::onStarted);
    connect(this, &hookSpace::THook::stop, client, &hookSpace::TClient::onStopped);
    connect(client, &hookSpace::TClient::socketConnected, this, &hookSpace::THook::onSocketConnected);
    connect(client, &hookSpace::TClient::errorConnected, this, &hookSpace::THook::onErrorConnected);
    connect(client, &hookSpace::TClient::socketDisconnected, this, &hookSpace::THook::onDisconnected);


    connect(this, &hookSpace::THook::run, jsonClient, &hookSpace::JSONClient::onStarted);
    connect(this, &hookSpace::THook::writeJson, jsonClient, &hookSpace::JSONClient::onWrite);
    connect(this, &hookSpace::THook::stop, jsonClient, &hookSpace::JSONClient::onStopped);

    //Pause
    connect(this, &hookSpace::THook::pause, hookKeyboard, &hookSpace::THookKeyboard::onHookStop);
    connect(this, &hookSpace::THook::pause, hookMouse, &hookSpace::THookMouse::onHookStop);
    connect(this, &hookSpace::THook::pause, client, &hookSpace::TClient::onStopped);
    //Resume
    connect(this, &hookSpace::THook::resume, hookKeyboard, &hookSpace::THookKeyboard::onHookRun);
    connect(this, &hookSpace::THook::resume, hookMouse, &hookSpace::THookMouse::onHookRun);
    connect(this, &hookSpace::THook::resume, client, &hookSpace::TClient::onStarted);
    //convertion event
    connect(this, &hookSpace::THook::convertionJsonRequest, convertor, &hookSpace::TEventsConvertor::onConvertEventToJson);
    connect(convertor, &hookSpace::TEventsConvertor::convertToJsonReady, this, &hookSpace::THook::onConvertionToJsonDone);

    connect(this, &hookSpace::THook::convertionRequest, convertor, &hookSpace::TEventsConvertor::onConvertEvent);
    connect(convertor, &hookSpace::TEventsConvertor::convertReady, this, &hookSpace::THook::onConvertionDone);
    connect(convertor, &hookSpace::TEventsConvertor::convertError, this, &hookSpace::THook::onConvertionError);
    //actives keyboard
    connect(this, &hookSpace::THook::run, hookKeyboard, &hookSpace::THookKeyboard::onHookRun);
    connect(this, &hookSpace::THook::stop, hookKeyboard, &hookSpace::THookKeyboard::onHookStop);
    connect(this, &hookSpace::THook::setFileKeyName, hookKeyboard, &hookSpace::THookKeyboard::setFileName);
    connect(this, &hookSpace::THook::setHotKeyName, hookKeyboard, &hookSpace::THookKeyboard::setHotKey);
    connect(this, &hookSpace::THook::setFormatName, hookKeyboard, &hookSpace::THookKeyboard::setFormat);
    connect(this, &hookSpace::THook::setPathName, hookKeyboard, &hookSpace::THookKeyboard::setPath);

    connect(hookKeyboard, &hookSpace::THookKeyboard::pressHotKey, this, &hookSpace::THook::stopHook);
    connect(hookKeyboard, &hookSpace::THookKeyboard::hookDeviceRunned, this, &hookSpace::THook::onHookDeviceRunned);
    connect(hookKeyboard, &hookSpace::THookKeyboard::hookDeviceStopped, this, &hookSpace::THook::onHookDeviceStopped);
    connect(hookKeyboard, &hookSpace::THookKeyboard::hookDeviceError, this, &hookSpace::THook::onHookDeviceError);
    connect(hookKeyboard, &hookSpace::THookKeyboard::eventReady, this, &hookSpace::THook::onEventReady);
    //actives mouse
    connect(this, &hookSpace::THook::run, hookMouse, &hookSpace::THookMouse::onHookRun);
    connect(this, &hookSpace::THook::stop, hookMouse, &hookSpace::THookMouse::onHookStop);
    connect(this, &hookSpace::THook::setFileMouseName, hookMouse, &hookSpace::THookMouse::setFileName);
    connect(this, &hookSpace::THook::setFormatName, hookMouse, &hookSpace::THookMouse::setFormat);
    connect(this, &hookSpace::THook::setPathName, hookMouse, &hookSpace::THookMouse::setPath);
   // connect(this, &hookSpace::THook::setHotKeyName, hookMouse, &hookSpace::THookMouse::setHotKey);

    connect(hookMouse, &hookSpace::THookMouse::hookDeviceRunned, this, &hookSpace::THook::onHookDeviceRunned);
    connect(hookMouse, &hookSpace::THookMouse::hookDeviceStopped, this, &hookSpace::THook::onHookDeviceStopped);
    connect(hookMouse, &hookSpace::THookMouse::hookDeviceError, this, &hookSpace::THook::onHookDeviceError);
    connect(hookMouse, &hookSpace::THookMouse::eventReady, this, &hookSpace::THook::onEventReady);
    //additional
    connect(hookMouse, &hookSpace::THookMouse::changeStatusEvent, hookKeyboard, &hookSpace::THookKeyboard::onChangeStatusEvent);
    connect(hookKeyboard, &hookSpace::THookKeyboard::changeStatusEvent, hookKeyboard, &hookSpace::THookKeyboard::onChangeStatusEvent);
}

hookSpace::THook::~THook(){
    if(isRun)
        emit stop();

    threadClient.quit();
    threadConvertor.quit();
    threadHookDevices.quit();
    threadConvertor.wait();
    threadClient.wait();
    //threadHookDevices.wait();
}

void hookSpace::THook::onConnectionRequest(QString ip, int port){
    if (isOnline)
        emit requestConnection(ip, port);
}

void hookSpace::THook::onSocketConnected(){
    if (isOnline)
        emit connectionDone();
}

void hookSpace::THook::onDisconnected(){
    if (isOnline){
        emit socketDisconnected();
        if(isRun)
            emit stop();
    }
}

void hookSpace::THook::onErrorConnected(){
    if (isOnline)
        emit error(connectionFailed);
}

void hookSpace::THook::pauseHook(){
    if(isRun)
    {
        emit pause();
        emit hookPaused();
    }
    else
        emit error(hookIsStopped);
}

void hookSpace::THook::resumeHook(){
    if(!isRun){
        emit resume();
        emit hookResumed();
    }
    else
        emit error(hookIsStopped);
}

void hookSpace::THook::runHook(){
    if(isRun)
        emit error(hookIsRunned);
    else{

        emit run();

}
}
/*
void hookSpace::THook::onOnlineModeChanged(bool online){
    isOnline=online;
}
*/
void hookSpace::THook::stopHook(){
    if(isRun)
        emit stop();
    else
        emit error(hookIsStopped);
}

void hookSpace::THook::checkState(){
    if (countActiveHookDevice == countDevice){
        isRun = true;
        emit hookStarted();
    }
    else
        if(countActiveHookDevice + countHookDeviceError == countDevice){
            emit stop();
            emit error(errorStart);
            countHookDeviceError = 0;
        }
}

void hookSpace::THook::onHookDeviceRunned(){
    countActiveHookDevice++;
    checkState();

}

void hookSpace::THook::onHookDeviceStopped(){
   /*if (countActiveHookDevice == 0){
        isRun = false;
        emit hookStopped();
    }
    else if(--countActiveHookDevice == 0){
        isRun = false;
        emit hookStopped();
    }*/
    if (--countActiveHookDevice == 0){
        isRun = false;
        emit hookStopped();
    }

}

void hookSpace::THook::onHookDeviceError(){
    countHookDeviceError++;
    checkState();
}

void hookSpace::THook::onChangeFileKeyName(QString name){
    emit setFileKeyName(name);
}

void hookSpace::THook::onChangeFileMouseName(QString name){
    emit setFileMouseName(name);
}

void hookSpace::THook::onChangeHotKeyName(QString name){
    emit setHotKeyName(name);
}

void hookSpace::THook::onChangeFormatName(QString name){
    emit setFormatName(name);
}

void hookSpace::THook::onChangePathName(QString name){
    emit setPathName(name);
}

void hookSpace::THook::onEventReady(TEvent* event){
    //TEvent *currentEvent = dynamic_cast<hookSpace::TEvent*>(event);
    //dynamic_cast<hookSpace::TMouseEvent*>(event)->getCountClick()
    while(!(dynamic_cast<hookSpace::TEvent*>(event)->getScreenSave()) && timeOfClick.msecsTo(QTime::currentTime()) < 1000)  { sleep(0); }
    dynamic_cast<hookSpace::TEvent*>(event)->setScreenSave(false);
    qDebug() << "event ready start convertion";
    qDebug() << "event: " << event;

    emit convertionRequest(event);
    emit convertionJsonRequest(event);
}

void hookSpace::THook::onConvertionDone(QString *event){
    if(isRun)
    {
        emit sendData(event);
    }
    else
        delete event;
}

void hookSpace::THook::onConvertionToJsonDone(QJsonObject *event){
    if(isRun)
        emit writeJson(event);
    else
        delete event;
}

void hookSpace::THook::onConvertionError(){
    emit error(convertError);
}
