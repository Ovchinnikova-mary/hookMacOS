#include <QtDebug>
#include "DeviceHook.h"
#include <QtConcurrent/QtConcurrent>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#ifndef QT_MAC_USE_COCOA
#import <Cocoa/Cocoa.h>
id global_monitor_mouse = nil;
id global_monitor_keyboard = nil;

hookSpace::THookDevice::~THookDevice(){
   if (global_monitor_mouse){
        [NSEvent removeMonitor:global_monitor_mouse];
        global_monitor_mouse= nil;
    }
    else if(global_monitor_keyboard){
        [NSEvent removeMonitor:global_monitor_keyboard];
        global_monitor_keyboard= nil;
    }

}

void hookSpace::THookDevice::onHookRun(){
    if(isRun){
        emit hookDeviceRunned();
        return;
    }

    bool test = AXIsProcessTrusted();
    qDebug() << "Process trusted: "<<test << endl;
    if(test){
        isRun = true;
        QtConcurrent::run(this, &hookSpace::THookDevice::run);
        emit hookDeviceRunned();
    }
    else
        emit hookDeviceError();
}

void hookSpace::THookDevice::onHookStop(){
    if(isRun){
        isRun = false;
        emit hookDeviceStopped();
        if (global_monitor_mouse){
            [NSEvent removeMonitor:global_monitor_mouse];
            global_monitor_mouse= nil;
        }
        else if(global_monitor_keyboard){
            [NSEvent removeMonitor:global_monitor_keyboard];
            global_monitor_keyboard= nil;
        }
    }
}

void hookSpace::THookDevice::sendEvent(){

    emit changeStatusEvent(false);
    emit setModifiers(event);
    emit eventReady(event);
    event = 0;
}

void hookSpace::THookMouse::getEvent(){
    const int mask_mouse =
            NSEventMaskLeftMouseDown | NSEventMaskLeftMouseUp |
            NSEventMaskRightMouseDown | NSEventMaskRightMouseUp |
            NSEventMaskOtherMouseDown | NSEventMaskOtherMouseUp |
            NSEventMaskLeftMouseDragged | NSEventMaskRightMouseDragged |
            NSEventMaskOtherMouseDragged | NSEventMaskScrollWheel;
    global_monitor_mouse = [NSEvent addGlobalMonitorForEventsMatchingMask:mask_mouse
               handler:^(NSEvent* event_data) {
        signalProcessing(event_data);
    }];
}

void hookSpace::THookMouse::signalProcessing(void *event_data){

    NSEvent *e = reinterpret_cast<NSEvent *>(event_data);
    Q_ASSERT(e);
    if (mouseDown) {
        CGPoint currentPos = [e locationInWindow];
        //qDebug() << "mouse dragged" << endl;
        if (sqrt(pow(currentPos.x - lastCursorPos.x, 2) + pow(currentPos.y - lastCursorPos.y, 2)) > 24){
            TMouseEvent *currentEvent = createNewDragEvent(mouseDragTrack);
            if (!dragRun) {
                qDebug() << "Start dragged" << endl;
                dragRun = true;
                dynamic_cast<hookSpace::TMouseEvent*>(event)->setAsDragStart();
                sendEvent();
            }
            qDebug() << "mouse dragged" << endl;
            currentEvent->setPoint(currentPos);
            event = currentEvent;
            sendEvent();
            lastCursorPos = currentPos;
        }
    }

    if (!mouseDown && ([e type]==NSEventTypeLeftMouseDown || [e type]==NSEventTypeRightMouseDown)){
        // Нажатие кнопки мыши
        TypesOfMouseButton type = ([e type]==NSEventTypeLeftMouseDown) ? Left : Right;
        qDebug() << "Start click mouse" << endl;

        if (!event || !timerRun || [e type]==NSEventTypeRightMouseDown){
            event = createNewClickEvent(type);
        }

        CGPoint currentPos = [e locationInWindow];
        if (timerRun && dynamic_cast<hookSpace::TMouseEvent*>(event)->getCountClick()){
            if (abs(currentPos.x - lastCursorPos.x) > 10 || abs(currentPos.y - lastCursorPos.y) > 10) {
                // если условие НЕ выполняется, то это двойной клик ЛКМ,
                qDebug() << "don't double click" << endl;
                timerRun = false;
                event = createNewClickEvent(type);
            }
        }
        lastCursorPos = currentPos;
        mouseDown = true;
        dynamic_cast<hookSpace::TMouseEvent*>(event)->addClick();
        if (dynamic_cast<hookSpace::TMouseEvent*>(event)->getCountClick() == 1) {
            dynamic_cast<hookSpace::TMouseEvent*>(event)->setPoint(currentPos);
            if (type == Left) { timeOfClick = QTime::currentTime(); }
        }
        if (dynamic_cast<hookSpace::TMouseEvent*>(event)->getCountClick() == 2)
            event = 0;
    }

    if (!mouseDown && ([e type]==NSEventTypeScrollWheel)){
        // Движение колёсика
        qDebug() << "Scroll wheel mouse" << endl;
        if ([e deltaY]>0){
            qDebug() << "Scroll wheel mouse up" << endl;
            event = createNewScrollEvent(Upward);
        }
        else {
            qDebug() << "Scroll wheel mouse down" << endl;
            event = createNewScrollEvent(Downward);
        }
        sendEvent();
    }

    if (mouseDown && ([e type]==NSEventTypeLeftMouseUp || [e type]==NSEventTypeRightMouseUp)){
        // Отпускание кнопки мыши
        TMouseEvent *currentEvent = dynamic_cast<hookSpace::TMouseEvent*>(event);
        mouseDown = false;
        if(event && currentEvent->getTypeEvent() == mouseClick && currentEvent->getTypeButton() == Left && currentEvent->getCountClick() == 1){
            timerRun = true;
            hookSpace::DelayThread *delayThread = new hookSpace::DelayThread(event,timeOfClick);
            connect(delayThread,SIGNAL(send(TEvent*)),this,SLOT(runTimer(TEvent*)));
            delayThread->start();
        }

        if(event && currentEvent->getTypeEvent() == mouseClick && currentEvent->getTypeButton() == Right){
            qDebug() << "end click mouse right: "<< dynamic_cast<hookSpace::TEvent*>(event)->getScreenSave() << endl;
            sendEvent();
        }
        if(!event && dragRun){
            qDebug() << "end dragged"<<endl;
            event = createNewDragEvent(mouseDragStop);
            dynamic_cast<hookSpace::TMouseEvent*>(event)->setPoint([e locationInWindow]);
            dragRun = false;
            sendEvent();
        }
        if(dragRun){
            dragRun = false;
            emit hookDeviceError();
        }
    }
}


void hookSpace::THookKeyboard::getEvent(){
    const int mask_keyboard =
                NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged;
    global_monitor_keyboard = [NSEvent addGlobalMonitorForEventsMatchingMask:mask_keyboard
            handler:^(NSEvent* event_data) {
        signalProcessing(event_data);
    }];
}

void hookSpace::THookKeyboard::signalProcessing(void *event_data){
// обрабатывает событие клавиатуры
    NSEvent *e = reinterpret_cast<NSEvent *>(event_data);
    Q_ASSERT(e);
    qDebug() << "start hook keyboard: hot key: " << hotKey << endl;
    qDebug() << "modifier: " << [e modifierFlags] << endl;
    ModifierNames modifierKey = modifierName([e modifierFlags]); // получили модификатор

    //проверяем горячую клавишу
    if([e type] == NSEventTypeKeyDown || [e type] == NSEventTypeFlagsChanged){
        QString currentKey = searchKey([e keyCode]);
        qDebug() << "current key: " << currentKey << endl;
        if(currentKey == hotKey){
            HotKey = true;
            if(isRun){
                emit pressHotKey();
            }
        }
    }

    if(!HotKey){
        //отпускаем клавишу
        if([e type] == NSEventTypeKeyUp){   
            qDebug() << "modifier key up: " << QString::number(modifierKey) << endl;
            if(modifierKey != no){//существует модификатор
                if (eventIsNew && event){
                    //qDebug() << "end key press; modifier: " << modifierKey << " event: " << event << endl;
                    dynamic_cast<hookSpace::TKeyPressEvent*>(event)->setKey(searchKey([e keyCode]));
                    sendEvent();
                    emit removeModifier(modifierKey);
                }
                else{
                    emit removeModifier(modifierKey);
                }
            }
        }

        if([e type] == NSEventTypeKeyDown){   
            if(keyDownModifier){//узнаем был ли нажат модификатор
                //qDebug() << "delete with start combo key+modifier; event: " << event << endl;
                event = 0;
                keyDownModifier = false;
            }
            if (!event){
                qDebug() << "create new key press and modifier:  " << QString::number(modifierKey) << endl;
                event = createNewKeyPressEvent();
            }
            if (modifierKey != no){//если модификатор не равен no то добавляем
                qDebug() << "add modifier" << endl;
                emit addModifier(modifierKey);}
            else{//если нет модификатора находим и отправляем
                dynamic_cast<hookSpace::TKeyPressEvent*>(event)->setKey(searchKey([e keyCode]));
                sendEvent();
            }
        }

        if([e type] == NSEventTypeFlagsChanged){
            ModifierNames key = keyModifierName([e keyCode]);

            if(event && keyDownModifier && (modifierKey != no)){
                if(keyDownModifier){
                    //qDebug() << "delete with start combo modifiers; event: " << event << endl;
                    event = 0;
                    keyDownModifier = false;
                }
            //qDebug() << "create new combo key press with modifier; event: " << event << endl;
            event = createNewKeyPressEvent();
            emit addModifier(modifierKey);
            }

            if(keyDownModifier && event){
                qDebug() << "end key press modifier; event:" << event << endl;
                keyDownModifier = false;
                dynamic_cast<hookSpace::TKeyPressEvent*>(event)->setKey(searchKey([e keyCode]));
                sendEvent();
            }
            else if(modifierKey != no){
                if (eventIsNew && event){
                    qDebug() << "end combo key press with modifier; event: " << event << endl;
                    dynamic_cast<hookSpace::TKeyPressEvent*>(event)->setKey(searchKey([e keyCode]));
                    emit addModifier(modifierKey);
                    sendEvent();
                }
                else{
                    emit removeModifier(modifierKey);
                }
            }
            //если нажимаем модификатор
            if ((modifierKey == key) && !event && !keyDownModifier){
                //qDebug() << "create new key press modifier; event: " << event << endl;
                event = createNewKeyPressEvent();
                keyDownModifier = true;           
            }

        }
    }
}

#endif
