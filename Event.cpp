#include <QTime>
#include <QFile>
#include <QTextStream>
#include "Event.h"
#include <stdlib.h>
#include <unistd.h>
#include <QJsonObject>
#include <QtGui>
#include <QFileInfo>

hookSpace::TEvent::TEvent(){
    imageNames =new QList<QString*>(), xmlWriter = 0;
    modifiers = 0, image = 0;
    imageName=0;
    screenSave = false;
}

hookSpace::TEvent::~TEvent(){
    delete imageNames;
    delete xmlWriter;
    delete modifiers;
    delete image;
}

void hookSpace::TEvent::setScreenSave(bool screenSave1){
    screenSave = screenSave1;
}

bool hookSpace::TEvent::getScreenSave(){
    return screenSave;
}

void hookSpace::TEvent::addImageName(QString* imName){
    qDebug() << "start add image name: " << imName << endl;
    if (!imageName){
        imageName=imName;
    }
    imageNames->append(imName);
    qDebug() << "add image count: " << imageNames->count() << endl;
    screenSave = true;
    //dynamic_cast<hookSpace::THookDevice*>(dev)->setScreenSave(true);
}

void hookSpace::TEvent::setModifiers(TModifier* m){
    modifiers = m;
}

bool hookSpace::TEvent::haveImageName(){
    return imageName != 0;
}

bool hookSpace::TEvent::loadImage(){
    while(!imageName) {sleep(1);}
    QFile im(*imageName);
    while(!im.exists()) {sleep(1);}
    if(!im.open(QIODevice::ReadOnly))
        return 0;
    image = new QByteArray(im.readAll());
    // im.remove();
    im.close();

    return 1;
}

QString* hookSpace::TEvent::convert(){
    qDebug() << "start convert: modifiers " << modifiers->toQString() << endl;
    QTime time = QTime::currentTime();
    long long msTime = (((long long)(time.hour()) * 60 + time.minute()) * 60 + time.second()) * 1000 + time.msec();

    while (!modifiers) {sleep(1); }
    QString mod = modifiers->toQString();

    QString *eventXml = new QString();
    xmlWriter = new QXmlStreamWriter(eventXml);

    if(!loadImage())
        return 0;

    xmlWriter->writeStartElement("content.snapshot");
    xmlWriter->writeAttribute("type", type);
    qDebug() << "time: " << QString::number(msTime) << endl;
    xmlWriter->writeAttribute("time", QString::number(msTime));

    xmlWriter->writeStartElement("picture");
   // xmlWriter->writeAttribute("size", QString::number(imageNames->first()->length()));
    qDebug() << "size" << image->size() << endl;
    xmlWriter->writeAttribute("size", QString::number(image->size()));
    //xmlWriter->writeAttribute("size", QString::number(1024*768));
    xmlWriter->writeCharacters(image->toBase64());
    xmlWriter->writeEndElement();

    if(!mod.isEmpty()){
        xmlWriter->writeStartElement("mod");
        xmlWriter->writeCharacters(mod);
        xmlWriter->writeEndElement();
    }

    this->convertSpecific();

    xmlWriter->writeEndElement();   // content.snapshot

    if(xmlWriter->hasError())
        return 0;
    qDebug() << "Element XML: " << eventXml;
    hookSpace::writeFile(eventXml);
    return eventXml;
}

QJsonObject* hookSpace::TEvent::convertToJSON(){
    qDebug() << "start convert json" << endl;
    QJsonObject *eventJson = new QJsonObject();
    qDebug() << "count screen: " << imageNames->count();
    qDebug() << "screen: " << imageNames->first()->split("/").at(4) + "/" + imageNames->first()->split("/").at(5) << endl;

    eventJson->insert("pictureLink",imageNames->first()->split("/").at(4) + "/" + imageNames->first()->split("/").at(5));
    if (imageNames->count()>1){
        eventJson->insert("pictureReleaseLink",imageNames->last()->split("/").at(4) + "/" + imageNames->last()->split("/").at(5));
    }
    qDebug() << "cont start convert json: "<< eventJson << endl;
    this->convertSpecificToJson(eventJson);

    qDebug() << "Element JSON: " << eventJson;
    return eventJson;
}

hookSpace::TKeyPressEvent::TKeyPressEvent(){
    type = "keyPress";
}

hookSpace::TKeyPressEvent::TKeyPressEvent(QString k){
    type = "keyPress";
    key = k;
}

void hookSpace::TKeyPressEvent::setKey(QString k){
    key = k;
}

void hookSpace::TKeyPressEvent::convertSpecific(){
    qDebug() << "convertSpecific" << endl;
    if (!xmlWriter)
        return;

    xmlWriter->writeStartElement("event");
    xmlWriter->writeAttribute("key", key);
    xmlWriter->writeEndElement();
}
void hookSpace::TKeyPressEvent::convertSpecificToJson(QJsonObject *&switchData){
    qDebug() << "convertSpecific JSON: " << modifiers->toQString() << endl;

    QJsonObject actionSwitch;
    if (type=="keyPress"){
        actionSwitch.insert("key",key);
        if (!modifiers->toQString().isEmpty()){
           actionSwitch.insert("actionId",12);
            actionSwitch.insert("modKey",modifiers->toQString());        }
        else{
            actionSwitch.insert("actionId",9);
        }
        switchData->insert("actionSwitch",actionSwitch);
    }
}

hookSpace::TMouseEvent::TMouseEvent(TypesOfEvents typeOfDrag){
    typeEvent = typeOfDrag;
    countClick = 0;
    window = 0;
    typeButton = Left;

    switch (typeOfDrag) {
    case mouseDragStart:
        type = "mouseDragStart";
        break;
    case mouseDragTrack:
        type = "mouseDragTrack";
        break;
    case mouseDragStop:
        type = "mouseDragStop";
        break;
    default:
        break;
    }
}

hookSpace::TMouseEvent::TMouseEvent(TypesOfMouseButton t){
    window = new TArea(point.x, point.y, 75, 75);
    type = "mouseClick";
    countClick = 0;
    typeEvent = mouseClick;
    typeButton = t;
    mouseButton = (t == Left) ? "Left" : "Right";
}

hookSpace::TMouseEvent::TMouseEvent(Ways w){
    type = "mouseWheel";
    typeButton = Wheel;
    typeEvent = mouseWheel;
    if (w == Upward)
        scroll = "Upward";
    if (w == Downward)
        scroll = "Downward";
    window = 0;
    countClick = 0;
}

hookSpace::TMouseEvent::~TMouseEvent(){
    delete window;
}

void hookSpace::TMouseEvent::addClick(){
    if(countClick < 2 && typeEvent == mouseClick)
        countClick++;
}

int hookSpace::TMouseEvent::getCountClick(){
    return countClick;
}

hookSpace::TypesOfMouseButton hookSpace::TMouseEvent::getTypeButton(){
    return typeButton;
}

hookSpace::TypesOfEvents hookSpace::TMouseEvent::getTypeEvent(){
    return typeEvent;
}

void hookSpace::TMouseEvent::setAsDragStart(){
    typeEvent = mouseDragStart;
    type = "mouseDragStart";
}

void hookSpace::TMouseEvent::setPoint(CGPoint p){
    point = p;
}
void hookSpace::TMouseEvent::setCountClick(int clickCount1){
    countClick = clickCount1;
}
void hookSpace::TMouseEvent::convertSpecific(){
    qDebug() <<"Specific XML" << endl;
    if (!xmlWriter)
        return;

    if(typeEvent == mouseClick || typeEvent == mouseDragStart || typeEvent == mouseWheel)
        xmlWriter->writeStartElement("event");
    if(typeEvent == mouseClick || typeEvent == mouseDragStart)
        xmlWriter->writeAttribute("button", mouseButton);
    if(typeEvent == mouseClick)
        xmlWriter->writeAttribute("count", QString::number(countClick));
    if(typeEvent == mouseWheel)
        xmlWriter->writeAttribute("way", scroll);
    if(typeEvent != mouseWheel){
        xmlWriter->writeStartElement("point");
        xmlWriter->writeAttribute("x", QString::number((int)point.x));
        xmlWriter->writeAttribute("y", QString::number((int)point.y));
        xmlWriter->writeEndElement();
    }
    if(typeEvent == mouseClick){
        xmlWriter->writeStartElement("window");
        qDebug() << "x: " << QString::number((int)point.x) << endl;
        qDebug() << "y: " << QString::number((int)point.y) << endl;
        xmlWriter->writeAttribute("x", QString::number((int)point.x));
        xmlWriter->writeAttribute("y", QString::number((int)point.y));
        xmlWriter->writeAttribute("width", QString::number(75));
        xmlWriter->writeAttribute("height", QString::number(75));
        xmlWriter->writeEndElement();
    }
    if(typeEvent == mouseClick || typeEvent == mouseDragStart || typeEvent == mouseWheel)
        xmlWriter->writeEndElement(); // event
}

void hookSpace::TMouseEvent::convertSpecificToJson(QJsonObject *&switchData){
    //  QJsonObject switchData;
    QJsonObject actionSwitch;
    if (typeEvent == mouseClick){
        if (countClick==2)
            (mouseButton=="Left") ?actionSwitch.insert("actionId",4):  actionSwitch.insert("actionId",8);
        if (countClick == 1)
            (mouseButton=="Left") ?actionSwitch.insert("actionId",1):  actionSwitch.insert("actionId",5);
        actionSwitch.insert("xLeft", point.x);
        actionSwitch.insert("yLeft", point.y);
        actionSwitch.insert("xRight", point.x+50);
        actionSwitch.insert("yRight", point.y+50);
      //  switchData->insert("actionSwitch",actionSwitch);

    }
    if (typeEvent == mouseDragStart || typeEvent == mouseDragStop || typeEvent == mouseDragTrack){
        actionSwitch.insert("actionId",13);
        actionSwitch.insert("x", point.x);
        actionSwitch.insert("y", point.y);
//        switchData->insert("actionSwitch",actionSwitch);
    }
    if (typeEvent == mouseWheel){
        (scroll=="Downward") ? actionSwitch.insert("actionId",15) : actionSwitch.insert("actionId",14);
    }
            switchData->insert("actionSwitch",actionSwitch);
}



void hookSpace::TEventsConvertor::onConvertEvent(TEvent* event){
    qDebug() << "Convert Event to XML" << endl;
    QString *res = event->convert();

    if (res){
        emit convertReady(res);
     //   writeLog(*res);
    }
    else
        emit convertError();

    // delete event;
}

void hookSpace::TEventsConvertor::onConvertEventToJson(TEvent* event){
    qDebug() << "Convert Event to JSON" << endl;
    QJsonObject *eventObject= event->convertToJSON();
    if (eventObject)
        emit convertToJsonReady(eventObject);

    else
        emit convertError();
    //writeLog(*res);*/
    // delete event;
}

hookSpace::TModifier::TModifier(){    
    qRegisterMetaType<hookSpace::ModifierNames>("ModifierNames");
    optionM = 0, ctrlM = 0, shiftM = 0, cmdM = 0;
}

hookSpace::TModifier::TModifier(const TModifier &m)
    :QObject::QObject()
{
    optionM = m.optionM, ctrlM = m.ctrlM, shiftM = m.shiftM, cmdM = m.cmdM;
}

void hookSpace::TModifier::addModifier(ModifierNames m){
    if (m == option) optionM = 1;
    if (m == ctrl) ctrlM = 1;
    if (m == shift) shiftM = 1;
    if (m == cmd) cmdM = 1;
}

void hookSpace::TModifier::removeModifier(ModifierNames m){
    if (m == option) optionM = 0;
    if (m == ctrl) ctrlM = 0;
    if (m == shift) shiftM = 0;
    if (m == cmd) cmdM = 0;
}

void hookSpace::TModifier::setModifiers(TEvent* ev){
    ev->setModifiers(new TModifier(*this));
}

QString hookSpace::TModifier::toQString(){
    QString res;

    if (optionM)
        res += "Option";
    if (ctrlM){
        if(res.length()) res += ", ";
        res += "Control";
    }
    if (shiftM){
        if(res.length()) res += ", ";
        res += "Shift";
    }
    if (cmdM){
        if(res.length()) res += ", ";
        res += "Command";
    }

    return res;
}

void hookSpace::TModifier::clearAll(){
    optionM = 0, ctrlM = 0, shiftM = 0, cmdM = 0;
}

hookSpace::TArea::TArea(int x1, int y1, int width1, int height1){
    x = x1, y = y1, width = width1, height = height1;
}

void hookSpace::writeFile(QString *s){
    QFile out(PRO_FILE_PWD + QString("script.txt"));
    qDebug() << "start write in file script" << endl;
    qDebug() << s;
    if (!out.open(QIODevice::Append))
    {
        qDebug() << "File don't open";
        return;
    }
    QFileInfo fileinfo("script.txt");
    QString absPath = fileinfo.absoluteFilePath();
    qDebug() << absPath;
    QTextStream stream(&out);
    stream << s + '\n';
    out.close();
}


void hookSpace::writeLog(QString s){
    QFile log("log.txt");
    if(!log.open(QIODevice::Append))
        return;
    QTextStream stream(&log);
    stream << s + '\n';
    log.close();
}
