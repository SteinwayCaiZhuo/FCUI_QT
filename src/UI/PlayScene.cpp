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
#include <QPainter>
#include <QIcon>
#include <QFont>
#include <QtAlgorithms>
#include <cstdlib>
#include "StartScene.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

int PlayScene::commandsCount = 0;
extern int haveFrame;
extern QPoint mapCenterPoint;
PlayScene::PlayScene(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PlayScene)
{
    ui->setupUi(this);
    this->setWindowTitle("FC16UI-PLAY");
    this->setGeometry(QRectF(QPoint(150,90),QSize(900, 900)).toRect());
    this->setWindowIcon(QIcon(":/FC16UIResource/Logo1.PNG"));
    //qDebug()<<"In PlayScene():, this: "<<this;

    //GameSetting and mapSetting

    {
        //Map setting
        mapSize = QSizeF(50, 50);
        pixelSize = QSizeF(18,18);

        originPoint = QPointF(0,0);
        wheelScaleRate = 0.0005;
        translateScaleRate = 1;
        autoView = true;
        singleMode = true;
        //Game setting
        exit_thread_flag = false;
        goToLoopBegin_flag = false;
        playSpeed = 1.0;
    }

    //UI initial

    {
        statusWindow = new QWidget(nullptr);
        statusWindow->hide();
        statusWindow->setWindowTitle("FC16UI-STATUS");
        statusWindow->setGeometry(QRect(QPoint(1100, 90), QSize(700,1000)));
        statusWindow->setWindowFlags (Qt::WindowMinimizeButtonHint);
        statusWindow->setWindowIcon(QIcon(":/FC16UIResource/Logo1.PNG"));




        QImage backImg;
        if(backImg.load(":/FC16UIResource/newNoTower.png"))
        {
            mapBackGround = new QLabel(this);
            mapBackGround->setGeometry(QRectF(originPoint,
                                              QSizeF(mapSize.width()*pixelSize.width(), mapSize.height()*pixelSize.height())).toRect());
            mapBackGround->setScaledContents(true);
            mapBackGround->setPixmap(QPixmap::fromImage(backImg));
            QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
            effect->setOpacity(0.5);
            mapBackGround->setGraphicsEffect(effect);
        }

        //qDebug()<<"Loaded backImg";

        briefInfo = new QTextBrowser(this);
        QGraphicsOpacityEffect* e_brief = new QGraphicsOpacityEffect(this);
        e_brief->setOpacity(0.5);
        briefInfo->setGraphicsEffect(e_brief);
        briefInfo->setFixedSize(100,120);
        briefInfo->setGeometry(QRect(QPoint(this->size().width()-briefInfo->size().width(),this->size().height()-briefInfo->size().height()), briefInfo->size()));

        goBackButton = new QPushButton("BACK", statusWindow);
        goBackButton->setGeometry(QRectF(QPointF(0, 0), QSizeF(80, 30)).toRect());
        connect(goBackButton, SIGNAL(clicked(bool)),this,
                SLOT(goBackButtonClicked()));
        goBackButton->hide();
        goBackButton->setEnabled(false);

        startGameButton = new QPushButton("START", statusWindow);
        startGameButton->setGeometry(QRectF(QPointF(100, 0), QSizeF(80, 30)).toRect());
        connect(startGameButton, SIGNAL(clicked(bool)),
                this, SLOT(startGameButtonClicked()));
        startGameButton->show();

        resumeGameButton = new QPushButton("PAUSE", statusWindow);
        resumeGameButton->setGeometry(QRectF(QPointF(200, 0), QSizeF(80, 30)).toRect());
        connect(resumeGameButton, SIGNAL(clicked(bool)),this,
                SLOT(resumeGameButtonClicked()));

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

        singleContinuousButton = new QPushButton("Continuous", statusWindow);
        singleContinuousButton->setGeometry(QRectF(QPointF(600, 0), QSizeF(80, 30)).toRect());
        connect(singleContinuousButton, SIGNAL(clicked(bool)),this,
                SLOT(singleContinuousButtonClicked()));
        singleContinuousButton->show();

        QHBoxLayout* layout1 = new QHBoxLayout;
        //layout1->addWidget(goBackButton);
        startGameButton->setFixedSize(startGameButton->size());
        resumeGameButton->setFixedSize(resumeGameButton->size());
        speedDownButton->setFixedSize(speedDownButton->size());
        speedUpButton->setFixedSize(speedUpButton->size());
        autoViewButton->setFixedSize(autoViewButton->size());
        singleContinuousButton->setFixedSize(singleContinuousButton->size());
        layout1->addWidget(startGameButton);
        layout1->addWidget(resumeGameButton);
        layout1->addWidget(speedDownButton);
        layout1->addWidget(speedUpButton);
        layout1->addWidget(autoViewButton);
        layout1->addWidget(singleContinuousButton);


        //qDebug()<<"Loaded buttons";

        roundComboBox = new QComboBox(statusWindow);
        roundComboBox->setGeometry(QRect(QPoint(600, 50), QSize(100,30)));
        connect(roundComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(goToLoopBeginCallback()));

        roundInfo = new QTextBrowser(statusWindow);
        roundInfo->setText("GAME NOT START");
        roundInfo->setGeometry(QRectF(QPointF(400, 50), QSizeF(200, 30)).toRect());
        roundInfo->show();
        roundInfo->setReadOnly(true);

        fontSizeBox = new QComboBox(statusWindow);
        fontSizeBox->setGeometry(QRect(QPoint(100,50),QSize(100,30)));
        for(int i = 1;i<=30;i++)
        {
            fontSizeBox->addItem(QString::number(i));
        }
        fontSizeBox->setCurrentIndex(7);
        connect(fontSizeBox, SIGNAL(currentIndexChanged(QString)),
                this,SLOT(changeFontSizeCallback()));
        fontSizeBox->hide();


        playStateLabel = new QLabel(statusWindow);
        playStateLabel->setGeometry(QRect(QPoint(0,50), QSize(100,30)));
        playStateLabel->setVisible(false);
        playStateLabel->setEnabled(false);
        useView = true;
        if(useView)
        {
            viewID = -1;

            viewPlayerBox = new QComboBox(statusWindow);
            viewPlayerBox->setGeometry(QRect(QPoint(280, 50), QSize(100,30)));
            connect(viewPlayerBox, SIGNAL(currentTextChanged(QString)), this, SLOT(changeViewPlayerCallback()));

            for(int i = -1;i<4;i++)
            {
                viewPlayerBox->addItem(QString::number(i));
            }

        }

        QHBoxLayout*layout2 = new QHBoxLayout;
        viewPlayerBox->setFixedSize(viewPlayerBox->size());
        layout2->addWidget(viewPlayerBox);
        roundInfo->setFixedSize(roundInfo->size());
        layout2->addWidget(roundInfo);
        roundComboBox->setFixedSize(roundComboBox->size());
        layout2->addWidget(roundComboBox);


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
        playerInfoTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

        commandInfoList = new QListWidget(statusWindow);
        commandInfoList->setGeometry(QRect(QPoint(0, 120), QSize(600, 270)));
        commandInfoList->setEditTriggers(QAbstractItemView::NoEditTriggers);

        commandInfoList->setVisible(true);
        QVBoxLayout* layout3 = new QVBoxLayout;
        commandInfoList->setMinimumHeight(100);
        layout3->addWidget(commandInfoList);
        playerInfoTable->setFixedHeight(500);
        layout3->addWidget(playerInfoTable);

        QVBoxLayout*layout = new QVBoxLayout;
        layout->addLayout(layout1);
        layout->addLayout(layout2);
        layout->addLayout(layout3);
        statusWindow->setLayout(layout);

        towerInfo = new QTextBrowser(this);
        towerInfo->setGeometry(QRect(QPoint(0, 200), QSize(200, 50)));
        towerInfo->setText("For tower");
        QGraphicsOpacityEffect* effectTowerInfo = new QGraphicsOpacityEffect();
        effectTowerInfo->setOpacity(0.5);
        towerInfo->setGraphicsEffect(effectTowerInfo);
        towerInfo->hide();
        towerInfo->setReadOnly(true);

        soldierInfo = new QTextBrowser(this);
        soldierInfo->setGeometry(QRect(QPoint(0, 0), QSize(200, 50)));
        soldierInfo->setText("For soldier");
        QGraphicsOpacityEffect* effectSoldierInfo = new QGraphicsOpacityEffect();
        effectSoldierInfo->setOpacity(0.5);
        soldierInfo->setGraphicsEffect(effectSoldierInfo);
        soldierInfo->hide();
        soldierInfo->setReadOnly(true);

        //qDebug()<<"Loaded other UI components.";
    }

    //Worker thread;
    {
        Worker* worker = new Worker;
        //worker->mainLogic = mainLogic;
        worker->playScene = this;
        worker->moveToThread(&workThread);
        connect(this, &PlayScene::runThreadStart, worker, &Worker::doWork);

        connect(&workThread, SIGNAL(finished()), &workThread, SLOT(deleteLater()));
        workThread.start();
        //worker->doWork();
        emit(runThreadStart());
    }

    //Timers
    {
        opacityTimer = new QTimer(this);
        connect(opacityTimer, SIGNAL(timeout()), this, SLOT(opacityUpdate()));

        updateGeometryTimer = new QTimer(this);
        connect(updateGeometryTimer, SIGNAL(timeout()),this, SLOT(myUpdateGeometry()));
        updateGeometryTimer->start(5);

        raiseTimer = new QTimer(this);
        connect(raiseTimer, SIGNAL(timeout()),this, SLOT(raiseWidgetss()));
        raiseTimer->start(2);

        raiseTimer2 = new QTimer(this);
        connect(raiseTimer2, SIGNAL(timeout()),this, SLOT(raiseWidgets2()));
        raiseTimer2->start(20);

        autoViewMaxStep = 0;
        autoViewCurrentStep = 0;
        autoViewInterval = 20;//msec per Operation
        focusTime = 300;//msec
        autoViewTimer = new QTimer(this);
        connect(autoViewTimer, SIGNAL(timeout()), this, SLOT(autoViewAdjust()));

        viewTimer = new QTimer(this);
        connect(viewTimer, SIGNAL(timeout()),this, SLOT(viewUpdate()));
        viewTimer->start(50);

        //qDebug()<<"Loaded timers.";
    }

    statusWindow->show();
    playState = PlayState::NOTSTART;//qDebug()<<"In PlayScene(), set playState = NOTSTART";
}
void PlayScene::gameInit()
{
    this->clearSoldiers();
    this->clearTowers();
    this->originPoint = QPoint(0,0);
    this->pixelSize = defaultPixelSize;
    playState = PlayState::NOTSTART;//qDebug()<<"In gameInit(), set playState = NOTSTART";
    singleMode = true;
    singleContinuousButton->setText("Continuous");
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

    if(playState == NOTSTART || playState == GAMESTOP)
    {
        mainLogic->clearData();
        mainLogic->LoadData();
        this->showStatusWindow();

        gameInit();
        playState = PlayState::NOPAUSERUNNING;//qDebug()<<"In startGameButtonClicked(), set playState = NOPAUSERUNNING";

        startGameButton->setText("STOP");
        resumeGameButton->setText("PAUSE");

        roundComboBox->clear();
        for(auto item: mainLogic->fileTemp.keys())
        {
            roundComboBox->addItem(QString::number(item));
        }
        roundComboBox->addItem("End");
        roundComboBox->setCurrentIndex(0);

        viewPlayerBox->setCurrentIndex(0);
    }
    else if(playState == GAMEPAUSE || playState == NOPAUSERUNNING || playState == THREADPAUSE)
    {
        playState = PlayState::GAMESTOP;//qDebug()<<"In startGameButtonClicked(), set playState = GAMESTOP";
        //Whether to clear the data.
        startGameButton->setText("START");
        resumeGameButton->setText("PAUSE");

    }
}

void PlayScene::resumeGameButtonClicked()
{
    //qDebug()<<"REsume game button clicked. playState : "<<playState;
    switch(playState)
    {
    case NOTSTART:
    {
        //This should be invisiable.
        resumeGameButton->setText("RESUME");
        break;
    }
    case GAMEPAUSE:
    { //Resume and do next action
        playState = NOPAUSERUNNING;//qDebug()<<"In resumeGameButtonClicked(), set playState = NOPAUSERUNNING";
        resumeGameButton->setText("PAUSE");

        break;

    }
    case GAMESTOP:
    {
        resumeGameButton->setText("RESUME");
        break;
    }
    case NOPAUSERUNNING:
    {
        playState = GAMEPAUSE;//qDebug()<<"In resumeGameButtonClicked(), set playState = GAMEPAUSE";
        resumeGameButton->setText("RESUME");
    }
    case THREADPAUSE:
    {
        break;
    }
    }
}

void PlayScene::speedUpButtonClicked()
{
    if(playSpeed>minimumPlaySpeed)
        playSpeed*= 0.91;
    else
        playSpeed = minimumPlaySpeed;
}

void PlayScene::speedDownButtonClicked()
{
    playSpeed*= 1.1;
}

void PlayScene::goBackButtonClicked()
{
    mainLogic->startScene->show();
    if(playState == GAMEPAUSE || playState == NOPAUSERUNNING || playState == THREADPAUSE)
        startGameButtonClicked();//To stop game
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

void PlayScene::singleContinuousButtonClicked()
{
    if(singleMode == true)
        // From single step to continuous
    {
        singleMode = false;
        singleContinuousButton->setText("Single");
        if(playState == GAMEPAUSE)
        {
            resumeGameButtonClicked();
        }

    }

    //From continuous to single step
    else
    {
        singleMode = true;

        singleContinuousButton->setText("Continuous");

    }
}

void PlayScene::changeViewPlayerCallback()
{
    //qDebug()<<"Change View";
    viewID = viewPlayerBox->currentText().toInt();
}

void PlayScene::showStatusWindow()
{
    if(statusWindow!=nullptr)
        statusWindow->show();
}

void PlayScene::myUpdateGeometry()
{
    switch(playState)
    {
    case NOTSTART:
    {
        playStateLabel->setText("NOT START");
        break;
    }
    case GAMEPAUSE:
    {
        playStateLabel->setText("GAME PAUSE");
        break;
    }
    case GAMESTOP:
    {
        playStateLabel->setText("GAME STOP");
        break;
    }
    case NOPAUSERUNNING:
    {
        playStateLabel->setText("RUNNING");
        break;
    }
    case THREADPAUSE:
    {
        playStateLabel->setText("THREAD PAUSE");
        break;
    }
    }

    if(playState == NOTSTART || playState == NOPAUSERUNNING)
        return;

    mapBackGround->setGeometry(QRectF(originPoint,QSizeF(mapSize.width()*pixelSize.width(), mapSize.height()*pixelSize.height())).toRect());
    //const_cast<QRect&>(mapBackGround->geometry()).setRight(std::max(mapBackGround->geometry().right(), 1000));
    UI::TTower*ttower;
    for(auto it = towers.begin(); it!=towers.end();it++)
    {
        ttower = mainLogic->towers[it.key()];
        try
        {
            it.value()->setGeometry(QRect(mapToGeo(ttower->m_Position+QPoint(-1,1)),(pixelSize*3).toSize()));
            it.value()->show();
        }
        catch(const std::exception&){}
    }
    UI::TSoldier*tsoldier;
    for(auto it = soldiers.begin(); it!=soldiers.end();it++)
    {
        if(mainLogic->soldiers.find(it.key())!=mainLogic->soldiers.end())
            tsoldier = mainLogic->soldiers[it.key()];
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
            //qDebug()<<"Fail to update soldier geometry in update geometry";

        }
    }

    for(MoveSoldier* move : MoveSoldier::moveToDelete)
    {    delete move;
    }
    MoveSoldier::moveToDelete.clear();

    for(auto it = towers.begin();it!=towers.end();it++)
    {
        int id = it.key();
        if(towerBars.keys().contains(id))
        {
            towerBars[id]->setGeometry(QRect(it.value()->geometry().topLeft(), QSize(pixelSize.width()*3*mainLogic->towers[id]->m_nBlood/300.0, pixelSize.height())));
            towerBars[id]->raise();
            towerBars[id]->show();
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
                //qDebug()<<"Load red succeeded";
            }
            else
            {
                //qDebug()<<"Load red failed.";
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

    mapCenterPoint = QPoint(this->size().width()/2, this->size().height()/2);

    briefInfo->setGeometry(QRect(QPoint(this->size().width()-briefInfo->size().width(), this->size().height()-briefInfo->size().height()), briefInfo->size()));

}

void PlayScene::raiseWidgetss()
{
    this->soldierInfo->raise();
    this->towerInfo->raise();


}
void PlayScene::raiseWidgets2()
{
    if(playState == NOPAUSERUNNING || playState == THREADPAUSE || playState == GAMEPAUSE)
    {

    for(auto item: towers)
    {
        item->show();
       item->raise();
    }
    }
}

void PlayScene::viewUpdate()
{
    if(useView)
    {
        for(int i = 0;i<50;i++)
        {
            for(int j = 0;j<50;j++)
            {

            }
        }
    }
}

void PlayScene::paintEvent(QPaintEvent *event)
{
    if(playState == NOTSTART || playState == GAMESTOP)
        return;

    QPainter painter1(this);
    painter1.setRenderHint(QPainter::Antialiasing, true);
    painter1.setPen(QPen(Qt::black, 3,Qt::SolidLine, Qt::RoundCap));
    painter1.setBrush(QBrush(Qt::black, Qt::SolidPattern));
    QPolygon polygon;
    if(useView && viewID!=-1)
        for(int i = 0;i<50;i++)
        {
            for(int j = 0;j<50;j++)
            {
                if(mainLogic->players[viewID]->m_VVView[i][j]==0)
                {
                    polygon.clear();

                    QPoint leftTop = mapToGeo(QPoint(i,j));
                    polygon.push_back(leftTop);
                    polygon.push_back(leftTop+QPoint(0, pixelSize.height()));
                    polygon.push_back(leftTop+QPoint(pixelSize.width(),pixelSize.height()));
                    polygon.push_back(leftTop+QPoint(pixelSize.width(),0));
                    polygon.push_back(leftTop);

                    painter1.drawPolygon(polygon);
                }
            }
        }

    if(haveFrame == 1)
    {
        for(int i = 1;i<5;i++)
        {
            painter1.drawLine(mapToGeo(QPointF(i*10.0, -1)), mapToGeo(QPointF(i*10.0,49.0)));
            painter1.drawLine(mapToGeo(QPointF(0.0, i*10.0 - 1)), mapToGeo(QPointF(50.0,i*10.0 - 1)));
        }
    }
}

void PlayScene::focusOn(const QPointF &point)
{
    if(autoView && (mapToGeo(point)-QPointF(450,450)).manhattanLength()>200)
    {
        pixelSizeDiff = autoViewPixelSize-pixelSize;
        targetFocusPoint = point;
        autoViewInterval = 20;
        autoViewMaxStep = std::max(0.0f, playSpeed*focusTime/autoViewInterval);
        autoViewCurrentStep = 0;
        autoViewTimer->start(autoViewInterval);
        //qDebug()<<"Focus on begin";
    }
    else
    {
        playState = NOPAUSERUNNING;//qDebug()<<"In focus on, too near, set playState = NOPAUSERUNNING";
    }
}
void PlayScene::focusOn(const QPoint &point)
{
    if(autoView && (mapToGeo(point)-mapCenterPoint).manhattanLength()>200)
    {
        pixelSizeDiff = autoViewPixelSize-pixelSize;
        targetFocusPoint = point;
        autoViewInterval = 20;
        autoViewMaxStep = std::min(0.0f, playSpeed*focusTime/autoViewInterval);
        autoViewCurrentStep = 0;
        autoViewTimer->start(autoViewInterval);
        //qDebug()<<"focus on begin";
    }
    else
    {
        playState = NOPAUSERUNNING;//qDebug()<<"In focus on, too near, set playState = NOPAUSERUNNING";
    }
}

void PlayScene::autoViewAdjust()
{
    if(playState == NOTSTART || playState == GAMESTOP)
        return;
    if(!autoView || autoViewCurrentStep>=autoViewMaxStep || autoViewMaxStep<=0)
    {
        autoViewTimer->stop();
        originPoint += (mapCenterPoint - mapToGeo(targetFocusPoint));
        if(playState == THREADPAUSE)
        {

            playState = NOPAUSERUNNING;//qDebug()<<"In auto view adjust finished, set playState = NOPAUSERUNNING";
        }
        return;
    }
    else
    {
        pixelSize+= pixelSizeDiff/autoViewMaxStep;
        originPoint += (mapCenterPoint - mapToGeo(targetFocusPoint))/(autoViewMaxStep-autoViewCurrentStep);
        autoViewCurrentStep++;
        //qDebug()<<"Auto view current Step is:"<<autoViewCurrentStep<<" -- maxStep is: "<<autoViewMaxStep;
        if(autoViewCurrentStep >= autoViewMaxStep && autoViewMaxStep>0)
        {
            autoViewCurrentStep = autoViewMaxStep = 0;
        }
    }
}

void PlayScene::playerUpdate(UI::TPlayer*player)
{
    //qDebug()<<"In player Update";
    if(player==nullptr)
    {
        //qDebug()<<"player == nullptr, cannot update";
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
    playState = NOPAUSERUNNING;//qDebug()<<"playerupdate finished, set playState = nopauserunning";
    //qDebug()<<"player id is "<<player->m_nID;
}

void PlayScene::towerUpdate(UI::TTower*tower)
{

    QImage img;
    QString fileName = ":/FC16UIResource/newRes/towers/tower"+QString::number(tower->m_nLevel)+QString::number(tower->getOwnerID())+".png";
    //qDebug()<<"Tower filename: "<<fileName;
    if(img.load(fileName))
    {
        QLabel* towerLabel = nullptr;
        if(towers.find(tower->m_nID)==towers.end())
        {
            towerLabel =new QLabel(this);
            towerLabel->setGeometry(QRectF(mapToGeo(QPointF(tower->m_Position.x()-1,tower->m_Position.y()-1)),
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
        mainLogic->WriteLog("Fail to load the image of soldier");
    }
    playState = NOPAUSERUNNING;//qDebug()<<"towerupdate finished, set playState = nopauserunning";
    //qDebug()<<"tower id is "<<tower->m_nID;
}

void PlayScene::soldierUpdate(UI::TSoldier*soldier)
{
    //qDebug()<<"In soldier update";
    if(soldier == nullptr)
    {
    }
    else
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
                //qDebug()<<"A new SOldierLabel created, id:"<<soldier->m_nID;

            }
            else
                soldierLabel = soldiers[soldier->m_nID];
            soldierLabel->setGeometry(QRectF(mapToGeo(soldier->m_Position),
                                             pixelSize).toRect());
            soldierLabel->setScaledContents(true);
            soldierLabel->setPixmap(QPixmap::fromImage(img));
            soldierLabel->raise();
            soldierLabel->show();
            mainLogic->WriteLog("Successfully loaded the image of soldier");

        }
        else
        {
            mainLogic->WriteLog("Fail to load the image of soldier");
        }
        mainLogic->WriteLog("SoldierFilename is "+fileName.toStdString());

        //qDebug()<<"Soldier id = "<<soldier->m_nID;
    }
    playState = NOPAUSERUNNING;//qDebug()<<"soldierupdate finished, set playState = nopauserunning";
    //qDebug()<<"Return from soldierUpdate()";
}

void PlayScene::commandUpdate(UI::Command*command)
{
    opacityLabels.clear();

    //CommandList

    try
    {
        QListWidgetItem* item = new QListWidgetItem(
                    "Round "+ QString::number(mainLogic->gameRound)+
                    ": Player-"+QString::number(command->m_pOwner->m_nID)+":"+
                    command->m_pOwner->m_strVecCommands[command]);
        commandInfoList->addItem(item);
        commandInfoList->setCurrentItem(item);
    }
    catch(const std::exception&)
    {}

    float speed = playSpeed;
    switch (command->m_nCommandType) {
    case UI::CommandType::Move:
    {

        QPointF translateVec;
        switch(command->m_nMoveDirection)
        {
        case UI::UP:
            translateVec = QPointF(0,1)*command->m_nMoveDistance;
            //qDebug()<<"MoveMove UP:";
            break;
        case UI::DOWN:
            translateVec = QPointF(0,-1)*command->m_nMoveDistance;
            //qDebug()<<"MoveMove DOWN";
            break;
        case UI::RIGHT:
            translateVec = QPointF(1,0)*command->m_nMoveDistance;
            //qDebug()<<"MoveMove RIGHT";
            break;
        case UI::LEFT:
            translateVec = QPointF(-1,0)*command->m_nMoveDistance;
            //qDebug()<<"MoveMove LEFT";
            break;
        default:
            break;
        }
        MoveSoldier* moveAction = new MoveSoldier(&command->m_pMoveSoldier->m_Position, speed*500);
        moveAction->setValue(0, command->m_pMoveSoldier->m_Position);
        moveAction->setValue(1, command->m_pMoveSoldier->m_Position+translateVec);
        if(true)//command->m_nMoveDirection == UI::LEFT)
        {
            //qDebug()<<"m_pMovePosition:"<<command->m_pMoveSoldier->m_Position;
            //qDebug()<<"Dest:"<<command->m_pMoveSoldier->m_Position+translateVec;
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
            //qDebug()<<"Fail to load upgrade.png ";thread_pause = false;
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
            //qDebug()<<"load produce FAIL";thread_pause = false;
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
            //qDebug()<<"load attack FAIL";thread_pause = false;
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
            //qDebug()<<"load victim FAIL";thread_pause = false;
        }
    }
        break;

    default:
        //qDebug()<<"Incorrect command type, thread_pause = false";thread_pause = false;
        break;
    }
}

void PlayScene::roundUpdate()
{
    roundInfo->setText("Round "+QString::number(mainLogic->gameRound));
    if(playState == THREADPAUSE)
    {
        playState = NOPAUSERUNNING;//qDebug()<<"ROUNDupdate finished, set playState = nopauserunning";
    }

    QString info;
    info = "Round: "+QString::number(mainLogic->gameRound);
    for(UI::TPlayer* player:mainLogic->players)
    {
        info= info+"\n"+"Player"+QString::number(player->m_nID)+": "+QString::number(player->m_nScore);
    }
    briefInfo->setText(info);
    //qDebug()<<"roundUpdate() finished";
}

void PlayScene::clearTowers()
{
    for(auto tower:towers)
    {    try
        {
            tower->hide();
            mainLogic->WriteLog("clear a tower");
        }
        catch(const std::exception&)
        {

        }
    }
    towers.clear();
}

void PlayScene::clearSoldiers()
{
    if(playState != THREADPAUSE)
        return;
    for(auto id: soldiers.keys())
    {    try
        {
            if(!mainLogic->soldiers.keys().contains(id))
            {
                //qDebug()<<"hide a soldier, id: "<<id;
                soldiers[id]->hide();
                delete soldiers[id];

                soldiers.remove(id);
                mainLogic->WriteLog("clear a soldier");
            }
        }
        catch(const std::exception&)
        {
            //qDebug()<<"Clear soldier raise error";

        }
    }

    if(playState == THREADPAUSE)
    {

        playState = NOPAUSERUNNING;//qDebug()<<"clear soldiers finished, set playState = nopauserunning";
    }
    //qDebug()<<"Return from clearSoldiers()";
}

void PlayScene::resumeThread()
{
    //qDebug()<<"Call resumeThread()";
    if(playState == THREADPAUSE)
    {
        playState = NOPAUSERUNNING;//qDebug()<<"Resume thread called, set playState = nopauserunning";
        opacityLabels.clear();
        opacityTimer->stop();
    }
    //qDebug()<<"Return from resumeThread()";
}
void PlayScene::resumeThreadAutoView()
{
    //qDebug()<<"Call resumeThreadAutoView()";
    resumeThread();
    //qDebug()<<"Return from resumeThreadAutoView";
}

void PlayScene::goToLoopBeginCallback()
{
    //qDebug()<<"Call goToLoopBeginCallback()";
    goToLoopBegin_flag = true;
    //qDebug()<<"Return from goToLoopBeginCallback()";
}

void PlayScene::changeFontSizeCallback()
{
    //qDebug()<<"Call changeFontSizeCallback()";
    int fontSize = fontSizeBox->currentText().toInt();
    //qDebug()<<"FontSize = "<<fontSize;
    QFont fontA("Microsoft YaHei", fontSize, 50, false);
    mainLogic->app->setFont(fontA);
    this->setFont(fontA);
    statusWindow->setFont(fontA);
    mainLogic->startScene->setFont(fontA);
    //qDebug()<<"Return from changeFontSizeCallback";
}

void PlayScene::opacityUpdate()
{
    //qDebug()<<"Call opacityUpdate()";
    for(QLabel* item: opacityLabels)
    {
        try
        {
            item->setGraphicsEffect(item->graphicsEffect());
        }
        catch(const std::exception&)
        {}

    }
    //qDebug()<<"Return from opacityUpdate()";
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
    //qDebug()<<"Call mousePressEvent()";
    if(playState == NOTSTART || playState == GAMESTOP)
        ;
    else
    {
        QPoint mousePos = QCursor::pos()-this->geometry().topLeft();
        //qDebug()<<"mousePos: "<<mousePos;
        soldierInfo->hide();
        towerInfo->hide();
        soldierInfo->clear();
        towerInfo->clear();
        for(UI::TSoldier*soldier: mainLogic->soldiers)
        {
            try
            {
                if(soldier == nullptr)
                    continue;
                if(soldiers.keys().contains(soldier->m_nID)
                        &&(soldiers[soldier->m_nID]->geometry().center()-mousePos).manhattanLength()<=(pixelSize.width()+pixelSize.height())*0.25)
                {
                    updateSoldierInfo(soldier);
                    soldierInfo->setGeometry(QRect(mousePos + QPoint(10,10), soldierInfo->size()));
                    soldierInfo->show();
                    break;
                }
            }
            catch(const std::exception&){}
        }
        for(UI::TTower*tower:mainLogic->towers)
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

    //qDebug()<<"Return from mousePressEvent()";
}

void PlayScene::updateSoldierInfo(UI::TSoldier *soldier)
{
    //qDebug()<<"Call updateSoldierInfo()";
    if(playState == NOTSTART || playState == GAMESTOP)
    {}
    else
    {
        QString content = soldierInfo->document()->toPlainText();
        content += "Soldier ID:"+QString::number(soldier->m_nID)+
                "; Owner:"+QString::number(soldier->m_pOwner->m_nID)+"; Blood:"+QString::number(soldier->m_nBlood)
                +"; Exp:" +QString::number(soldier->m_nExp)+"; "+UI::SoldierTypeEnum2Str(soldier->m_nSoldierType)+"\n";
        soldierInfo->setText(content);
    }
    //qDebug()<<"Return from updateSoldierInfo";
}
void PlayScene::updateTowerInfo(UI::TTower *tower)
{
    //qDebug()<<"Call updateTowerInfo";
    if(playState == NOTSTART || playState == GAMESTOP)
    {}
    else
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
    //qDebug()<<"Return from updateTowerInfo()";
}

void PlayScene::wheelEvent(QWheelEvent *event)
{
    //qDebug()<<"Call wheelEvent()";
    if(playState == NOTSTART || playState == GAMESTOP)
    {}
    if(autoView)
    {}
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
    //qDebug()<<"Return from wheelEvent()";
}

void PlayScene::keyPressEvent(QKeyEvent *event)
{
    //qDebug()<<"Call keyPressEvent()";
    if(playState == NOTSTART || playState == GAMESTOP)
    {}
    if(!autoView)
    {
        signed short temp =  (signed short)(event->key());
        //qDebug()<<"Temp is: "<<temp;
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
    //qDebug()<<"Return from keyPressEvent()";
}

void PlayScene::closeEvent(QCloseEvent *event)
{
    //qDebug()<<"Call closeEvent()";
    statusWindow->close();
    QApplication::quit();
    //qDebug()<<"Return from closeEvent()";
}


void Worker::doWork()
{
    //qDebug()<<"Do work START +++++++++++++++++++++";
    mainLogic = UI::MainLogic::GetInstance();
    playScene->playState = PlayScene::NOTSTART;//qDebug()<<"In doWork begin, set playState = NOTSTART";
    try
    {
        while(!playScene->exit_thread_flag)
        {

            playScene->goToLoopBegin_flag = false;
            //qDebug()<<"playState is "<<playScene->playState;
            checkState();
            //qDebug()<<"After check state, goToLoopBegin?"<<playScene->goToLoopBegin_flag;
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;

            //qDebug()<<"New Round";
            //qDebug()<<"Logic update begin";
            if(playScene->roundComboBox->currentText() == "End")
            {
                playScene->playState = PlayScene::GAMESTOP;
                qDebug()<<"GameStopped";
                continue;
            }
            int round = playScene->roundComboBox->currentText().toInt();
            //qDebug()<<"round = "<<round;
            if(!mainLogic->LogicUpdate(round))
            {
                checkState();
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
            }
            //qDebug()<<"Round : "<<playScene->roundComboBox->currentText().toInt();
            //qDebug()<<"Logic Update finished";
            checkState();
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            //qDebug()<<"Logic update end";

            //qDebug()<<"round update begin";
            checkState();
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            playScene->playState = PlayScene::THREADPAUSE;//qDebug()<<"In doWork, Before call roundUpdate, set playState = TRHEADPAUSE";
            QMetaObject::invokeMethod(playScene, "roundUpdate", Qt::QueuedConnection);
            checkState();
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            //qDebug()<<"round update end";

            //qDebug()<<"players update begin";


            for(auto player: mainLogic->players)
            {
                try
                {
                    checkState();
                    if(playScene->exit_thread_flag)
                        break;
                    if(playScene->goToLoopBegin_flag)
                        continue;
                    playScene->playState = PlayScene::THREADPAUSE;//qDebug()<<"In doWork, Before playerUpdate, set playState = TRHEADPAUSE";
                    QMetaObject::invokeMethod(playScene, "playerUpdate",
                                              Qt::QueuedConnection, Q_ARG(UI::TPlayer*, player));
                }
                catch(const std::exception&){}
                checkState();
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
            }
            //qDebug()<<"players update end";



            //qDebug()<<"towers update begin";
            for(auto tower: mainLogic->towers)
            {
                try{
                    checkState();
                    if(playScene->exit_thread_flag)
                        break;
                    if(playScene->goToLoopBegin_flag)
                        continue;
                    playScene->playState = PlayScene::THREADPAUSE;//qDebug()<<"In doWork, before calling towerUpdate(), set playState = THREADPAUSE";
                    QMetaObject::invokeMethod(playScene,"towerUpdate",
                                              Qt::QueuedConnection,Q_ARG(UI::TTower*, tower));
                }
                catch(const std::exception&){}
                checkState();
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
            }
            //qDebug()<<"towers update end";


            //qDebug()<<"Clear soldiers begin";
            checkState();
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            playScene->playState = PlayScene::THREADPAUSE;//qDebug()<<"In dowork, before calling clearSoldiers(), set playState = THREADPAUSE";
            QMetaObject::invokeMethod(playScene,"clearSoldiers",
                                      Qt::QueuedConnection);
            checkState();
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            //qDebug()<<"Clear soldiers end";

            //qDebug()<<"soldiers update begin";
            for(auto soldier: mainLogic->soldiers)
            {
                try
                {
                    checkState();
                    if(playScene->exit_thread_flag)
                        break;
                    if(playScene->goToLoopBegin_flag)
                        continue;
                    playScene->playState = PlayScene::THREADPAUSE;//qDebug()<<"In doWork(), before calling soldierUpdate, set playState = THREADPAUSE";
                    QMetaObject::invokeMethod(playScene,"soldierUpdate",
                                              Qt::QueuedConnection, Q_ARG(UI::TSoldier*, soldier));

                }
                catch(const std::exception&)
                {
                }
                checkState();
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
            }
            //qDebug()<<"soldiers update end";

            //qDebug()<<"Have pause?";
            if(playScene->singleMode&&playScene->playState == PlayScene::NOPAUSERUNNING)
            {
                //qDebug()<<"Pause begin";
                try
                {
                    checkState();
                    if(playScene->exit_thread_flag)
                        break;
                    if(playScene->goToLoopBegin_flag)
                        continue;

                    playScene->playState = PlayScene::GAMEPAUSE;//qDebug()<<"In doWork, single mode&&have pause, so set playState = GAMEPAUSE";
                    playScene->resumeGameButton->setText("RESUME");
                }
                catch(const std::exception&){}
                checkState();
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
                //qDebug()<<"Pause end";
            }
            else
            {
                //qDebug()<<"No pause";
            }

            //qDebug()<<"update view begin";
            checkState();
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            playScene->playState = PlayScene::THREADPAUSE;//qDebug()<<"In doWork, Before calling updateView(), set playState = THREADPAUSE";
            //qDebug()<<"Updating view";
            mainLogic->updateView();
            checkState();
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            int currID = 0;
            //qDebug()<<"commands update begin";
            for(UI::Command* command: mainLogic->commands)
            {
                if(command->m_pOwner->m_nID> currID)
                {
                    //qDebug()<<"update view begin";
                    currID = command->m_pOwner->m_nID;
                    checkState();
                    if(playScene->exit_thread_flag)
                        break;
                    if(playScene->goToLoopBegin_flag)
                        continue;
                    playScene->playState = PlayScene::THREADPAUSE;//qDebug()<<"In doWork, updateViewer()2, set playState = THREADPAUSE";
                    mainLogic->updateView();
                    //qDebug()<<"update view 2";
                    checkState();
                    if(playScene->exit_thread_flag)
                        break;
                    if(playScene->goToLoopBegin_flag)
                        continue;
                    //qDebug()<<"Update view end";

                }
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
                checkState();
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
                playScene->playState = PlayScene::THREADPAUSE;//qDebug()<<"In doWork, before calling focusOn(), set playState = THREADPAUSE";
                QMetaObject::invokeMethod(playScene, "focusOn", Qt::QueuedConnection, Q_ARG(QPointF, focusPos));
                checkState();
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
                //qDebug()<<"A Command update begin";
                try
                {
                    checkState();
                    if(playScene->exit_thread_flag)
                        break;
                    if(playScene->goToLoopBegin_flag)
                        continue;
                    playScene->playState = PlayScene::THREADPAUSE;//qDebug()<<"In doWork, before calling commandUpdate(), set playState = THREADPAUSE";
                    QMetaObject::invokeMethod(playScene,"commandUpdate",
                                              Qt::QueuedConnection, Q_ARG(UI::Command*, command));
                }
                catch(const std::exception&)
                {
                }
                checkState();
                if(playScene->exit_thread_flag)
                    break;
                if(playScene->goToLoopBegin_flag)
                    continue;
                //qDebug()<<"A command update end";

                //qDebug()<<"Have pause?";
                if(playScene->singleMode && playScene->playState == PlayScene::NOPAUSERUNNING)
                {
                    //qDebug()<<"Pause begin";
                    checkState();
                    if(playScene->exit_thread_flag)
                        break;
                    if(playScene->goToLoopBegin_flag)
                        continue;
                    playScene->playState = PlayScene::GAMEPAUSE;//qDebug()<<"In dowork, singleMode && have pause, set playState = GAMEPAUSE";
                    playScene->resumeGameButton->setText("RESUME");
                    checkState();
                    if(playScene->exit_thread_flag)
                        break;
                    if(playScene->goToLoopBegin_flag)
                        continue;
                    //qDebug()<<"Pause end";
                }
                else
                {
                    //qDebug()<<"No pause";
                }

            }
            //qDebug()<<"Commands update end";
            checkState();
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            playScene->playState = PlayScene::THREADPAUSE;//qDebug()<<"In doWork(), before calling updateView3, set playState = THREADPAUSE";
            mainLogic->updateView();
            checkState();
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;

            if(!playScene->singleMode)
                QThread::currentThread()->msleep(playScene->playSpeed*500);
            checkState();
            if(playScene->exit_thread_flag)
                break;
            if(playScene->goToLoopBegin_flag)
                continue;
            if(playScene->playState == PlayScene::NOPAUSERUNNING)
            {
                if(playScene->roundComboBox->count() == playScene->roundComboBox->maxCount())
                {
                    if(playScene->playState == PlayScene::NOPAUSERUNNING
                            || playScene->playState == PlayScene::GAMEPAUSE
                            ||  playScene->playState == PlayScene::THREADPAUSE)
                    {
                        playScene->playState = PlayScene::GAMESTOP;//qDebug()<<"In doWork, GameFinished, set playState = GAMESTOP";

                    }
                }
                else
                {
                    playScene->roundComboBox->setCurrentIndex(playScene->roundComboBox->currentIndex()+1);
                }
            }

        }
    }
    catch(const std::exception&){}
    //qDebug()<<"Break from doWork, exit_thread_flag = "<<playScene->exit_thread_flag;
}

//Return false to exit
void Worker::checkState()
{
    //qDebug()<<"in check state. PlayState is "<<playScene->playState;
    if(playScene->playState == PlayScene::NOTSTART
            || playScene->playState == PlayScene::GAMESTOP)
    {

        playScene->goToLoopBegin_flag = true;
        return;
    }
    if(playScene->exit_thread_flag)
        return;
    //qDebug()<<"Not return in first check";
    while(playScene->playState == PlayScene::THREADPAUSE
          || playScene->playState == PlayScene::GAMEPAUSE)
    {

        if(playScene->playState == PlayScene::NOTSTART
                || playScene->playState == PlayScene::GAMESTOP)
        {

            playScene->goToLoopBegin_flag = true;
            break;
        }
        if(playScene->exit_thread_flag)
            break;
    };
    //qDebug()<<"Return from check state. PlayState is "<<playScene->playState;
}
QSet<MoveSoldier*> MoveSoldier::moveToDelete;


MoveSoldier::MoveSoldier(QPointF*moveObject, int duration)
{
    this->mainLogic = UI::MainLogic::GetInstance();
    this->playScene = mainLogic->playScene;
    this->moveObject = moveObject;
    this->duration = duration;
    this->interval = 20;//default
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
    //qDebug()<<"Call startMove()";
    if(playScene->playState == PlayScene::NOPAUSERUNNING)
    {
        playScene->playState = PlayScene::THREADPAUSE;//qDebug()<<"In startMove, set playState = THREADPAUSE";
    }
    this->currStep = 0;
    //qDebug()<<"From "<<values[0]<<" to "<<values[values.size()-1];
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
    if(currStep>=maxStep )
    {
        *moveObject = values[values.size()-1].second;
        moveTimer->stop();
        moveToDelete.insert(this);
        if(playScene->playState == PlayScene::THREADPAUSE)
        {
            playScene->playState = PlayScene::NOPAUSERUNNING;//qDebug()<<"In update value finished, set playState = NOPAUSERUNNING";
        }
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
