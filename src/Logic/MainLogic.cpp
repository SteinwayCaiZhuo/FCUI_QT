#include "MainLogic.h"
#include "../UI/PlayScene.h"

#include <windows.h>
#include <thread>
#include <chrono>
#include <QDebug>



namespace UI
{

MainLogic* MainLogic::m_pInstance = nullptr;

MainLogic::MainLogic()
{

    gameState = GameState::GAME_NOT_START;
    mapFile = "gameMap.png";
    loadFileName = "result.txt";
    logFileStream.open("log.txt", std::ios::out);
    logFileStream << "Starting..\n";
    playerAlive = 4;
    speed = 1;//Default setting
    gameRound = 0;


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
    if(startScene!=nullptr)
        delete startScene;
    if(playScene != nullptr)
        delete playScene;
}





void MainLogic::GameStart()
{
    //if (gameState != GameState::GAME_NOT_START)
    //	return;
    //gameRound = 0;
    //gameState = GameState::GAME_RUNNING;
    MainLogic::clearData();
    LoadData();
    playScene->showStatusWindow();
}

void MainLogic::GameLoop()
{

}

void MainLogic::GameOver()
{
    if (gameState == GameState::GAME_NOT_START)
    {
        return;
    }

    else
    {
        gameRound = 0;
        gameState = GameState::GAME_NOT_START;
        MainLogic::clearData();
    }

}

void MainLogic::GamePause()
{
    if (gameState != GameState::GAME_RUNNING)
        return;
    else
    {
        gameState = GameState::GAME_PAUSE;
    }
}

void MainLogic::GameResume()
{
    if (gameState != GameState::GAME_PAUSE)
        return;
    else
    {
        gameState = GameState::GAME_RUNNING;
    }
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
    QFile myFile(loadFileName);
    if (!myFile.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug()<<"File open error";
    QTextStream tempStream(&myFile);
    QString tempString;
    QString lineTemp;
    int roundTemp = 0;
    bool tempMark=false;
    while(!myFile.atEnd())
    {
        lineTemp = tempStream.readLine();
        if(lineTemp == "RoundEnd")
        {
            fileTemp[roundTemp] = tempString;
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
    myFile.close();

    if (ifsGameResult.is_open())
        ifsGameResult.close();
    ifsGameResult.open(loadFileName.toStdString(), std::ios::in);
    if (!ifsGameResult.is_open()) return;
    MainLogic::GetInstance()->WriteLog("Succesfully loaded the file");
    qDebug()<<"Successfully loaded the file";
    //ifsGameResult.close();
    if(playScene==nullptr)
    {
        qDebug()<<"PlayScene is nullptr";
        playScene = new PlayScene();
        qDebug()<<"In MainLogic, playScene is constructed.";
    }
    playScene->show();
    startScene->hide();
}

bool MainLogic::LogicUpdate()
{

    std::string strLine;
    std::string mark_type;
    int mark_lines;
    std::stringstream strstrm("test");
    while (true)
    {
        getline(ifsGameResult, strLine);

        //End of game
        if (strLine.empty())
            return false;

        MyClear(strstrm);
        strstrm << strLine;
        strstrm >> mark_type >> mark_lines;
        MyClear(strstrm);
        if (mark_type.empty() || mark_type == "RoundEnd")
            break;
        else
        {
            try
            {
                parseLines(mark_type, mark_lines);
            }
            catch (std::exception&)
            {
                MainLogic::GetInstance()->WriteLog("Logic Update error!");
            }
        }

    }
    return true;
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
                                    players[id]->m_VVView[ii][jj] = numberStrs.at(jj).toInt();
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


                        std::string strLine = stringList[lineCnt++].toStdString();
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

void MainLogic::parseLines(const std::string&mark_type, const int&mark_lines)
{
    std::string strLine;
    std::stringstream strstrm("init");

    if (mark_type == "RoundBegin")
    {
        if (mark_lines != 1)
        {

        }
        getline(ifsGameResult, strLine);
        MyClear(strstrm);
        strstrm << strLine;
        strstrm >> gameRound;
        MyClear(strstrm);
        WriteLog("Game round: " + std::to_string(gameRound));
    }
    else if (mark_type == "PlayerAlive")
    {
        if (mark_lines != 1)
        {}
        getline(ifsGameResult, strLine);
        MyClear(strstrm);
        strstrm << strLine;
        strstrm >> playerAlive;
        MyClear(strstrm);

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

        for (int i = 0; i < mark_lines; i++)
        {
            getline(ifsGameResult, strLine);
            MyClear(strstrm);
            strstrm << strLine;
            strstrm >> temp_str >> id;
            MyClear(strstrm);
            getline(ifsGameResult, strLine);

            players[id]->Generate(strLine);
            players[id]->m_nID = id;

            //WriteLog("Player " + std::to_string(id));
            //WriteInfo(players[id]);
        }

    }
    else if (mark_type == "TowerInfo")
    {
        for (int i = 0; i < mark_lines; i++)
        {
            getline(ifsGameResult, strLine);
            towers[i]->Generate(strLine);
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
            getline(ifsGameResult, strLine);
            try
            {
                TSoldier* newSoldier = new TSoldier();
                newSoldier->Generate(strLine);
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
        for (int i = 0; i < mark_lines; i++)
        {
            getline(ifsGameResult, strLine);
            {
                std::stringstream strstrm(strLine);
                strstrm >> mark_player_id >> mark_commands_lines;
            }
            for (int j = 0; j < mark_commands_lines; j++)
            {
                getline(ifsGameResult, strLine);

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
        MyClear(strstrm);
    }
}

void MainLogic::clearData()
{
    WriteLog("ClearData..");
    //loadFileName = "";

    for (auto item : players)
        delete item;
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

