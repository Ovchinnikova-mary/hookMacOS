#include <QtConcurrent/QtConcurrent>
#include <QFile>
#include <QCursor>

#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include <QWindow>
#include <QWidget>

#include <fcntl.h>
#include "DeviceHook.h"

hookSpace::THookDevice::THookDevice(TModifier *modif){
    modifier = modif;
    event = 0, isRun = false, countScreen = 0;
    eventIsNew = true;
    fd=0;
    keyDownModifier = false;
    HotKey = false;
    connect(this, &hookSpace::THookDevice::setModifiers, modifier, &hookSpace::TModifier::setModifiers);
    connect(this, &hookSpace::THookDevice::getScreen, this, &hookSpace::THookDevice::onGetScreen);
}


void hookSpace::THookDevice::setFileName(const QString name){
    strcpy(fileName, name.toLocal8Bit().data());
}

void hookSpace::THookDevice::setHotKey(const QString name){
    strcpy(hotKey, name.toLocal8Bit().data());
}

void hookSpace::THookDevice::setFormat(const QString name){
    strcpy(format, name.toLocal8Bit().data());
}

void hookSpace::THookDevice::setPath(const QString name){
    strcpy(path, name.toLocal8Bit().data());
}

void hookSpace::THookDevice::run(){
    
    //while(isRun)
    if(isRun)
        this->getEvent();
}

QPixmap shootScreen()
{
    CGImageRef windowImage = CGDisplayCreateImage(CGMainDisplayID());
    windowImage = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionOnScreenOnly, 0, kCGWindowImageBoundsIgnoreFraming);
    QPixmap originalPixmap = QtMac::fromCGImageRef(windowImage);
    CGImageRelease(windowImage);
    qDebug() << "size screen " << originalPixmap.size();
    return originalPixmap;

}

void hookSpace::THookDevice::onGetScreen(hookSpace::TEvent* event){
    const int STR_EQUAL = 0;
    qDebug() << "start screenshot; format: " << format;
    qDebug() << "path: " << path;
    QString nameImage = this->generateImageName();
    //QString *name = new QString(path + QString("/Images/") + nameImage + QString(".") + format);
    QString *name = new QString(path + QString("/Images/") + nameImage + QString(".") + format);
    QFile file(*name);
    file.open(QIODevice::WriteOnly);
    if (QString::compare(format,"png") == STR_EQUAL){
        shootScreen().save(&file, "PNG");
    }
    else if (QString::compare(format,"jpeg") == STR_EQUAL){
        shootScreen().save(&file, "JPEG");
    }
    event->addImageName(name);
}


hookSpace::THookMouse::THookMouse(TModifier *modif)
    : hookSpace::THookDevice::THookDevice(modif)
{
    timerRun = false, mouseDown = false;
    dragRun = false;
}

QString hookSpace::THookMouse::generateImageName(){
    return "MouseEvent" + QString::number(countScreen++);
}

hookSpace::TMouseEvent* hookSpace::THookMouse::createNewDragEvent(TypesOfEvents typeDrag){
    TMouseEvent *ev = new TMouseEvent(typeDrag);
    emit getScreen(ev);
    emit setModifiers(ev);
    emit changeStatusEvent(true);
    return ev;
}

hookSpace::TMouseEvent* hookSpace::THookMouse::createNewClickEvent(TypesOfMouseButton typeButton){
    TMouseEvent *ev = new TMouseEvent(typeButton);
    emit getScreen(ev);
    emit setModifiers(ev);
    emit changeStatusEvent(true);
    return ev;
}

hookSpace::TMouseEvent* hookSpace::THookMouse::createNewScrollEvent(Ways way){
    TMouseEvent *ev = new TMouseEvent(way);
    emit getScreen(ev);
    emit setModifiers(ev);
    emit changeStatusEvent(true);
    return ev;
}

void hookSpace::THookMouse::runTimer(){
    //   while(timerRun && dynamic_cast<hookSpace::TMouseEvent*>(event1)->getCountClick() != 2 && timeOfClick.msecsTo(QTime::currentTime()) < 500)  { sleep(0); }
    // пол секунды на то, чтобы сделать второй клик, значение практически выдумано
    // timerRun = false;
    // emit changeStatusEvent(false);
    // emit eventReady(event);
    qDebug()<<"test";
    //emit eventReady(nullptr);
}
void hookSpace::THookMouse::runTimer(TEvent *event1){
 //   while(timerRun && dynamic_cast<hookSpace::TMouseEvent*>(event1)->getCountClick() != 2 && timeOfClick.msecsTo(QTime::currentTime()) < 500)  { sleep(0); }
    // пол секунды на то, чтобы сделать второй клик, значение практически выдумано
    timerRun = false;
    emit changeStatusEvent(false);
    emit eventReady(event1);
}

hookSpace::THookKeyboard::THookKeyboard(TModifier *modif)
    : hookSpace::THookDevice::THookDevice(modif)
{
    connect(this, &hookSpace::THookKeyboard::addModifier, modifier, &hookSpace::TModifier::addModifier);
    connect(this, &hookSpace::THookKeyboard::removeModifier, modifier, &hookSpace::TModifier::removeModifier);
}

void hookSpace::THookKeyboard::onChangeStatusEvent(bool isNew){
    // Если данный слот связан с сигналами changeStausEvent,
    // то изменение статуса происходит в потоке, в котором создан объект данного класса
    eventIsNew = isNew;
}

QString hookSpace::THookKeyboard::generateImageName(){
    return "KeyBoardEvent" + QString::number(countScreen++);
}

hookSpace::TKeyPressEvent* hookSpace::THookKeyboard::createNewKeyPressEvent(){
    TKeyPressEvent* ev = new TKeyPressEvent();
    emit getScreen(ev);
    emit changeStatusEvent(true);
    return ev;
}

hookSpace::ModifierNames hookSpace::THookKeyboard::modifierName(int keyCode){
    ModifierNames res = no;
    if (keyCode == 262401 || keyCode == 270592){ res = ctrl; }
    if (keyCode == 131330 || keyCode == 131332){ res = shift; }
    if (keyCode == 524576 || keyCode == 524608){  res = option; }
    if (keyCode == 1048840){ res = cmd; }
    return res;
}

hookSpace::ModifierNames hookSpace::THookKeyboard::keyModifierName(int keyCode){
    ModifierNames res = no;
    if (keyCode == 59 || keyCode == 62){ res = ctrl; }
    if (keyCode == 56 || keyCode == 60){ res = shift; }
    if (keyCode == 58 || keyCode == 61){  res = option; }
    if (keyCode == 55){ res = cmd; }
    return res;
}

QString hookSpace::THookKeyboard::searchKey(int keyCode){
    keyCode += 1;
    qDebug() << "searchKey; KeyCode file: " << keyCode;
    QFile KeyFile(PRO_FILE_PWD + QString("/keyCodesOSX.txt"));
    if(!KeyFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "File don't open";
        return "";
    }
    QTextStream in(&KeyFile);
    for (int i = 1; i < keyCode; in.readLine(), i++) { }
    QString res = in.readLine();
    KeyFile.close();
    return res;
}

void hookSpace::getScreenCrutch(THookDevice *hookDev, TEvent* ev){
    hookDev->getScreen(ev);
}


hookSpace::DelayThread::DelayThread(TEvent *event1, QTime timeOfClick1)
{
    event = event1;
    timeOfClick=timeOfClick1;
}

void hookSpace::DelayThread::run(){
    while(dynamic_cast<hookSpace::TMouseEvent*>(event)->getCountClick() != 2 && timeOfClick.msecsTo(QTime::currentTime()) < 500)  { sleep(0); }
    // пол секунды на то, чтобы сделать второй клик, значение практически выдумано
   qDebug() <<" send; countClick: " << dynamic_cast<hookSpace::TMouseEvent*>(event)->getCountClick() << endl;
        //emit getScreen(event);
    //while(!dynamic_cast<hookSpace::TEvent*>(event)->getScreenSave() && timeOfClick.msecsTo(QTime::currentTime()) < 500)  { sleep(0); }
    //dynamic_cast<hookSpace::TEvent*>(event)->setScreenSave(false);
    //qDebug() <<" ScreenSave status: " << dynamic_cast<hookSpace::TMouseEvent*>(event)->getScreenSave() << endl;
    emit send(event);
    //emit changeStatusEvent(false);
    //emit eventReady(event);}
}
void hookSpace::DelayThread::stop()
{
  // emit getScreen(event);
    emit send(event);

}
