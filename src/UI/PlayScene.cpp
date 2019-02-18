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
#include "StartScene.h"

int PlayScene::commandsCount = 0;

PlayScene::PlayScene(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PlayScene)
{
    ui->setupUi(this);
    this->setWindowTitle("FC16UI-PLAY");
    this->setGeometry(QRectF(QPoint(160,90),QSize(1600, 900)).toRect());

    UI::MainLogic::GetInstance()->logFileStream<<"Main thread is "<<QThread::currentThread()<<std::endl;

    UI::MainLogic::GetInstance()->playScene = this;

    //Map setting
    mapSize = QSizeF(100, 100);
    pixelSize = QSizeF(9,9);

    originPoint = QPointF(0,0);
    wheelScaleRate = 0.0005;
    autoView = false;
    singleMode = true;
    //Game setting
    exit_thread_flag = true;

    QImage backImg;
    if(backImg.load(":/FC16UIResource/background.png"))
    {
        mapBackGround = new QLabel(this);
        mapBackGround->setGeometry(QRectF(originPoint,
            QSizeF(mapSize.width()*pixelSize.width(), mapSize.height()*pixelSize.height())).toRect());
        mapBackGround->setScaledContents(true);
        mapBackGround->setPixmap(QPixmap::fromImage(backImg));
    }

    QImage rightBackImg;
    if(rightBackImg.load(":/FC16UIResource/back/rightBack.jpg"))
    {
        rightBackGround = new QLabel(this);
        rightBackGround->setGeometry(QRect(mapBackGround->geometry().topRight(),
            this->geometry().bottomRight()-this->geometry().topLeft()));
        rightBackGround->setScaledContents(true);
        rightBackGround->setPixmap(QPixmap::fromImage(rightBackImg));
        rightBackGround->hide();
    }

    goBackButton = new QPushButton("BACK", this);
    goBackButton->setGeometry(QRectF(QPointF(900, 0), QSizeF(80, 30)).toRect());
    connect(goBackButton, SIGNAL(clicked(bool)),this,
            SLOT(goBackButtonClicked()));

    startGameButton = new QPushButton("START", this);
    startGameButton->setGeometry(QRectF(QPointF(1000, 0), QSizeF(80, 30)).toRect());
    connect(startGameButton, SIGNAL(clicked(bool)),
            this, SLOT(startGameButtonClicked()));
    startGameButton->show();

    resumeGameButton = new QPushButton("PAUSE", this);
    resumeGameButton->setGeometry(QRectF(QPointF(1100, 0), QSizeF(80, 30)).toRect());
    connect(resumeGameButton, SIGNAL(clicked(bool)),this,
            SLOT(resumeGameButtonClicked()));
    if(!singleMode)
        resumeGameButton->setEnabled(false);
    resumeGameButton->setVisible(false);

    speedUpButton = new QPushButton("FASTER", this);
    speedUpButton->setGeometry(QRectF(QPointF(1300, 0), QSizeF(80, 30)).toRect());
    connect(speedUpButton, SIGNAL(clicked(bool)),this,
            SLOT(speedUpButtonClicked()));
    speedDownButton = new QPushButton("SLOWER", this);
    speedDownButton->setGeometry(QRectF(QPointF(1200, 0), QSizeF(80, 30)).toRect());
    connect(speedDownButton, SIGNAL(clicked(bool)),this,
            SLOT(speedDownButtonClicked()));



    autoViewButton = new QPushButton("Manual", this);
    if(!autoView)autoViewButton->setText("Auto");
    autoViewButton->setGeometry(QRectF(QPointF(1400, 0), QSizeF(80, 30)).toRect());
    connect(autoViewButton, SIGNAL(clicked(bool)),this,
            SLOT(autoViewButtonClicked()));
    autoViewButton->setVisible(false);

    singleContinousButton = new QPushButton("Continous", this);
    singleContinousButton->setGeometry(QRectF(QPointF(1500, 0), QSizeF(80, 30)).toRect());
    connect(singleContinousButton, SIGNAL(clicked(bool)),this,
            SLOT(singleContinousButtonClicked()));

    roundInfo = new QLabel(this);
    roundInfo->setText("GAME NOT START");
    roundInfo->setGeometry(QRectF(QPointF(1300, 50), QSizeF(200, 30)).toRect());
    roundInfo->show();

    playerInfoTable = new QTableWidget(this);
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
    playerInfoTable->setGeometry(QRect(QPoint(900,400), QSize(600, 500)));
    playerInfoTable->setVisible(true);

    commandInfoList = new QListWidget(this);
    commandInfoList->setGeometry(QRect(QPoint(900, 300), QSize(600, 99)));
    commandInfoList->setVisible(true);

    towerInfo = new QTextBrowser(this);
    towerInfo->setGeometry(QRect(QPoint(900, 200), QSize(600, 99)));
    towerInfo->setVisible(true);
    towerInfo->setText("For tower");

    soldierInfo = new QTextBrowser(this);
    soldierInfo->setGeometry(QRect(QPoint(900, 100), QSize(600, 99)));
    soldierInfo->setVisible(true);
    soldierInfo->setText("For soldier");


    Worker* worker = new Worker;
    worker->moveToThread(&workThread);
    connect(this, &PlayScene::runThreadStart, worker, &Worker::doWork);

    //connect(worker,  SIGNAL(finishedWork()), &workThread, SLOT(quit());
    //connect(worker, SIGNAL(finishedWork()), worker, SLOT(deleteLater()));
    connect(&workThread, SIGNAL(finished()), &workThread, SLOT(deleteLater()));
    workThread.start();
    thread_pause = false;



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
    autoViewInterval = 1;
    focusTime = 25;
    autoViewTimer = new QTimer(this);
    connect(autoViewTimer, SIGNAL(timeout()), this, SLOT(autoViewAdjust()));
}
void PlayScene::init()
{
    this->clearSoldiers();
    this->clearTowers();
    this->originPoint = QPoint(0,0);
    this->pixelSize = defaultPixelSize;
    thread_pause = false;
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
        it.value()->setGeometry(QRect(mapToGeo(ttower->m_Position-QPoint(1,1)),(pixelSize*3).toSize()));
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
            it.value()->setGeometry(QRect(mapToGeo(tsoldier->m_Position),pixelSize.toSize()));
        }
        catch(const std::exception&){}
        }
        else
        {
            qDebug()<<"Fail to update soldier geometry in update geometry";
            //soldiers.remove(it.key());
        }
    }


}

void PlayScene::raiseWidgetss()
{

    this->startGameButton->raise();
    this->resumeGameButton->raise();
    this->goBackButton->raise();
    this->speedDownButton->raise();
    this->speedUpButton->raise();
    this->singleContinousButton->raise();
    this->roundInfo->raise();
    this->commandInfoList->raise();
    this->playerInfoTable->raise();
    this->soldierInfo->raise();
    this->towerInfo->raise();
}

void PlayScene::focusOn(const QPoint &point)
{
    if(autoView && (mapToGeo(point)-QPoint(450,450)).manhattanLength()>200)
    {
        pixelSizeDiff = autoViewPixelSize-pixelSize;
        targetFocusPoint = point;
        autoViewMaxStep = focusTime;
        autoViewCurrentStep = 0;
        autoViewInterval = 20;
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
        if(autoViewCurrentStep == autoViewMaxStep>0)
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
        commandInfoList->addItem(new QListWidgetItem(
          "Round "+ QString::number(UI::MainLogic::GetInstance()->gameRound)+
          ": Player-"+QString::number(command->m_pOwner->m_nID)+":"+
           command->m_pOwner->m_strVecCommands[command]));
    }
    catch(const std::exception&)
    {}

    float speed = UI::MainLogic::GetInstance()->speed;
    switch (command->m_nCommandType) {
    case UI::CommandType::Move:
    {

        UI::TSoldier* tsoldier = command->m_pMoveSoldier;

        QLabel* uiSoldier = soldiers[tsoldier->m_nID];
        QPoint translateVec;
        switch(command->m_nMoveDirection)
        {
        case UI::UP:
            translateVec = QPoint(0,1)*command->m_nMoveDistance;
            break;
        case UI::DOWN:
            translateVec = QPoint(0,-1)*command->m_nMoveDistance;
            break;
        case UI::RIGHT:
            translateVec = QPoint(1,0)*command->m_nMoveDistance;
            break;
        case UI::LEFT:
            translateVec = QPoint(-1,0)*command->m_nMoveDistance;
            break;
        default:
            break;
        }
        QPropertyAnimation*posAnimation = new QPropertyAnimation(uiSoldier,"geometry");
        posAnimation->setDuration(speed*500);
        posAnimation->setStartValue(QRectF(mapToGeo(tsoldier->m_Position), pixelSize));
        posAnimation->setEndValue(QRectF(mapToGeo(tsoldier->m_Position+translateVec), pixelSize));
        tsoldier->m_Position += translateVec;
        posAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        QTimer::singleShot(posAnimation->duration(), this, SLOT(resumeThread()));
    }
        break;
    case UI::CommandType::Upgrade:
    {
        QImage img;
        if(img.load(":/FC16UIResource/upgrade.png"))
        {
            QLabel* upgradeLabel = new QLabel(this);
            upgradeLabel->setScaledContents(true);
            upgradeLabel->setGeometry(QRect(mapToGeo(command->m_pUpgradeTower->m_Position-QPoint(1,1)),pixelSize.toSize()*3));
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
            produceLabel->setGeometry(QRect(mapToGeo(command->m_pProduceTower->m_Position-QPoint(1,1)),pixelSize.toSize()*3));
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
        UI::TSoldier* attacker = command->m_pAttackObject;

        UI::TSoldier* victim_s = dynamic_cast<UI::TSoldier*>(command->m_pVictimObject);
        UI::TTower* victim_t = dynamic_cast<UI::TTower*>(command->m_pVictimObject);
        if(attacker==nullptr)
         { qDebug()<<"attacker== nullptr";thread_pause = false;return;}
        QLabel* attackerLabel = soldiers[attacker->m_nID];
        QPoint attackerPos = attackerLabel->pos();
        QLabel* victimLabel = nullptr;
        QPoint vicPos = attackerLabel->pos();
        if(victim_s!=nullptr)
        {
            victimLabel = soldiers[victim_s->m_nID];
            vicPos = victimLabel->pos();
        }
        else if(victim_t !=nullptr)
        {
            victimLabel = towers[victim_t->m_nID];
            vicPos = mapToGeo(victim_t->m_Position);
        }
        else
        {
            return;
        }
        QPropertyAnimation* animation1 = new QPropertyAnimation(attackerLabel, "pos");
        animation1->setDuration(speed*1000);
        animation1->setKeyValueAt(0, attackerPos);
        animation1->setKeyValueAt(0.5, vicPos);
        animation1->setKeyValueAt(1, attackerPos);
        animation1->start(QAbstractAnimation::DeleteWhenStopped);

        if(victimLabel!=nullptr)
        {
            QGraphicsOpacityEffect* effect = dynamic_cast<QGraphicsOpacityEffect*>(victimLabel->graphicsEffect());
            if(effect == nullptr)
                effect = new QGraphicsOpacityEffect();
            effect->setOpacity(0);
            victimLabel->setGraphicsEffect(effect);
            QPropertyAnimation*animation = new QPropertyAnimation(effect, "opacity");
            animation->setDuration(animation1->duration());
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

        QTimer::singleShot(animation1->duration(),this, SLOT(resumeThread()));


        break;
    }
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
    qDebug()<<"Resume thread: set thread_pause = false";thread_pause = false;
    opacityLabels.clear();
    opacityTimer->stop();
}

void PlayScene::resumeThreadAutoView()
{
    qDebug()<<"ResumeThreadAutoView: set thread_pause = false";thread_pause = false;
    opacityLabels.clear();
    opacityTimer->stop();
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
                  originPoint.y()+ (mapSize.height() -pos.y())*pixelSize.height());

}

void PlayScene::mousePressEvent(QMouseEvent *event)
{
    if(exit_thread_flag)
        return;
    else
    {
        QPoint mousePos = QCursor::pos()-this->geometry().topLeft();
        qDebug()<<"mousePos: "<<mousePos;
        soldierInfo->clear();
        towerInfo->clear();
        for(UI::TSoldier*soldier: UI::MainLogic::GetInstance()->soldiers)
        {
            try
            {
                if(soldiers.find(soldier->m_nID)!=soldiers.end()&&(soldiers[soldier->m_nID]->geometry().center()-mousePos).manhattanLength()<=(pixelSize.width()+pixelSize.height()))
                    updateSoldierInfo(soldier);
            }
            catch(const std::exception&){}
        }
        for(UI::TTower*tower:UI::MainLogic::GetInstance()->towers)
        {
            try
            {
                if((towers[tower->m_nID]->geometry().center() - mousePos).manhattanLength()<=(pixelSize.width()+pixelSize.height()))
                    updateTowerInfo(tower);
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
        float rate = std::max(1 + event->delta()*wheelScaleRate, 0.6f);

        pixelSize *= rate;
        QPoint mousePos = QCursor::pos()-this->geometry().topLeft();

        originPoint.setX(mousePos.x() + (originPoint.x()-mousePos.x())*rate);
        originPoint.setY(mousePos.y() + (originPoint.y()-mousePos.y())*rate);

        myUpdateGeometry();
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

        qDebug()<<"Logic update begin";
        if(!UI::MainLogic::GetInstance()->LogicUpdate())
        {
            if(playScene->exit_thread_flag)
                break;
        }
        if(playScene->exit_thread_flag)
            break;
        QThread::currentThread()->msleep(30);
        while(playScene->thread_pause);
        qDebug()<<"Logic update end";

        qDebug()<<"round update begin";
        QMetaObject::invokeMethod(playScene, "roundUpdate", Qt::QueuedConnection);
        if(playScene->exit_thread_flag)
            break;
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
            QThread::currentThread()->msleep(30);
            while(playScene->thread_pause);
        }
        qDebug()<<"towers update end";

        qDebug()<<"Clear soldiers begin";
        QMetaObject::invokeMethod(playScene,"clearSoldiers",
                                  Qt::QueuedConnection);
        if(playScene->exit_thread_flag)
            break;
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
            QPoint focusPos;
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
                    focusPos = command->m_pUpgradeTower->m_Position-QPoint(1,1);
                    break;
                }
                case UI::CommandType::Produce:
                {
                    focusPos = command->m_pProduceTower->m_Position-QPoint(1,1);
                    break;
                }
                default:
                    break;
                }
                //qDebug()<<"before thread_pause:"<<playScene->thread_pause;
               // QMetaObject::invokeMethod(playScene, "focusOn", Qt::QueuedConnection, Q_ARG(QPoint, focusPos));
               // qDebug()<<"after thread_pause:"<<playScene->thread_pause;
               // QThread::currentThread()->msleep(50);
               // qDebug()<<"after Sleep: thread_pause:"<<playScene->thread_pause;
                //while(playScene->thread_pause)
                //{
                //    qDebug()<<"In while 1: thread_pause:"<<playScene->thread_pause;
                //    QThread::currentThread()->msleep(50);
                //}
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
        QThread::currentThread()->msleep(30);
        while(playScene->thread_pause);
    }
    }
    catch(const std::exception&){}
    qDebug()<<"Break from doWork, exit_thread_flag = "<<playScene->exit_thread_flag;
}


