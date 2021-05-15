#include "jsonclient.h"

#include <QApplication>
#include <QScreen>

hookSpace::JSONClient::JSONClient()
{
    countMessage = 0;
    frameArray = new QJsonArray();
}

void hookSpace::JSONClient::onStarted(){
    jsonObjects=nullptr;

}

void hookSpace::JSONClient:: onStopped(){
   /* Display *display;
    Screen *screen;
    display = XOpenDisplay(NULL);
    screen = DefaultScreenOfDisplay(display);*/
    qDebug() << "start save in json file" << endl;
    QRect rec = QApplication::screens().at(0)->geometry();

    QScreen *screen = QApplication::screens().at(0);
    QSize size = screen->availableSize();
    QJsonObject objectName;
    objectName.insert("pictureHeight",rec.height());
    objectName.insert("pictureWidth",    rec.width());
    objectName.insert("frames",*frameArray);
    QJsonDocument document(objectName);
    QFile jsonFile(PRO_FILE_PWD + QString("/Script.json"));
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(document.toJson());
}
void hookSpace::JSONClient::writeScrollEvent(QList<QJsonObject *> *jsonObjects){

    int actionTypeId =(jsonObjects->last()->operator []("actionSwitch")).toObject().operator []("actionId").toInt();
    qDebug() << "id action: " << actionTypeId << endl;
    if (actionTypeId==14|| actionTypeId==15){
        qDebug() << "write scroll event" << endl;
        QJsonObject actionSwitch;
        QJsonObject scrollEvent;
        actionSwitch.insert("actionId",actionTypeId);
        scrollEvent.insert("frameNumber",countMessage);
        scrollEvent.insert("pictureLink",jsonObjects->first()->operator []("pictureLink"));
        actionSwitch.insert("ticksCount",jsonObjects->size());
        jsonObjects->pop_front();
        if (!jsonObjects->isEmpty()){
            QJsonArray scrollPictures;
            for (int i=0; i<jsonObjects->length(); i++){
                QJsonObject scrollTick;
                scrollTick.insert("pictureNumber",i+1);
                scrollTick.insert("pictureLink",jsonObjects->at(i)->operator []("pictureLink"));
                scrollPictures.append(scrollTick);
            }
            actionSwitch.insert("switchPictures",scrollPictures);
        }
        scrollEvent.insert("actionSwitch",actionSwitch);

        frameArray->append(scrollEvent);
        qDebug() << "countMessage: " << countMessage << " framwArray count: " << frameArray->size() << endl;
        return;
    }

    if (actionTypeId==13){
        qDebug() << "write dragged event" << endl;
        QJsonObject dragEvent;
        QJsonObject actionSwitch;
        actionSwitch.insert("actionId",actionTypeId);
        dragEvent.insert("frameNumber",countMessage);
        actionSwitch.insert("startXLeft",(jsonObjects->first()->operator []("actionSwitch")).toObject()["x"].toInt());
                actionSwitch.insert("startYLeft",(jsonObjects->first()->operator []("actionSwitch")).toObject()["y"].toInt());
                actionSwitch.insert("startXRight",(jsonObjects->first()->operator []("actionSwitch")).toObject()["x"].toInt()+50);
                actionSwitch.insert("startYRight",(jsonObjects->first()->operator []("actionSwitch")).toObject()["y"].toInt()+50);
                actionSwitch.insert("finishXLeft",(jsonObjects->last()->operator []("actionSwitch")).toObject()["x"].toInt());
                actionSwitch.insert("finishYLeft",(jsonObjects->last()->operator []("actionSwitch")).toObject()["y"].toInt());
                actionSwitch.insert("finishXRight",(jsonObjects->last()->operator []("actionSwitch")).toObject()["x"].toInt()+50);
                actionSwitch.insert("finishYRight",(jsonObjects->last()->operator []("actionSwitch")).toObject()["y"].toInt()+50);
                dragEvent.insert("pictureLink",jsonObjects->first()->operator []("pictureLink"));
                jsonObjects->pop_front();

        if (!jsonObjects->isEmpty()){
            QJsonArray dragPictures;
            for (int i=0; i<jsonObjects->length(); i++){
                QJsonObject dragPicture;
                dragPicture.insert("pictureNumber",i+1);
                dragPicture.insert("pictureLink",jsonObjects->at(i)->operator []("pictureLink"));

                dragPicture.insert("x",(jsonObjects->at(i)->operator []("actionSwitch")).toObject()["x"].toInt());
                dragPicture.insert("y",(jsonObjects->at(i)->operator []("actionSwitch")).toObject()["y"].toInt());
                                                dragPictures.append(dragPicture);
            }
            actionSwitch.insert("switchPictures",dragPictures);
        }
        dragEvent.insert("actionSwitch",actionSwitch);
        frameArray->append(dragEvent);
        qDebug() << "countMessage: " << countMessage << " framwArray count: " << frameArray->size() << endl;
        return;
    }
}

void hookSpace::JSONClient::onWrite(QJsonObject* jsonObject){
    qDebug() << "JSON object: " << jsonObject;
    qDebug() << "start write JSON in file: " << (jsonObject->operator []("actionSwitch")).toObject()["actionId"].toInt() << endl;

    if ((jsonObject->operator []("actionSwitch")).toObject()["actionId"].toInt()==15 || (jsonObject->operator []("actionSwitch")).toObject()["actionId"].toInt()==14 || (jsonObject->operator []("actionSwitch")).toObject()["actionId"].toInt()==13){
        if (!jsonObjects)

            jsonObjects=new QList<QJsonObject *>();   
        if (jsonObjects->isEmpty() || (jsonObjects->last()->operator []("actionSwitch")).toObject().operator []("actionId").toInt()==(jsonObject->operator []("actionSwitch")).toObject()["actionId"].toInt()){
            jsonObjects->append(jsonObject);
            return;
       }
       else{
            countMessage++;
            writeScrollEvent(jsonObjects);
            jsonObjects->clear();
            jsonObjects = new QList<QJsonObject *>();
            jsonObjects->append(jsonObject);
            return;
        }
    }
    else {
        
        if (jsonObjects)
            if (!jsonObjects->isEmpty()){
                countMessage++;
                writeScrollEvent(jsonObjects);
                jsonObjects->clear();
            }
    }



    // QJsonArray frameArray;
    countMessage++;
    jsonObject->insert("frameNumber",countMessage);
    frameArray->append(*jsonObject);
    qDebug() << "countMessage: " << countMessage << " framwArray count: " << frameArray->size() << endl;
    /* if (frameArray->at(countMessage-1)[""]){
            jsonObject["ticksCount"] = frameArray->at(countMessage-1)["ticksCount"].toInt()+1;*/

    //}
}
