#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include <QErrorMessage>
//#include <QFileDialog>
//#include <QPainter>
//#include <fstream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    qDebug()<<"MaxCount: "<<QString::number(QThreadPool::globalInstance()->maxThreadCount());
    hookRun = false;
    isPaused = false;
    isOnline = false;
    ui->setupUi(this);
    ui->pushButtonPause->setVisible(false);
    //ui->pushButtonRunStop->setVisible(false);
    ui->lineEditIP->setText("192.168.0.25");
    ui->lineEditPort->setInputMask("0000");
    ui->lineEditPort->setText("6666");
    ui->lineEditKey->setText("/dev/input/event4");
    ui->lineEditMouse->setText("/dev/input/event3");
    ui->pushButtonConnect->setVisible(false);
    ui->lineEditKey->setVisible(false);
    ui->lineEditMouse->setVisible(false);
    ui->lineEditPath->setText(PRO_FILE_PWD);

    connect(ui->pushButtonConnect, &QPushButton::clicked, this, &MainWindow::onButtonConnectClicked);
    connect(ui->pushButtonRunStop, &QPushButton::clicked, this, &MainWindow::onButtonRunStopClicked);
    connect(ui->pushButtonPause, &QPushButton::clicked, this, &MainWindow::onButtonPauseClicked);

    connect(this, &MainWindow::connectHook, &hook, &hookSpace::THook::onConnectionRequest);
    connect(this, &MainWindow::runHook, &hook, &hookSpace::THook::runHook);
    connect(this, &MainWindow::pauseHook,&hook,&hookSpace::THook::pauseHook);
    connect(this, &MainWindow::resumeHook,&hook,&hookSpace::THook::resumeHook);
    connect(this, &MainWindow::stopHook, &hook, &hookSpace::THook::stopHook);
    connect(this, &MainWindow::changeFileKeyName, &hook, &hookSpace::THook::onChangeFileKeyName);
    connect(this, &MainWindow::changeFileMouseName, &hook, &hookSpace::THook::onChangeFileMouseName);

    connect(this, &MainWindow::changeHotKeyName, &hook, &hookSpace::THook::onChangeHotKeyName);
    connect(this, &MainWindow::changeFormatName, &hook, &hookSpace::THook::onChangeFormatName);
    connect(this, &MainWindow::changePathName, &hook, &hookSpace::THook::onChangePathName);
   // connect (this,&MainWindow::isOnlineModeChanged, &hook,&hookSpace::THook::onOnlineModeChanged);

    connect(&hook, &hookSpace::THook::connectionDone, this, &MainWindow::onConnectionDone);
    connect(&hook, &hookSpace::THook::socketDisconnected, this, &MainWindow::onDisconnected);
    connect(&hook, &hookSpace::THook::hookStarted, this, &MainWindow::onHookStarted);
    connect(&hook, &hookSpace::THook::hookPaused, this, &MainWindow::onHookPaused);
    connect(&hook, &hookSpace::THook::hookResumed, this, &MainWindow::onHookResumed);
    connect(&hook, &hookSpace::THook::hookStopped, this, &MainWindow::onHookStopped);
    connect(&hook, &hookSpace::THook::error, this, &MainWindow::onHookError);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnectionDone(){
    messager.showMessage("Соединение установлено");

    //emit changeFileKeyName(ui->lineEditKey->text());
    //emit changeFileMouseName(ui->lineEditMouse->text());
    emit changeHotKeyName(ui->comboBoxHotKey->currentText());
    emit changeFormatName(ui->comboBoxFormat->currentText());
    emit changePathName(ui->lineEditPath->text());

    connectionChanged(true);
}

void MainWindow::onDisconnected(){
    qDebug() << "disconnected" << endl;
    messager.showMessage("Соединение разорвано");
    connectionChanged(false);
}

void MainWindow::connectionChanged(bool isConnected){
    //ui->pushButtonRunStop->setVisible(isConnected);
    ui->pushButtonConnect->setVisible(!isConnected);
    ui->lineEditIP->setEnabled(!isConnected);
    ui->lineEditKey->setEnabled(!isConnected);
    ui->lineEditMouse->setEnabled(!isConnected);
    ui->lineEditPort->setEnabled(!isConnected);
    ui->checkBox->setEnabled(!isConnected);
    ui->lineEditPath->setEnabled(!isConnected);
    ui->comboBoxFormat->setEnabled(!isConnected);
    ui->comboBoxHotKey->setEnabled(!isConnected);
    ui->pushButtonRunStop->setVisible(isConnected);
    ui->pushButtonRunStop->setEnabled(isConnected);
    ui->pushButtonRunStop->setEnabled(isConnected);
}

void MainWindow::onHookStarted(){
    hookRun = true;
    ui->pushButtonRunStop->setText("Stop hook");
    ui->pushButtonPause->setVisible(true);
    ui->pushButtonPause->setEnabled(true);
    ui->pushButtonPause->setText("Pause hook");
}

void MainWindow::onHookError(hookSpace::hookErrors error){
    if(error == hookSpace::connectionFailed)
        messager.showMessage("Ошибка подключения к редактору");
    if(error == hookSpace::noConnection)
        messager.showMessage("Нет соединения с редактором");
    if(error == hookSpace::hookIsRunned){
        messager.showMessage("Захватчик уже запущен");
        onHookStarted();
    }
    if(error == hookSpace::hookIsStopped){
        messager.showMessage("Захватчик уже остановлен");
        onHookStopped();
    }
    if(error == hookSpace::errorStart)
        messager.showMessage("Не удалось запустить процесс перехвата");
    if(error == hookSpace::convertError)
        messager.showMessage("Ошибка преобразования события");
}

void MainWindow::onHookPaused(){
    hookRun = false;
    isPaused = true;
    ui->pushButtonPause->setText("Resume hook");
}
void MainWindow::onHookResumed(){
    hookRun = true;
    isPaused = false;
    ui->pushButtonPause->setText("Pause hook");
}

void MainWindow::onHookStopped(){
    qDebug() << "stop" << endl;
    hookRun = false;
    if (!isPaused){
        qDebug() << "update faild" << endl;
        ui->pushButtonRunStop->setText("Run hook");
        ui->pushButtonPause->setEnabled(false);
    }
    else{

        ui->pushButtonPause->setText("Resume hook");
    }

}

void MainWindow::onButtonPauseClicked(){
    if(hookRun){
        isPaused=true;
        emit pauseHook();
    }
    else
        emit resumeHook();
}

void MainWindow::onButtonConnectClicked(){
    emit connectHook(ui->lineEditIP->text(), ui->lineEditPort->text().toInt());
}

void MainWindow::onButtonRunStopClicked(){
   if (!isOnline){
        //emit changeFileKeyName(ui->lineEditKey->text());
        //emit changeFileMouseName(ui->lineEditMouse->text());
        emit changeHotKeyName(ui->comboBoxHotKey->currentText());
        emit changeFormatName(ui->comboBoxFormat->currentText());
        emit changePathName(ui->lineEditPath->text());

        connectionChanged(true);
    }
    if (!hookRun){
        emit runHook();
    }
    else
    emit stopHook();
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    if (ui->checkBox->isChecked()){
        isOnline = true;
        hook.isOnline=true;
        ui->lineEditIP->setEnabled(true);
        ui->lineEditPort->setEnabled(true);
        ui->pushButtonConnect->setEnabled(true);
        ui->pushButtonConnect->setVisible(true);
        ui->pushButtonRunStop->setVisible(false);
        ui->pushButtonRunStop->setEnabled(false);
        ui->comboBoxFormat->setCurrentText("png");
        ui->lineEditPath->setEnabled(false);
        ui->comboBoxFormat->setEnabled(false);
        ui->comboBoxHotKey->setEnabled(false);

    }
    else{
        isOnline=false;
        hook.isOnline=false;
        ui->lineEditIP->setEnabled(false);
        ui->lineEditPort->setEnabled(false);
        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonConnect->setVisible(false);
        ui->pushButtonRunStop->setVisible(true);
        ui->pushButtonRunStop->setEnabled(true);
        ui->lineEditPath->setEnabled(true);
        ui->comboBoxFormat->setEnabled(true);
        ui->comboBoxHotKey->setEnabled(true);
    }
       // emit isOnlineModeChanged(isOnline);
}
