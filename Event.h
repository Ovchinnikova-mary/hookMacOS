#ifndef TEVENT_H
#define TEVENT_H

#include <QList>
#include <QPoint>
#include <QXmlStreamWriter>
#include <ApplicationServices/ApplicationServices.h>
#include <QtMac>


namespace hookSpace {
    class TModifier;
    class TArea;
    class TEvent;
    class TKeyPressEvent;
    class TMouseEvent;
    class TEventsConvertor;

    enum ModifierNames{ctrl, shift, option, cmd, no};
    enum TypesOfEvents {keyPress, mouseClick, mouseDragStart, mouseDragTrack, mouseDragStop, mouseWheel};
    enum TypesOfMouseButton{Left, Right, Wheel};
    enum Ways{Upward, Downward}; //wheel

    void writeLog(QString);
    void writeFile(QString*);
}

Q_DECLARE_METATYPE(hookSpace::ModifierNames)

class hookSpace::TModifier : public QObject{
Q_OBJECT
public:
    TModifier();
    TModifier(const TModifier &m);

    QString toQString();

public slots:
   void clearAll();
   void removeModifier(ModifierNames m);
   void addModifier(ModifierNames m);
   void setModifiers(TEvent* ev);

private:
    int ctrlM, shiftM, optionM, cmdM;
};

class hookSpace::TArea {
public:
    TArea(int x1, int y1, int width1, int height1);

    int x, y, width, height;
};

class hookSpace::TEvent{
public:
    TEvent();
    virtual ~TEvent();
    void setScreenSave(bool  screenSave1);
    bool getScreenSave();
    QString* convert();
    void setImageName(QString* imName);
    bool haveImageName();
    void setModifiers(TModifier* m);
    QJsonObject *convertToJSON();
    void addImageName(QString *imName);
public slots:

protected:
    QList<QString*> *imageNames;
    int id;
    bool loadImage();
    virtual void convertSpecific() = 0;
    virtual void convertSpecificToJson(QJsonObject *&)=0;
    hookSpace::TModifier* modifiers;
    QString type;   // keyPress / mouseWheel / mouseClick / mouseDragStart / mouseDragTrack / mouseDragStop
    QString *imageName;
    QByteArray *image;
    QXmlStreamWriter *xmlWriter;
    bool screenSave;
};

class hookSpace::TKeyPressEvent : public hookSpace::TEvent{
public:
    TKeyPressEvent();
    TKeyPressEvent(QString k);

    void setKey(QString k);

private:
    void convertSpecific();

    QString key;
   void convertSpecificToJson(QJsonObject *&);
};

class hookSpace::TMouseEvent : public hookSpace::TEvent{
public:
    TMouseEvent(TypesOfEvents typeOfDrag);
    TMouseEvent(TypesOfMouseButton t);
    TMouseEvent(Ways w);

    ~TMouseEvent();

    void setAsDragStart();
    void addClick();
    void setPoint(CGPoint p);
    void setCountClick(int clickCount1);
    int getCountClick();
    hookSpace::TypesOfMouseButton getTypeButton();
    hookSpace::TypesOfEvents getTypeEvent();

    void convertToJSON();
private:
    void convertSpecific();
void convertSpecificToJson(QJsonObject *&);
    hookSpace::TypesOfEvents typeEvent;
    hookSpace::TypesOfMouseButton typeButton;
    QString mouseButton;
    CGPoint point;
    QString scroll;    // Upward / Downward
    hookSpace::TArea *window;
    int countClick;
};

class hookSpace::TEventsConvertor : public QObject{
Q_OBJECT
public slots:
    void onConvertEvent(TEvent* event);
    void onConvertEventToJson(TEvent* event);

signals:
    void convertReady(QString* xmlEvent);
    void convertToJsonReady(QJsonObject *jsonEvent);
    void convertError();
};
#endif // TEVENT_H

