#include "MainLogic.h"
#include "../UI/PlayScene.h"

#include <windows.h>
#include <thread>
#include <chrono>
#include <QDebug>
#include <iostream>
#include <fstream>
#include <string>


namespace UI
{

MainLogic* MainLogic::m_pInstance = nullptr;

MainLogic::MainLogic()
{


    logFileStream.open("log.txt", std::ios::out);
    logFileStream << "Starting..\n";
    playerAlive = 4;
    gameRound = 0;
    MapSize = 50;


    for (int i = 0; i < PLAYER_NUM; i++)
    {
        players.push_back(new TPlayer());
        players[i]->m_nID = i;

    }
    for (int i = 0; i < TOWER_NUM; i++)
        towers.push_back(new TTower());
    startScene  = new StartScene();
    playScene = nullptr;
    startScene->show();
}


MainLogic::~MainLogic()
{
    logFileStream << "MainLogic destruction..\n";
    logFileStream.close();
}


void MainLogic::LoadData()
{
    if (!loadFileName.size())
    {
        MainLogic::GetInstance()->WriteLog("File name is empty");
        qDebug()<<"Load File name empty";
        return;
    }

    fileTemp.clear();
    std::ifstream fin(loadFileName.toStdString(), std::ios::in);
    std::string stdTemp;
    int cnt = 0;
    QVector<QString>fileVec;
    while(!fin.eof())
    {
       std::getline(fin,stdTemp);
       cnt++;
       if(cnt%100==0)
       {
           qDebug()<<cnt<<"  "<<QString::fromStdString(stdTemp);
       }
       fileVec.push_back(QString::fromStdString(stdTemp));
    }
    fin.close();
    QString tempString;

    int roundTemp = 0;
    bool tempMark=false;
    for(int i = 0;i<fileVec.size();)
    {
        QString lineTemp = fileVec[i++];
        //qDebug()<<lineTemp;
        if(lineTemp == "RoundEnd")
        {
            fileTemp[roundTemp] = tempString;
            qDebug()<<"RoundTemp: "<<roundTemp;
            tempString.clear();
        }
        else
        {

            if(tempMark)
            {
                roundTemp = lineTemp.toInt();
                tempMark = false;
            }
            if(lineTemp.split(" ").at(0)=="RoundBegin")
            {
                tempMark = true;
                tempString.clear();
            }

            tempString += lineTemp+"\n";
        }
    }



    if(playScene==nullptr)
    {
        qDebug()<<"PlayScene is nullptr";
        playScene = new PlayScene();
        playScene->mainLogic = this;
        qDebug()<<"In MainLogic, playScene is constructed.";
    }
    playScene->show();
    startScene->hide();
}

bool MainLogic::LogicUpdate(const int&round)
{
    //qDebug()<<"In Logic Update , round: "<<round;
    if(fileTemp.keys().contains(round))
    {
        //qDebug()<<"The Information we get is ";
        //qDebug()<<fileTemp[round];
        QStringList stringList = fileTemp[round].split("\n");
        int listSize = stringList.size()-1;
        //qDebug()<<"list size = "<<listSize;
        for(int lineCnt = 0;lineCnt<listSize;)
        {
            try
            {
                std::string mark_type;
                int mark_lines;
                //qDebug()<<"Analysing in top, lineCnt = "<<lineCnt;
                if(lineCnt >= listSize)
                    break;
                QString stringLine = stringList[lineCnt++];
                mark_type = stringLine.split(" ").at(0).toStdString();
                mark_lines = stringLine.split(" ").at(1).toInt();
                if(mark_type =="RoundBegin")
                {
                    if (mark_lines != 1)
                    {
                        //This should be an error;
                    }
                    //qDebug()<<"Reading in Round begin, lineCnt = "<<lineCnt;
                    gameRound = stringList[lineCnt++].toInt();
                    WriteLog("Game round: " + std::to_string(gameRound));
                }
                else if (mark_type == "PlayerAlive")
                {
                    if (mark_lines != 1)
                    {}
                    //qDebug()<<"Reading in PlayerAlive, lineCnt = "<<lineCnt;
                    playerAlive = stringList[lineCnt++].toInt();

                }
                else if (mark_type == "PlayerInfo")
                {
                    if (players.size() < PLAYER_NUM)
                    {
                        clearData();
                        initData();

                    }
                    if (mark_lines > PLAYER_NUM)
                    {}

                    std::string temp_str;
                    int id;

                    //qDebug()<<"Reading in PlayerInfo, lineCnt = "<<lineCnt;

                    for (int i = 0; i < mark_lines; i++)
                    {
                        //qDebug()<<"Reading in Player, lineCnt = "<<lineCnt;
                        QString stringLine = stringList[lineCnt++];
                        temp_str = stringLine.split(" ").at(0).toStdString();
                        id = stringLine.split(" ").at(1).toInt();

                        //qDebug()<<"Generate an player, lineCnt = "<<lineCnt;
                        players[id]->Generate(stringList[lineCnt++].toStdString());
                        players[id]->m_nID = id;


                        //WriteLog("Player " + std::to_string(id));
                        //WriteInfo(players[id]);
                    }

                }
                else if (mark_type == "TowerInfo")
                {
                    for (int i = 0; i < mark_lines; i++)
                    {
                        //qDebug()<<"Reading a tower, lineCnt = "<<lineCnt;
                        towers[i]->Generate(stringList[lineCnt++].toStdString());
                        towers[i]->SetVec2Position(i);
                        //WriteInfo(towers[i]);
                    }
                }
                else if (mark_type == "SoldierInfo")
                {
                    int maxSoldierID = maxKey(soldiers);

                    //Clear old soldiers
                    for (auto item : soldiers)
                    {
                        delete item;
                    }
                    soldiers.clear();

                    //Generate new soldiers
                    for (int i = 0; i < mark_lines; i++)
                    {
                        try
                        {
                            TSoldier* newSoldier = new TSoldier();
                            //qDebug()<<"Reading a soldier, lineCnt = "<<lineCnt;
                            newSoldier->Generate(stringList[lineCnt++].toStdString());
                            soldiers[newSoldier->m_nID]= newSoldier;

                            if (newSoldier->m_nID > maxSoldierID)
                                newSoldier->m_bFreshman = true;
                            //WriteInfo(newSoldier);
                        }
                        catch (const std::exception&)
                        {
                            MainLogic::GetInstance()->WriteLog("Error generating soldiers");
                        }
                    }


                }

                //Generate Command
                else if (mark_type == "CommandsInfo")
                {
                    std::string mark_type_command, temp_str;
                    int mark_player_id, mark_commands_lines, mark_info_command;

                    for (auto item : commands)
                        delete item;
                    commands.clear();
                    UI::Command* newCommand = nullptr;


                    int id = 0;
                    for (int i = 0; i < mark_lines; i++)
                    {
                        //qDebug()<<"Reading one player commands, lineCnt = "<<lineCnt;
                        QString strLineTest = stringList[lineCnt];
                        if(strLineTest.split(" ").size()>10)
                        {
                            int mapSize = 50;
                            for(int ii = 0 ;ii<mapSize;ii++)
                            {
                                QString stringLine2 = stringList[lineCnt++];
                                QList<QString> numberStrs = stringLine2.split(" ");
                                for(int jj = 0;jj<mapSize;jj++)
                                {
                                    players[id]->m_VVView[jj][49-ii] = numberStrs.at(jj).toInt();
                                }
                            }
                            qDebug()<<"Successfully update viewMap for player "<<id;
                        }
                        else
                        {
                             for(int iii = 0;iii<4;iii++)
                             {
                                 for(int jjj= 0 ;jjj<50;jjj++)
                                     for(int kkk = 0;kkk<50;kkk++)
                                         players[iii]->m_VVView[jjj][kkk] = 1;
                             }

                        }
                        id++;


                        std::string strLine = stringList[lineCnt].toStdString();
                        if(QString::fromStdString(strLine).split(" ").size()!=2)
                            continue;
                        else
                            lineCnt++;
                        {
                            std::stringstream strstrm(strLine);
                            strstrm >> mark_player_id >> mark_commands_lines;
                        }
                        for (int j = 0; j < mark_commands_lines; j++)
                        {
                            //qDebug()<<"Reading a command, lineCnt = "<<lineCnt;
                            std::string strLine = stringList[lineCnt++].toStdString();
                            std::stringstream strstrm(strLine);
                            strstrm >> mark_type_command;

                            newCommand = new Command();

                            try
                            {
                                newCommand->m_pOwner = players[mark_player_id];
                                newCommand->m_pOwner->m_strVecCommands[newCommand]=QString::fromStdString(strLine);
                                if (mark_type_command == "Move")
                                {
                                    newCommand->m_nCommandType = UI::CommandType::Move;

                                    //SoldierID
                                    strstrm >> mark_type_command >> mark_info_command;
                                    newCommand->m_pMoveSoldier = soldiers[mark_info_command];
                                    //Direction
                                    strstrm >> mark_type_command >> temp_str;
                                    newCommand->m_nMoveDirection = moveDirStr2Enum(temp_str);
                                    //Distance
                                    strstrm >> mark_type_command >> mark_info_command;
                                    newCommand->m_nMoveDistance = mark_info_command;

                                    newCommand->m_pMoveSoldier->m_strctSoldierMove = UI::SoldierMoveType{ true, newCommand->m_nMoveDirection, newCommand->m_nMoveDistance };

                                }
                                else if (mark_type_command == "Attack")
                                {
                                    newCommand->m_nCommandType = UI::CommandType::Attack;
                                    //SoldierID
                                    strstrm >> mark_type_command >> mark_info_command;
                                    newCommand->m_pAttackObject = soldiers[mark_info_command];
                                    //VicType
                                    strstrm >> mark_type_command >> temp_str;
                                    strstrm >> mark_type_command >> mark_info_command;
                                    if (temp_str == "Soldier")
                                        newCommand->m_pVictimObject = soldiers[mark_info_command];
                                    else if (temp_str == "Tower")
                                        newCommand->m_pVictimObject = towers[mark_info_command];
                                    newCommand->m_pAttackObject->m_pVictim = newCommand->m_pVictimObject;
                                }
                                else if (mark_type_command == "Upgrade")
                                {
                                    newCommand->m_nCommandType = UI::CommandType::Upgrade;
                                    strstrm >> temp_str >> mark_info_command;
                                    newCommand->m_pUpgradeTower = towers[mark_info_command];

                                }
                                else if (mark_type_command == "Produce")
                                {
                                    newCommand->m_nCommandType = UI::CommandType::Produce;

                                    strstrm >> temp_str >> mark_info_command;
                                    newCommand->m_pProduceTower = towers[mark_info_command];
                                    strstrm >> temp_str >> mark_type_command;
                                    newCommand->m_nProduceSoldierType = UI::SoldierTypeStr2Enum(mark_type_command);
                                }
                            }

                            catch (const std::exception&)
                            {
                                WriteLog("Command read error");

                                delete newCommand;
                                newCommand = nullptr;
                            }
                            if (newCommand != nullptr)
                            {
                                MainLogic::GetInstance()->commands.push_back(newCommand);
                                newCommand->m_pOwner->m_vecCommands.push_back(newCommand);

                                //WriteInfo(newCommand);
                            }
                        }
                    }
                }
            }
            catch (std::exception&)
            {
                MainLogic::GetInstance()->WriteLog("Logic Update error!");
            }
            if(lineCnt>=listSize )
                break;
        }
        //qDebug()<<"Break the for loop";
        return true;
    }
    else
    {
        return false;
    }
}

void MainLogic::updateView()
{
    for(int i = 0;i<4;i++)
        for(int j = 0 ;j<50;j++)
            for(int k = 0;k<50;k++)
            {
                players[i]->m_VVView[j][k] = 0;
    }
    for(int id = 0;id<4;id++)
    {
        for(int i = 0;i<towers.size();i++)
        {
            if(true)
            {//改一下塔的视野公开(已改)
                int x = towers[i]->m_Position.x();
                int y = towers[i]->m_Position.y();
                int level = towers[i]->m_nLevel;
                if (towers[i]->m_pOwner == nullptr) {
                    level = 1;
                }
                for(int j=x-2-level;j<x+3+level;j++)
                {
                    for(int k=y-2-level;k<y+3+level;k++)
                    {
                        if(j>=0&&j<50&&k>=0&&k<50)
                        {
                            players[id]->m_VVView[j][k] = 1;
                        }
                    }
                }
            }
        }
        for(auto i: soldiers.keys())
        {
            if(soldiers[i] == nullptr)
                continue;
            if(soldiers[i]->m_pOwner == players[id]){
                int x = soldiers[i]->m_Position.x();
                int y = soldiers[i]->m_Position.y();
                switch (soldiers[i]->m_nSoldierType)
                {
                case LightInfantry:
                    for(int j=x-3;j<x+4;j++)
                    {
                        for(int k=y-3;k<y+4;k++)
                        {
                            if(j>=0&&j<MapSize&&k>=0&&k<MapSize)
                            {
                                players[id]->m_VVView[j][k] = 1;
                            }
                        }
                    }
                    break;
                case LightArcher:
                    for(int j=x-3;j<x+4;j++)
                    {
                        for(int k=y-3;k<y+4;k++)
                        {
                            if(j>=0&&j<MapSize&&k>=0&&k<MapSize)
                            {
                                players[id]->m_VVView[j][k] = 1;
                            }
                        }
                    }
                    break;
                case LightKnight:
                    for(int j=x-5;j<x+6;j++)
                    {
                        for(int k=y-5;k<y+6;k++)
                        {
                            if(j>=0&&j<MapSize&&k>=0&&k<MapSize)
                            {
                                players[id]->m_VVView[j][k] = 1;
                            }
                        }
                    }
                    break;
                case Mangonel:
                    for(int j=x-2;j<x+3;j++)
                    {
                        for(int k=y-2;k<y+3;k++)
                        {
                            if(j>=0&&j<MapSize&&k>=0&&k<MapSize)
                            {
                                players[id]->m_VVView[j][k] = 1;
                            }
                        }
                    }
                    break;
                case HeavyInfantry:
                    for(int j=x-3;j<x+4;j++)
                    {
                        for(int k=y-3;k<y+4;k++)
                        {
                            if(j>=0&&j<MapSize&&k>=0&&k<MapSize)
                            {
                                players[id]->m_VVView[j][k] = 1;
                            }
                        }
                    }
                    break;
                case HeavyArcher:
                    for(int j=x-3;j<x+4;j++)
                    {
                        for(int k=y-3;k<y+4;k++)
                        {
                            if(j>=0&&j<MapSize&&k>=0&&k<MapSize)
                            {
                                players[id]->m_VVView[j][k] = 1;
                            }
                        }
                    }
                    break;
                case HeavyKnight:
                    for(int j=x-4;j<x+5;j++)
                    {
                        for(int k=y-4;k<y+5;k++)
                        {
                            if(j>=0&&j<MapSize&&k>=0&&k<MapSize)
                            {
                                players[id]->m_VVView[j][k] = 1;
                            }
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }


   if(playScene->playState == PlayScene::THREADPAUSE)
   {
       playScene->playState = PlayScene::NOPAUSERUNNING;
   }
}

void MainLogic::clearData()
{
    WriteLog("ClearData..");


    for (UI::TPlayer* item : players)
    {
        for(int j = 0;j<50;j++)
            for(int k = 0;k<50;k++)
                item->m_VVView[j][k] = 1;
        delete item;
    }
    players.clear();
    for (auto item : towers)
        delete item;
    towers.clear();
    for (auto item : soldiers)
        delete item;
    soldiers.clear();
    for (auto item : commands)
        delete item;
    commands.clear();

    gameRound = 0;
}

void MainLogic::initData()
{
    for (int i = 0; i < PLAYER_NUM; i++)
        players.push_back(new TPlayer());
    for (int i = 0; i < TOWER_NUM; i++)
        towers.push_back(new TTower());
    WriteLog("Initializing data...");
}

MainLogic * MainLogic::GetInstance()
{
    return m_pInstance;
}
void MainLogic::DestroyInstance()
{
    // No delete for stack object
    // REVIEW NEEDED.
}

void MainLogic::WriteLog(QString& message)
{
    logFileStream << message.toStdString() << std::endl;
}

void MainLogic::WriteLog(const std::string& message)
{
    logFileStream << message << std::endl;
}


void MainLogic::WriteInfo(const UI::TPlayer*player)
{
    logFileStream << "Rank " << player->m_nRank << " Score " << player->m_nScore
                  << " KillNum " << player->m_nKillNum << " TowerNum " << player->m_nTowerNum
                  << " SurvivalRound " << player->m_nSurvivalRound << " SoldierNum " << player->m_nSoldierNum
                  << " Resource " << player->m_nResource << " MaxPopulation " << player->m_nMaxPopulation
                  << " Population " << player->m_nPopulation << "\n";
}

void MainLogic::WriteInfo(const UI::TSoldier*soldier)
{
    logFileStream << "SoldierID " << soldier->m_nID << " Owner " << soldier->getOwnerID()
                  << " Type " << soldier->m_nSoldierType << " Level " << soldier->m_nLevel << " Blood " << soldier->m_nBlood
                  << " position (" << soldier->m_Position.x()<<", "<<soldier->m_Position.y() << "\n";
}

void MainLogic::WriteInfo(const UI::TTower*tower)
{
    logFileStream << "TowerID " << tower->m_nID << " Owner " << tower->getOwnerID()
                  << " Level " << tower->m_nLevel << " Blood " << tower->m_nBlood
                  << " Recruiting " << tower->m_bRecruiting << " RecruitingRound " << tower->m_nRecruitingRound
                  << " RecruitingType " << tower->m_nRecruitingType<<"\n";
}

void MainLogic::WriteInfo(const UI::Command*command)
{
    switch (command->m_nCommandType)
    {
    case UI::CommandType::Attack:
        logFileStream << "Attack " << command->m_pAttackObject->m_nID << " type " << command->m_pVictimObject->m_nUIType << " " << command->m_pVictimObject;
        break;
    case UI::CommandType::Move:
        logFileStream << "Move " << command->m_pMoveSoldier->m_nID << " Direction " << command->m_nMoveDirection << " Distance " << command->m_nMoveDistance;
        break;
    case UI::CommandType::Produce:
        logFileStream << "Produce " << "towerID " << command->m_pProduceTower->m_nID << " SoldierType " << command->m_nProduceSoldierType;
        break;
    case UI::CommandType::Upgrade:
        logFileStream << "Upgrade " << " TowerID " << command->m_pUpgradeTower->m_nID;
        break;
    default:
        break;
    }
    logFileStream << "\n";
}
}

