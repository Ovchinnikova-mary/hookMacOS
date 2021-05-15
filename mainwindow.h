#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include <QSystemTrayIcon>
#include <QErrorMessage>
#include "Hook.h"
namespace Ui {
class MainWindow;
}
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void onConnectionDone();
    void onDisconnected();
    void onHookPaused();
    void onHookStarted();
    void onHookResumed();
    void onHookStopped();
    void onHookError(hookSpace::hookErrors error);
    void onButtonConnectClicked();
    void onButtonPauseClicked();
    void onButtonRunStopClicked();
signals:
    void connectHook(QString ip, int port);
    void resumeHook();
    void pauseHook();
    void runHook();
    void stopHook();
    void changeFileKeyName(QString name);
    void changeFileMouseName(QString name);
    void changeHotKeyName(QString name);
    void changeFormatName(QString name);
     void changePathName(QString name);
   // void isOnlineModeChanged(bool isOnline);
private slots:
    void on_checkBox_stateChanged(int arg1);
private:
    void connectionChanged(bool isConnected);
    Ui::MainWindow *ui;
    //QSystemTrayIcon *ic;
    QErrorMessage messager;
    hookSpace::THook hook;
    bool hookRun,isPaused,isOnline;
};
#endif // MAINWINDOW_H
