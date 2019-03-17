#include "PlayScene.h"
#include "ui_PlayScene.h"
#include <QThread>
#include "../Logic/MainLogic.h"
#include "../Data/DataDef.h"
#include <iostream>
#include <QDebug>
#include <QPropertyAnimation>
#include <QGraphicsEffect>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QBrush>
#include <QIcon>
#include <QtAlgorithms>
#include <cstdlib>
#include "StartScene.h"

int PlayScene::commandsCount = 0;

PlayScene::PlayScene(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PlayScene)
{
    ui->setupUi(this);
    this->setWindowTitle("FC16UI-PLAY");
    this->setGeometry(QRectF(QPoint(150,90),QSize(900, 900)).toRect());

    qDebug()<<"In PlayScene():, this: "<<this;

    statusWindow = new QMainWindow(this);
    statusWindow->setWindowTitle("FC16UI-STATUS");
    statusWindow->setGeometry(QRect(QPoint(1100, 90), QSize(700,1000)));

    UI::MainLogic::GetInstance()->logFileStream<<"Main thread is "<<QThread::currentThread()<<std::endl;

    UI::MainLogic::GetInstance()->playScene = this;

    //Map setting
    mapSize = QSizeF(50, 50);
    pixelSize = QSizeF(18,18);

    originPoint = QPointF(0,0);
    wheelScaleRate = 0.0005;
    translateScaleRate = 1;
    autoView = true;
    singleMode = true;
    //Game setting
    exit_thread_flag = true;
    goToLoopBegin_flag = false;

    QImage backImg;
    if(backImg.load(":/FC16UIResource/newNoTower.png"))
    {
        mapBackGround = new QLabel(this);
        mapBackGround->setGeometry(QRectF(originPoint,
                                          QSizeF(mapSize.width()*pixelSize.width(), mapSize.height()*pixelSize.height())).toRect());
        mapBackGround->setScaledContents(true);
        mapBackGround->setPixmap(QPixmap::fromImage(backImg));
    }

    qDebug()<<"Loaded backImg";


    goBackButton = new QPushButton("BACK", statusWindow);
    goBackButton->setGeometry(QRectF(QPointF(0, 0), QSizeF(80, 30)).toRect());
    connect(goBackButton, SIGNAL(clicked(bool)),this,
            SLOT(goBackButtonClicked()));

    startGameButton = new QPushButton("START", statusWindow);
    startGameButton->setGeometry(QRectF(QPointF(100, 0), QSizeF(80, 30)).toRect());
    connect(startGameButton, SIGNAL(clicked(bool)),
            this, SLOT(startGameButtonClicked()));
    startGameButton->show();

    resumeGameButton = new QPushButton("PAUSE", statusWindow);
    resumeGameButton->setGeometry(QRectF(QPointF(200, 0), QSizeF(80, 30)).toRect());
    connect(resumeGameButton, SIGNAL(clicked(bool)),this,
            SLOT(resumeGameButtonClicked()));
    if(!singleMode)
        resumeGameButton->setEnabled(false);
    resumeGameButton->setVisible(false);

    speedUpButton = new QPushButton("FASTER", statusWindow);
    speedUpButton->setGeometry(QRectF(QPointF(400, 0), QSizeF(80, 30)).toRect());
    connect(speedUpButton, SIGNAL(clicked(bool)),this,
            SLOT(speedUpButtonClicked()));
    speedUpButton->show();

    speedDownButton = new QPushButton("SLOWER", statusWindow);
    speedDownButton->setGeometry(QRectF(QPointF(300, 0), QSizeF(80, 30)).toRect());
    connect(speedDownButton, SIGNAL(clicked(bool)),this,
            SLOT(speedDownButtonClicked()));
    speedDownButton->show();


    autoViewButton = new QPushButton("Manual", statusWindow);
    if(!autoView)autoViewButton->setText("Auto");
    autoViewButton->setGeometry(QRectF(QPointF(500, 0), QSizeF(80, 30)).toRect());
    connect(autoViewButton, SIGNAL(clicked(bool)),this,
            SLOT(autoViewButtonClicked()));
    autoViewButton->setVisible(true);

    singleContinousButton = new QPushButton("Continous", statusWindow);
    singleContinousButton->setGeometry(QRectF(QPointF(600, 0), QSizeF(80, 30)).toRect());
    connect(singleContinousButton, SIGNAL(clicked(bool)),this,
            SLOT(singleContinousButtonClicked()));
    singleContinousButton->show();

    qDebug()<<"Loaded buttons";

    roundComboBox = new QComboBox(statusWindow);
    roundComboBox->setGeometry(QRect(QPoint(600, 50), QSize(100,30)));
    connect(roundComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(goToLoopBeginCallback()));

    roundInfo = new QTextBrowser(statusWindow);
    roundInfo->setText("GAME NOT START");
    roundInfo->setGeometry(QRectF(QPointF(400, 50), QSizeF(200, 30)).toRect());
    roundInfo->show();

    playerInfoTable = new QTableWidget(statusWindow);
    playerInfoTable->setRowCount(UI::TPlayer::properties.size()+1);
    playerInfoTable->setColumnCount(4);
    QStringList tableHorizonalHead, tableVerticleHead;
    for(int i = 0;i<4;i++)
    {
        QString str = "Player "+QString::number(i);
        tableHorizonalHead<<str;
    }
    tableVerticleHead = QStringList(UI::TPlayer::properties.values());
    tableVerticleHead.append("TowerLabel");
    playerInfoTable->setHorizontalHeaderLabels(tableHorizonalHead);
    playerInfoTable->setVerticalHeaderLabels(tableVerticleHead);
    playerInfoTable->setShowGrid(false);
    playerInfoTable->setGeometry(QRect(QPoint(0,400), QSize(600, 500)));
    playerInfoTable->setVisible(true);

    commandInfoList = new QListWidget(statusWindow);
    commandInfoList->setGeometry(QRect(QPoint(0, 300), QSize(600, 99)));
    commandInfoList->setVisible(true);

    towerInfo = new QTextBrowser(this);
    towerInfo->setGeometry(QRect(QPoint(0, 200), QSize(600, 99)));
    towerInfo->setText("For tower");
    QGraphicsOpacityEffect* effectTowerInfo = new QGraphicsOpacityEffect();
    effectTowerInfo->setOpacity(0.5);
    towerInfo->setGraphicsEffect(effectTowerInfo);
    towerInfo->hide();

    soldierInfo = new QTextBrowser(this);
    soldierInfo->setGeometry(QRect(QPoint(0, 0), QSize(600, 99)));
    soldierInfo->setText("For soldier");
    QGraphicsOpacityEffect* effectSoldierInfo = new QGraphicsOpacityEffect();
    effectSoldierInfo->setOpacity(0.5);
    towerInfo->setGraphicsEffect(effectSoldierInfo);
    soldierInfo->hide();

    qDebug()<<"Loaded other UI components.";

    Worker* worker = new Worker;
    worker->moveToThread(&workThread);
    connect(this, &PlayScene::runThreadStart, worker, &Worker::doWork);

    //connect(worker,  SIGNAL(finishedWork()), &workThread, SLOT(quit());
    //connect(worker, SIGNAL(finishedWork()), worker, SLOT(deleteLater()));
    connect(&workThread, SIGNAL(finished()), &workThread, SLOT(deleteLater()));
    workThread.start();
    thread_pause = false;

    qDebug()<<"Loaded worker";

    opacityTimer = new QTimer(this);
    connect(opacityTimer, SIGNAL(timeout()), this, SLOT(opacityUpdate()));

    updateGeometryTimer = new QTimer(this);
    connect(updateGeometryTimer, SIGNAL(timeout()),this, SLOT(myUpdateGeometry()));
    updateGeometryTimer->start(5);

    raiseTimer = new QTimer(this);
    connect(raiseTimer, SIGNAL(timeout()),this, SLOT(raiseWidgetss()));
    raiseTimer->start(1);

    autoViewMaxStep = 0;
    autoViewCurrentStep = 0;
    autoViewInterval = 10;//msec per Operation
    focusTime = 300;//msec
    autoViewTimer = new QTimer(this);
    connect(autoViewTimer, SIGNAL(timeout()), this, SLOT(autoViewAdjust()));

    qDebug()<<"Loaded timers.";

    statusWindow->show();
}
void PlayScene::init()
{
    this->clearSoldiers();
    this->clearTowers();
    this->originPoint = QPoint(0,0);
    this->pixelSize = defaultPixelSize;
    thread_pause = false;
    singleMode = true;
    singleContinousButton->setText("Continous");
    autoView = true;
    autoViewButton->setText("Manual");
    roundInfo->setText("GAME NOT START");
    commandInfoList->clear();
}
PlayScene::~PlayScene()
{
    exit_thread_flag = true;
    this->hide();
    if(workThread.isRunning())
    {
        workThread.quit();
        workThread.wait();
    }
    if(startGameButton !=nullptr)
        delete startGameButton;
    if(resumeGameButton!=nullptr)
        delete resumeGameButton;
    if(speedDownButton!=nullptr)
        delete speedDownButton;
    if(speedUpButton!=nullptr)
        delete speedUpButton;
    if(goBackButton!=nullptr)
        delete goBackButton;
    if(mapBackGround!=nullptr)
        delete mapBackGround;
    if(rightBackGround != nullptr)
        delete rightBackGround;
    if(autoViewButton!=nullptr)
        delete autoViewButton;
    if(playerInfoTable!=nullptr)
        delete playerInfoTable;
    if(commandInfoList!=nullptr)
        delete commandInfoList;

    delete ui;
}

void PlayScene::startGameButtonClicked()
{
    if(startGameButton->text()=="START")
    {
        UI::MainLogic::GetInstance()->GameStart();
        init();
        exit_thread_flag = false;
        emit runThreadStart();
        startGameButton->setText("STOP");
        resumeGameButton->setText("PAUSE");
        resumeGameButton->setVisible(true);

        roundComboBox->clear();
        for(auto item: UI::MainLogic::GetInstance()->fileTemp.keys())
        {
            roundComboBox->addItem(QString::number(item));
        }
    }
    else if(startGameButton->text()=="STOP")
    {
        UI::MainLogic::GetInstance()->GameOver();
        exit_thread_flag = true;
        startGameButton->setText("START");
        resumeGameButton->setText("PAUSE");
        resumeGameButton->setVisible(false);
    }
}

void PlayScene::resumeGameButtonClicked()
{
    if(resumeGameButton->text()=="RESUME")
    {
        thread_pause = false;
        resumeGameButton->setText("PAUSE");
    }
    else if(resumeGameButton->text()=="PAUSE")
    {
        thread_pause = true;
        resumeGameButton->setText("RESUME");
    }
}

void PlayScene::speedUpButtonClicked()
{
    if(UI::MainLogic::GetInstance()->speed>0.033)
        UI::MainLogic::GetInstance()->speed*= 0.91;
    else
        UI::MainLogic::GetInstance()->speed = 0.03;
}

void PlayScene::speedDownButtonClicked()
{
    UI::MainLogic::GetInstance()->speed*= 1.1;
}

void PlayScene::goBackButtonClicked()
{
    UI::MainLogic::GetInstance()->startScene->show();
    //UI::MainLogic::GetInstance()->playScene->hide();
    qDebug()<<"Back clicked, set exit_thread_flag = true";
    exit_thread_flag = true;
    init();
    startGameButton->setText("START");
}

void PlayScene::autoViewButtonClicked()
{
    if(autoView == true)
    {
        autoView = false;
        autoViewButton->setText("Auto");
    }
    else if(autoView == false)
    {
        autoView = true;
        autoViewButton->setText("Manual");
    }
}

void PlayScene::singleContinousButtonClicked()
{
    if(singleMode == true)
    {
        singleMode = false;
        thread_pause = false;
        resumeGameButton->setEnabled(false);
        singleContinousButton->setText("Single");

    }
    else
    {
        singleMode = true;
        resumeGameButton->setEnabled(true);
        singleContinousButton->setText("Continous");
    }
}

void PlayScene::myUpdateGeometry()
{
    mapBackGround->setGeometry(QRectF(originPoint,QSizeF(mapSize.width()*pixelSize.width(), mapSize.height()*pixelSize.height())).toRect());
    //const_cast<QRect&>(mapBackGround->geometry()).setRight(std::max(mapBackGround->geometry().right(), 1000));
    UI::TTower*ttower;
    for(auto it = towers.begin(); it!=towers.end();it++)
    {
        ttower = UI::MainLogic::GetInstance()->towers[it.key()];
        try
        {
            it.value()->setGeometry(QRect(mapToGeo(ttower->m_Position+QPoint(-1,1)),(pixelSize*3).toSize()));
        }
        catch(const std::exception&){}
    }
    UI::TSoldier*tsoldier;
    for(auto it = soldiers.begin(); it!=soldiers.end();it++)
    {
        if(UI::MainLogic::GetInstance()->soldiers.find(it.key())!=UI::MainLogic::GetInstance()->soldiers.end())
            tsoldier = UI::MainLogic::GetInstance()->soldiers[it.key()];
        else
            tsoldier =nullptr;
        if(tsoldier!=nullptr)
        {
            try
            {
                //if((*(it.value()->actions().size())

                it.value()->setGeometry(QRect(mapToGeo(tsoldier->m_Position),pixelSize.toSize()));
            }
            catch(const std::exception&){}
        }
        else
        {
            qDebug()<<"Fail to update soldier geometry in update geometry";

        }
    }

    for(MoveSoldier* move : MoveSoldier::moveToDelete)
    {    delete move;
    }
    MoveSoldier::moveToDelete.clear();
    if(singleMode && resumeGameButton->text()=="RESUME")//state is Pause
        thread_pause = true;

    for(auto it = towers.begin();it!=towers.end();it++)
    {
        int id = it.key();
        qDebug()<<"id = "<<id;
        if(towerBars.keys().contains(id))
        {
            qDebug()<<"TowerBars contain";
            towerBars[id]->setGeometry(QRect(it.value()->geometry().topLeft(), QSize(pixelSize.width()*3*UI::MainLogic::GetInstance()->towers[id]->m_nBlood/300.0, pixelSize.height())));
            towerBars[id]->raise();
            towerBars[id]->show();
            qDebug()<<"Geometry is "<<towerBars[id]->geometry();
        }
        else
        {
            QImage img;
            if(img.load(":/FC16UIResource/red.png"))
            {
                towerBars[id]=new QLabel(this);
                towerBars[id]->setGeometry(0,0,0,0);
                towerBars[id]->setScaledContents(true);
                towerBars[id]->raise();
                towerBars[id]->show();
                towerBars[id]->setPixmap(QPixmap::fromImage(img));
                QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
                effect->setOpacity(0.6);
                towerBars[id]->setGraphicsEffect(effect);
                qDebug()<<"Load red succeeded";
            }
            else
            {
                qDebug()<<"Load red failed.";
            }
        }
    }
    for(auto it = towerBars.begin();it!=towerBars.end();it++)
    {
        if(!towers.keys().contains(it.key()))
        {
            delete it.value();
            towerBars.remove(it.key());
        }
    }

}

void PlayScene::raiseWidgetss()
{
    /*
    this->startGameButton->raise();
    this->resumeGameButton->raise();
    this->goBackButton->raise();
    this->speedDownButton->raise();
    this->speedUpButton->raise();
    this->singleContinousButton->raise();
    this->roundInfo->raise();
    this->commandInfoList->raise();
    this->playerInfoTable->raise();
    */
    this->soldierInfo->raise();
    this->towerInfo->raise();

}

void PlayScene::focusOn(const QPointF &point)
{
    if(autoView && (mapToGeo(point)-QPointF(450,450)).manhattanLength()>200)
    {
        pixelSizeDiff = autoViewPixelSize-pixelSize;
        targetFocusPoint = point;
        autoViewInterval = 20;
        autoViewMaxStep = std::max(1.0f, UI::MainLogic::GetInstance()->speed*focusTime/autoViewInterval);
        autoViewCurrentStep = 0;
        thread_pause = true;
        qDebug()<<"In focus on, set thread_pause = true";
        autoViewTimer->start(autoViewInterval);

    }
}
void PlayScene::focusOn(const QPoint &point)
{
    if(autoView && (mapToGeo(point)-QPoint(450,450)).manhattanLength()>200)
    {
        pixelSizeDiff = autoViewPixelSize-pixelSize;
        targetFocusPoint = point;
        autoViewInterval = 20;
        autoViewMaxStep = focusTime/autoViewInterval;
        autoViewCurrentStep = 0;
        thread_pause = true;
        qDebug()<<"In focus on, set thread_pause = true";
        autoViewTimer->start(autoViewInterval);

    }
}

void PlayScene::autoViewAdjust()
{
    if(!autoView || autoViewCurrentStep>=autoViewMaxStep || autoViewMaxStep<=0)
    {
        autoViewTimer->stop();
        return;
    }
    else
    {
        qDebug()<<"In autoViewAdjust: set thread_pause = true";thread_pause = true;
        pixelSize+= pixelSizeDiff/autoViewMaxStep;
        originPoint += (QPoint(450,450) - mapToGeo(targetFocusPoint))/(autoViewMaxStep-autoViewCurrentStep);
        autoViewCurrentStep++;
        qDebug()<<"Auto view current Step is:"<<autoViewCurrentStep<<" -- maxStep is: "<<autoViewMaxStep;
        if(autoViewCurrentStep >= autoViewMaxStep && autoViewMaxStep>0)
        {
            qDebug()<<"autoViewAdjust Finished, set thread_pause = false";thread_pause = false;
            autoViewCurrentStep = autoViewMaxStep = 0;
        }
    }
}

void PlayScene::playerUpdate(UI::TPlayer*player)
{
    qDebug()<<"In player Update";
    if(player==nullptr)
    {
        qDebug()<<"player == nullptr, cannot update";
        return;
    }
    int col = player->m_nID;

    for(int row: UI::TPlayer::properties.keys())
    {
        QTableWidgetItem* item = new QTableWidgetItem();
        switch(row)
        {
        case 0:
        {
            item->setText(QString::number(player->m_nRank));
            break;
        }
        case 1:
        {
            item->setText(QString::number(player->m_nScore));
            break;
        }
        case 2:
        {
            item->setText(QString::number(player->m_nKillNum));
            break;
        }
        case 3:
        {
            item->setText(QString::number(player->m_nSurvivalRound));
            break;
        }
        case 4:
        {
            item->setText(QString::number(player->m_nResource));
            break;
        }
        case 5:
        {
            item->setText(QString::number(player->m_nMaxPopulation));
            break;
        }
        case 6:
        {
            item->setText(QString::number(player->m_nPopulation));
            break;
        }
        case 7:
        {
            item->setText(QString::number(player->m_nTowerNum));
            break;
        }
        case 8:
        {
            QString temp;
            for(UI::TTower* tower: player->m_vecTowers)
            {
                temp +=QString::number(tower->m_nID)+" ";
            }
            item->setText(temp);
            break;
        }
        case 9:
        {
            item->setText(QString::number(player->m_nSoldierNum));
            break;
        }
        case 10:
        {
            QString temp;
            for(UI::TSoldier* soldier: player->m_vecrSoldiers)
            {
                temp +=QString::number(soldier->m_nID)+" ";
            }
            item->setText(temp);
            break;
        }
        default:
            break;
        }
        playerInfoTable->setItem(row, col, item);
    }
    playerInfoTable->setItem(UI::TPlayer::properties.size(),col,
                             new QTableWidgetItem(QIcon(":/FC16UIResource/tileset/towers/tower"+QString::number(player->m_nID)+".png"),""));
}

void PlayScene::towerUpdate(UI::TTower*tower)
{

    QImage img;
    QString fileName = ":/FC16UIResource/tileset/towers/tower"+QString::number(tower->getOwnerID())+".png";
    if(img.load(fileName))
    {
        QLabel* towerLabel = nullptr;
        if(towers.find(tower->m_nID)==towers.end())
        {
            towerLabel =new QLabel(this);
            towerLabel->setGeometry(QRectF(originPoint+QPointF((tower->m_Position.x()-1)*pixelSize.width(), (tower->m_Position.y()-1)*pixelSize.height()),
                                           pixelSize*3).toRect());
            towerLabel->setScaledContents(true);
            towerLabel->raise();
            towerLabel->show();
            towerLabel->setText(QString::number(tower->m_nID));
            towers[tower->m_nID] = towerLabel;

        }
        else
        {
            towerLabel = towers[tower->m_nID];
        }
        towerLabel->setPixmap(QPixmap::fromImage(img));
    }
    else
    {
        UI::MainLogic::GetInstance()->WriteLog("Fail to load the image of soldier");
    }

}

void PlayScene::soldierUpdate(UI::TSoldier*soldier)
{

    QImage img;
    QString fileName = ":/FC16UIResource/tileset/workspace/"+soldier->getImageName();
    if(img.load(fileName))
    {
        QLabel* soldierLabel =nullptr;
        if(soldiers.find(soldier->m_nID) == soldiers.end())
        {
            soldierLabel = new QLabel(this);
            soldiers[soldier->m_nID] = soldierLabel;
            qDebug()<<"A new SOldierLabel created, id:"<<soldier->m_nID;

        }
        else
            soldierLabel = soldiers[soldier->m_nID];
        soldierLabel->setGeometry(QRectF(originPoint+QPointF(soldier->m_Position.x()*pixelSize.width(), soldier->m_Position.y()*pixelSize.height()),
                                         pixelSize).toRect());
        soldierLabel->setScaledContents(true);
        soldierLabel->setPixmap(QPixmap::fromImage(img));
        soldierLabel->raise();
        soldierLabel->show();
        UI::MainLogic::GetInstance()->WriteLog("Successfully loaded the image of soldier");

    }
    else
    {
        UI::MainLogic::GetInstance()->WriteLog("Fail to load the image of soldier");
    }
    UI::MainLogic::GetInstance()->WriteLog("SoldierFilename is "+fileName.toStdString());


}

void PlayScene::commandUpdate(UI::Command*command)
{
    qDebug()<<"In command update, set thread_pause = true";thread_pause = true;
    opacityLabels.clear();

    //CommandList

    try
    {
        QListWidgetItem* item = new QListWidgetItem(
                    "Round "+ QString::number(UI::MainLogic::GetInstance()->gameRound)+
                    ": Player-"+QString::number(command->m_pOwner->m_nID)+":"+
                    command->m_pOwner->m_strVecCommands[command]);
        commandInfoList->addItem(item);
        commandInfoList->setCurrentItem(item);
    }
    catch(const std::exception&)
    {}

    float speed = UI::MainLogic::GetInstance()->speed;
    switch (command->m_nCommandType) {
    case UI::CommandType::Move:
    {

        QPointF translateVec;
        switch(command->m_nMoveDirection)
        {
        case UI::UP:
            translateVec = QPointF(0,1)*command->m_nMoveDistance;
            qDebug()<<"MoveMove UP:";
            break;
        case UI::DOWN:
            translateVec = QPointF(0,-1)*command->m_nMoveDistance;
            qDebug()<<"MoveMove DOWN";
            break;
        case UI::RIGHT:
            translateVec = QPointF(1,0)*command->m_nMoveDistance;
            qDebug()<<"MoveMove RIGHT";
            break;
        case UI::LEFT:
            translateVec = QPointF(-1,0)*command->m_nMoveDistance;
            qDebug()<<"MoveMove LEFT";
            break;
        default:
            break;
        }
        MoveSoldier* moveAction = new MoveSoldier(&command->m_pMoveSoldier->m_Position, speed*500);
        moveAction->setValue(0, command->m_pMoveSoldier->m_Position);
        moveAction->setValue(1, command->m_pMoveSoldier->m_Position+translateVec);
        if(true)//command->m_nMoveDirection == UI::LEFT)
        {
            qDebug()<<"m_pMovePosition:"<<command->m_pMoveSoldier->m_Position;
            qDebug()<<"Dest:"<<command->m_pMoveSoldier->m_Position+translateVec;
        }
        moveAction->startMove();

    }
        break;
    case UI::CommandType::Upgrade:
    {
        QImage img;
        if(img.load(":/FC16UIResource/upgrade.png"))
        {
            QLabel* upgradeLabel = new QLabel(this);
            upgradeLabel->setScaledContents(true);
            upgradeLabel->setGeometry(QRect(mapToGeo(command->m_pUpgradeTower->m_Position+QPoint(-1,1)),pixelSize.toSize()*3));
            upgradeLabel->setPixmap(QPixmap::fromImage(img));
            upgradeLabel->show();
            upgradeLabel->raise();
            QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
            effect->setOpacity(0);
            upgradeLabel->setGraphicsEffect(effect);
            QPropertyAnimation*animation = new QPropertyAnimation(effect, "opacity");
            animation->setDuration(speed*1000);
            int maxNum = 1000;
            int i;
            for(i = 0;i<maxNum/4;i++)
            {
                animation->setKeyValueAt(float(i)/maxNum,4*float(i)/maxNum );
            }
            for(i = maxNum*3/4;i<=maxNum;i++)
            {
                animation->setKeyValueAt(float(i)/maxNum,4*(1-float(i)/maxNum) );

            }
            opacityLabels.push_back(upgradeLabel);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
            opacityTimer->start(20);
            QTimer::singleShot(animation->duration(),this, SLOT(resumeThread()));

        }
        else
        {
            qDebug()<<"Fail to load upgrade.png ";thread_pause = false;
        }
    }
        break;
    case UI::CommandType::Produce:
    {

        QImage img;
        if(img.load(":/FC16UIResource/produce.png"))
        {
            QLabel* produceLabel = new QLabel(this);
            produceLabel->setGeometry(QRect(mapToGeo(command->m_pProduceTower->m_Position+QPoint(-1,1)),pixelSize.toSize()*3));
            produceLabel->setScaledContents(true);
            produceLabel->setPixmap(QPixmap::fromImage(img));
            produceLabel->show();
            produceLabel->raise();

            QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
            effect->setOpacity(0);
            produceLabel->setGraphicsEffect(effect);

            QPropertyAnimation*animation = new QPropertyAnimation(effect, "opacity");
            animation->setDuration(speed*1000);
            int maxNum = 1000;
            int i;
            for(i = 0;i<maxNum/4;i++)
            {
                animation->setKeyValueAt(float(i)/maxNum,sqrtf(4*float(i)/maxNum) );
            }
            for(i=maxNum*3/4;i<=maxNum;i++)
            {
                animation->setKeyValueAt(float(i)/maxNum,sqrtf(4*(1-float(i)/maxNum)) );

            }
            opacityLabels.push_back(produceLabel);

            animation->start(QAbstractAnimation::DeleteWhenStopped);
            opacityTimer->start(20);

            QTimer::singleShot(animation->duration(),this, SLOT(resumeThread()));

        }
        else
        {
            qDebug()<<"load produce FAIL";thread_pause = false;
        }
    }
        break;
    case UI::CommandType::Attack:
    {
        QImage imgAttack, imgVictim;
        if(imgAttack.load(":/FC16UIResource/attack.jpg"))
        {
            QLabel* attackLabel = new QLabel(this);
            attackLabel->setGeometry(QRect(mapToGeo(command->m_pAttackObject->m_Position),pixelSize.toSize()*1));
            attackLabel->setScaledContents(true);
            attackLabel->setPixmap(QPixmap::fromImage(imgAttack));
            attackLabel->show();
            attackLabel->raise();

            QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
            effect->setOpacity(0);
            attackLabel->setGraphicsEffect(effect);

            QPropertyAnimation*animation = new QPropertyAnimation(effect, "opacity");
            animation->setDuration(speed*1000);
            int maxNum = 1000;
            int i;
            for(i = 0;i<maxNum/4;i++)
            {
                animation->setKeyValueAt(float(i)/maxNum,sqrtf(4*float(i)/maxNum) );
            }
            for(i=maxNum*3/4;i<=maxNum;i++)
            {
                animation->setKeyValueAt(float(i)/maxNum,sqrtf(4*(1-float(i)/maxNum)) );

            }
            opacityLabels.push_back(attackLabel);

            animation->start(QAbstractAnimation::DeleteWhenStopped);
            opacityTimer->start(20);

            QTimer::singleShot(animation->duration(),this, SLOT(resumeThread()));

        }
        else
        {
            qDebug()<<"load attack FAIL";thread_pause = false;
        }
        if(imgVictim.load(":/FC16UIResource/victim.jpg"))
        {
            QLabel* victimLabel = new QLabel(this);
            QPointF vicPos;
            UI::TSoldier* victim_s = dynamic_cast<UI::TSoldier*>(command->m_pVictimObject);
            UI::TTower* victim_t = dynamic_cast<UI::TTower*>(command->m_pVictimObject);
            if(victim_s!=nullptr)
            {
                vicPos = victim_s->m_Position;
            }
            else if(victim_t !=nullptr)
            {
                vicPos = victim_t->m_Position;
            }
            victimLabel->setGeometry(QRect(mapToGeo(vicPos),pixelSize.toSize()*1));
            victimLabel->setScaledContents(true);
            victimLabel->setPixmap(QPixmap::fromImage(imgVictim));
            victimLabel->show();
            victimLabel->raise();

            QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
            effect->setOpacity(0);
            victimLabel->setGraphicsEffect(effect);

            QPropertyAnimation*animation = new QPropertyAnimation(effect, "opacity");
            animation->setDuration(speed*1000);
            int maxNum = 1000;
            int i;
            for(i = 0;i<maxNum/4;i++)
            {
                animation->setKeyValueAt(float(i)/maxNum,sqrtf(4*float(i)/maxNum) );
            }
            for(i=maxNum*3/4;i<=maxNum;i++)
            {
                animation->setKeyValueAt(float(i)/maxNum,sqrtf(4*(1-float(i)/maxNum)) );

            }
            opacityLabels.push_back(victimLabel);

            animation->start(QAbstractAnimation::DeleteWhenStopped);

        }
        else
        {
            qDebug()<<"load victim FAIL";thread_pause = false;
        }
    }
        break;
        /*case 520://UI::CommandType::Attack:
    {
        UI::TSoldier* attacker = command->m_pAttackObject;

        UI::TSoldier* victim_s = dynamic_cast<UI::TSoldier*>(command->m_pVictimObject);
        UI::TTower* victim_t = dynamic_cast<UI::TTower*>(command->m_pVictimObject);
        if(attacker==nullptr)
        { qDebug()<<"attacker== nullptr";thread_pause = false;return;}
        QLabel* victimLabel = nullptr;
        QPointF vicPos;

        if(victim_s!=nullptr)
        {
            victimLabel = soldiers[victim_s->m_nID];
            vicPos = victim_s->m_Position;
        }
        else if(victim_t !=nullptr)
        {
            victimLabel = towers[victim_t->m_nID];
            vicPos = victim_t->m_Position;
        }
        else
        {
            return;
        }
        MoveSoldier* moveAction = new MoveSoldier(&command->m_pAttackObject->m_Position, speed*1000);
        moveAction->setValue(0, command->m_pAttackObject->m_Position);
        moveAction->setValue(0.5, vicPos);
        moveAction->setValue(1, command->m_pAttackObject->m_Position);
        moveAction->startMove();

        if(victimLabel!=nullptr)
        {
            QGraphicsOpacityEffect* effect = dynamic_cast<QGraphicsOpacityEffect*>(victimLabel->graphicsEffect());
            if(effect == nullptr)
                effect = new QGraphicsOpacityEffect();
            effect->setOpacity(0);
            victimLabel->setGraphicsEffect(effect);
            QPropertyAnimation*animation = new QPropertyAnimation(effect, "opacity");
            animation->setDuration(moveAction->duration);
            animation->setStartValue(1);
            int maxNum = 1000;
            int i;
            for(i = 0.5*maxNum;i<=0.8*maxNum;i++)
            {
                if(i%2==0)
                    animation->setKeyValueAt(float(i)/maxNum,0);
                else
                    animation->setKeyValueAt(float(i)/maxNum,1);
            }
            animation->setKeyValueAt(float(i)/maxNum,1);
            animation->setEndValue(1);
            opacityLabels.push_back(victimLabel);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
            opacityTimer->start(20);
        }

        QTimer::singleShot(moveAction->duration, this, SLOT(resumeThread()));


        break;
    }
    */
    default:
        qDebug()<<"Incorrect command type, thread_pause = false";thread_pause = false;
        break;
    }
}

void PlayScene::roundUpdate()
{
    roundInfo->setText("Round "+QString::number(UI::MainLogic::GetInstance()->gameRound));

}

void PlayScene::clearTowers()
{
    for(auto tower:towers)
    {    try
        {
            tower->hide();

            UI::MainLogic::GetInstance()->WriteLog("clear a tower");
        }
        catch(const std::exception&)
        {

        }
    }
    towers.clear();
}

void PlayScene::clearSoldiers()
{
    for(auto id: soldiers.keys())
    {    try
        {
            if(UI::MainLogic::GetInstance()->soldiers.find(id) == UI::MainLogic::GetInstance()->soldiers.end())
            {
                soldiers[id]->hide();
                delete soldiers[id];
                qDebug()<<"hide a soldier, id: "<<id;
                soldiers.remove(id);
                UI::MainLogic::GetInstance()->WriteLog("clear a soldier");
            }
        }
        catch(const std::exception&)
        {
            qDebug()<<"Clear soldier raise error";

        }
    }
    //soldiers.clear();
}

void PlayScene::resumeThread()
{
    if(singleMode && resumeGameButton->text()=="RESUME")//state is Pause
        return;
    qDebug()<<"ResumeThread: set thread_pause = false";thread_pause = false;
    opacityLabels.clear();
    opacityTimer->stop();
}

void PlayScene::resumeThreadAutoView()
{
    if(singleMode && resumeGameButton->text()=="RESUME")//state is Pause
        return;
    qDebug()<<"ResumeThreadAutoView: set thread_pause = false";thread_pause = false;
    opacityLabels.clear();
    opacityTimer->stop();

}

void PlayScene::goToLoopBeginCallback()
{
    goToLoopBegin_flag = true;
}

void PlayScene::opacityUpdate()
{
    for(QLabel* item: opacityLabels)
    {
        try
        {
            item->setGraphicsEffect(item->graphicsEffect());
        }
        catch(const std::exception&)
        {}

    }
}

QPoint PlayScene::mapToGeo(const QPoint& pos)
{
    return QPoint(originPoint.x()+ pos.x()*pixelSize.width(),
                  originPoint.y()+ (mapSize.height()-1 -pos.y())*pixelSize.height());

}
QPoint PlayScene::mapToGeo(const QPointF& pos)
{
    return QPoint(originPoint.x()+ pos.x()*pixelSize.width(),
                  originPoint.y()+ (mapSize.height()-1 -pos.y())*pixelSize.height());

}

void PlayScene::mousePressEvent(QMouseEvent *)
{
    if(exit_thread_flag)
        return;
    else
    {
        QPoint mousePos = QCursor::pos()-this->geometry().topLeft();
        qDebug()<<"mousePos: "<<mousePos;
        soldierInfo->hide();
        towerInfo->hide();
        soldierInfo->clear();
        towerInfo->clear();
        for(UI::TSoldier*soldier: UI::MainLogic::GetInstance()->soldiers)
        {
            try
            {
                if(soldiers.find(soldier->m_nID)!=soldiers.end()&&(soldiers[soldier->m_nID]->geometry().center()-mousePos).manhattanLength()<=(pixelSize.width()+pixelSize.height())*0.25)
                {
                    updateSoldierInfo(soldier);
                    soldierInfo->setGeometry(QRect(mousePos + QPoint(10,10), soldierInfo->size()));
                    soldierInfo->show();
                    return;
                }
            }
            catch(const std::exception&){}
        }
        for(UI::TTower*tower:UI::MainLogic::GetInstance()->towers)
        {
            try
            {
                if((towers[tower->m_nID]->geometry().center() - mousePos).manhattanLength()<=(pixelSize.width()+pixelSize.height()))
                {
                    updateTowerInfo(tower);
                    towerInfo->setGeometry(QRect(mousePos+QPoint(10, 10), towerInfo->size()));
                    towerInfo->show();
                }
            }
            catch(const std::exception&){}
        }
    }
}

void PlayScene::updateSoldierInfo(UI::TSoldier *soldier)
{
    QString content = soldierInfo->document()->toPlainText();
    content += "Soldier ID:"+QString::number(soldier->m_nID)+
            "; Owner:"+QString::number(soldier->m_pOwner->m_nID)+"; Blood:"+QString::number(soldier->m_nBlood)
            +"; "+UI::SoldierTypeEnum2Str(soldier->m_nSoldierType)+"\n";
    soldierInfo->setText(content);
}
void PlayScene::updateTowerInfo(UI::TTower *tower)
{
    QString content = towerInfo->document()->toPlainText();
    content+= "Tower ID: "+QString::number(tower->m_nID)+
            "; Owner:"+QString::number(tower->getOwnerID())+"; Blood:"+QString::number(tower->m_nBlood)
            +"; Level:"+QString::number(tower->m_nLevel);
    if(tower->m_bRecruiting)
        content +="; RecuitingRound:"+QString::number(tower->m_nRecruitingRound)
                +" "+UI::SoldierTypeEnum2Str(tower->m_nRecruitingType);
    content+="\n";
    towerInfo->setText(content);
}

void PlayScene::wheelEvent(QWheelEvent *event)
{
    if(autoView)
        return;
    else if(!autoView)
    {
        if(event->modifiers() == Qt::ControlModifier)
        {
            float rate = std::max(1 + event->delta()*wheelScaleRate, 0.6f);

            pixelSize *= rate;
            QPoint mousePos = QCursor::pos()-this->geometry().topLeft();

            originPoint.setX(mousePos.x() + (originPoint.x()-mousePos.x())*rate);
            originPoint.setY(mousePos.y() + (originPoint.y()-mousePos.y())*rate);
        }
        else
        {
            originPoint.setY(originPoint.y()+event->delta()/120*pixelSize.height()*translateScaleRate);
        }
        myUpdateGeometry();
    }
}

void PlayScene::keyPressEvent(QKeyEvent *event)
{
    if(!autoView)
    {
        signed short temp =  (signed short)(event->key());
        qDebug()<<"Temp is: "<<temp;
        if( event->key() == Qt::Key_W || event->key() == Qt::Key_Up)
        {
            //Up
            originPoint.setY(originPoint.y()-translateScaleRate*pixelSize.height());
        }
        else if(event->key() == Qt::Key_A || event->key()==Qt::Key_Left)
        {
            //Left
            originPoint.setX(originPoint.x()-translateScaleRate*pixelSize.width());
        }
        else if(event->key() == Qt::Key_S || event->key() == Qt::Key_Down)
        {
            //Down
            originPoint.setY(originPoint.y()+translateScaleRate*pixelSize.height());
        }
        else if( event->key() == Qt::Key_D || event->key() == Qt::Key_Right)
        {
            //Right
            originPoint.setX(originPoint.x()+translateScaleRate*pixelSize.width());
        }
    }
    if(event->key() == Qt::Key_B)
    {
        try
        {
            roundComboBox->setCurrentIndex(roundComboBox->currentIndex()-1);
        }
        catch(const std::exception&){}
    }
    if(event->key()==Qt::Key_N)
    {
        try
        {
            roundComboBox ->setCurrentIndex(roundComboBox->currentIndex()+1);
        }
        catch(const std::exception&){}
    }
}

void Worker::doWork()
{
    PlayScene* playScene = UI::MainLogic::GetInstance()->playScene;
    qDebug()<<"Do work START +++++++++++++++++++++";
    try
    {
        while(playScene->exit_thread_flag == false)
        {
            qDebug()<<"new Round=====";
            playScene->goToLoopBegin_flag = false;
            qDebug()<<"Logic update begin";
            if(!UI::MainLogic::GetInstance()->LogicUpdate(playScene->roundComboBox->currentText().toInt()))
            {
                if(playScene->exit_thread_flag)
                    break;
            }

            qDebug()<<"Logic Update finished";
            if(playScene->exit_thread_flag)
                break;
            QThread::currentThread()->msleep(30);
            playScene->goToLoopBegin_flag = false;
            while(playScene->thread_pause);
            qDebug()<<"Logic update end";

            qDebug()<<"round update begin";
            QMetaObject::invokeMethod(playScene, "roundUpdate", Qt::QueuedConnection);
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            QThread::currentThread()->msleep(30);
            while(playScene->thread_pause);
            qDebug()<<"round update end";

            qDebug()<<"players update begin";
            for(auto player: UI::MainLogic::GetInstance()->players)
            {
                try
                {
                    QMetaObject::invokeMethod(playScene, "playerUpdate",
                                              Qt::QueuedConnection, Q_ARG(UI::TPlayer*, player));
                }
                catch(const std::exception&){}
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
                QThread::currentThread()->msleep(30);
                while(playScene->thread_pause);
            }
            qDebug()<<"players update end";



            qDebug()<<"towers update begin";
            for(auto tower: UI::MainLogic::GetInstance()->towers)
            {
                try{
                    QMetaObject::invokeMethod(playScene,"towerUpdate",
                                              Qt::QueuedConnection,Q_ARG(UI::TTower*, tower));
                }
                catch(const std::exception&){}
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
                QThread::currentThread()->msleep(30);
                while(playScene->thread_pause);
            }
            qDebug()<<"towers update end";

            qDebug()<<"Clear soldiers begin";
            QMetaObject::invokeMethod(playScene,"clearSoldiers",
                                      Qt::QueuedConnection);
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            QThread::currentThread()->msleep(30);
            while(playScene->thread_pause);
            qDebug()<<"Clear soldiers end";

            qDebug()<<"soliders update begin";
            for(auto soldier: UI::MainLogic::GetInstance()->soldiers)
            {
                try
                {
                    QMetaObject::invokeMethod(playScene,"soldierUpdate",
                                              Qt::QueuedConnection, Q_ARG(UI::TSoldier*, soldier));

                }
                catch(const std::exception&)
                {
                }
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
                QThread::currentThread()->msleep(30);
                while(playScene->thread_pause);
            }
            qDebug()<<"soldiers update end";

            qDebug()<<"Have pause?";
            if(playScene->singleMode&&playScene->resumeGameButton->text()=="PAUSE")
            {
                qDebug()<<"Pause begin";
                try
                {
                    QMetaObject::invokeMethod(playScene, "resumeGameButtonClicked",Qt::QueuedConnection);
                }
                catch(const std::exception&){}
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
                QThread::currentThread()->msleep(30);
                while(playScene->thread_pause);
                qDebug()<<"Pause end";
            }
            else
            {
                qDebug()<<"No pause";
            }

            qDebug()<<"commands update begin";
            for(UI::Command* command: UI::MainLogic::GetInstance()->commands)
            {
                QPointF focusPos;
                switch(command->m_nCommandType)
                {
                case UI::CommandType::Attack:
                {
                    focusPos = command->m_pAttackObject->m_Position;
                    break;
                }
                case UI::CommandType::Move:
                {
                    focusPos = command->m_pMoveSoldier->m_Position;
                    break;
                }
                case UI::CommandType::Upgrade:
                {
                    focusPos = command->m_pUpgradeTower->m_Position+QPoint(-1,1);
                    break;
                }
                case UI::CommandType::Produce:
                {
                    focusPos = command->m_pProduceTower->m_Position+QPoint(-1,1);
                    break;
                }
                default:
                    break;
                }
                //qDebug()<<"before thread_pause:"<<playScene->thread_pause;
                QMetaObject::invokeMethod(playScene, "focusOn", Qt::QueuedConnection, Q_ARG(QPointF, focusPos));
                // qDebug()<<"after thread_pause:"<<playScene->thread_pause;
                QThread::currentThread()->msleep(50);
                qDebug()<<"after Sleep: thread_pause:"<<playScene->thread_pause;
                int i = 0;
                if(playScene->goToLoopBegin_flag)
                    continue;
                while(playScene->thread_pause)
                {
                    qDebug()<<"In while "<<i++<<": thread_pause:"<<playScene->thread_pause;
                    QThread::currentThread()->msleep(50);
                }
                qDebug()<<"A Command update begin";
                try
                {

                    QMetaObject::invokeMethod(playScene,"commandUpdate",
                                              Qt::QueuedConnection, Q_ARG(UI::Command*, command));
                }
                catch(const std::exception&)
                {
                }
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
                QThread::currentThread()->msleep(30);
                while(playScene->thread_pause);
                qDebug()<<"A command update end";

                qDebug()<<"Have pause?";
                if(playScene->singleMode && playScene->resumeGameButton->text()=="PAUSE")
                {
                    qDebug()<<"Pause begin";
                    QMetaObject::invokeMethod(playScene, "resumeGameButtonClicked",Qt::QueuedConnection);
                    if(playScene->exit_thread_flag)
                        break;
                    QThread::currentThread()->msleep(30);
                    while(playScene->thread_pause);
                    qDebug()<<"Pause end";
                }
                else
                {
                    qDebug()<<"No pause";
                }

            }
            qDebug()<<"Commands update end";

            if(!playScene->singleMode)
                QThread::currentThread()->msleep(UI::MainLogic::GetInstance()->speed*1000);
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            playScene->roundComboBox->setCurrentIndex(playScene->roundComboBox->currentIndex()+1);
            QThread::currentThread()->msleep(30);
            while(playScene->thread_pause);
        }
    }
    catch(const std::exception&){}
    qDebug()<<"Break from doWork, exit_thread_flag = "<<playScene->exit_thread_flag;
}


QSet<MoveSoldier*> MoveSoldier::moveToDelete;


MoveSoldier::MoveSoldier(QPointF*moveObject, int duration)
{
    this->moveObject = moveObject;
    this->duration = duration;
    this->interval = 10;//default
    this->currStep=0;
    this->maxStep=duration/interval;
    this->moveTimer = new QTimer(this);
    connect(moveTimer,SIGNAL(timeout()), this, SLOT(updateValue()));
}


MoveSoldier::~MoveSoldier()
{
    delete moveTimer;
}


bool compareValue(const MoveSoldier::ValueType& a,
                  const MoveSoldier::ValueType&b)
{
    if(a.first < b.first)
        return true;
    else if(a.first>b.first)
        return false;
    else
        return 0;
}


void MoveSoldier::setValue(float step, const QPointF &value)
{
    if(step>=0 && step <=1)
        this->values.push_back(ValueType(step,value));
    std::sort(values.begin(),values.end(),compareValue);
}



void MoveSoldier::startMove()
{
    qDebug()<<"Start moveSoldier: set thread_pause = true";
    UI::MainLogic::GetInstance()->playScene->thread_pause = true;
    this->currStep = 0;
    qDebug()<<"From "<<values[0]<<" to "<<values[values.size()-1];
    moveTimer->start(interval);
}


QPair<MoveSoldier::ValueType, MoveSoldier::ValueType> MoveSoldier::getNeighbor()
{
    int index = 0;
    int valueCount = values.size();
    if(valueCount<2)
    {
        return QPair<ValueType, ValueType>(ValueType(0,*moveObject), ValueType(1,*moveObject));

    }
    else if(values[0].first>currStep/maxStep)
    {

        return QPair<ValueType, ValueType>(ValueType(0,*moveObject), values[0]);
    }
    else if(values[valueCount-1].first <currStep/maxStep)
        return QPair<ValueType, ValueType>(values[valueCount-1],ValueType(1, *moveObject));
    while(index+1<valueCount)
    {
        if(values[index+1].first<currStep/maxStep)
            index++;
        else
        {
            return QPair<ValueType, ValueType>(values[index], values[index+1]);
        }
    }
    return QPair<ValueType,ValueType>(values[0],values[values.size()-1]);
}

void MoveSoldier::updateValue()
{
    if(currStep>=maxStep && maxStep > 0)
    {
        moveTimer->stop();
        moveToDelete.insert(this);
        UI::MainLogic::GetInstance()->playScene->thread_pause = false;
    }
    else if(currStep<maxStep)
    {
        QPair<ValueType,ValueType>neighbors = getNeighbor();
        *moveObject = neighbors.first.second +
                (currStep/maxStep-neighbors.first.first)/(neighbors.second.first-neighbors.first.first)
                *(neighbors.second.second-neighbors.first.second);

        currStep++;
    }
}
