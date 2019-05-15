#include <QApplication>
#include <QStyle>
#include <QFile>
#include <QTextStream>
#include<QString>
#include <QDebug>
#include <string>
#include <fstream>
#include <iostream>
#include <QPoint>
#include "src/Logic/MainLogic.h"

Q_DECLARE_METATYPE(UI::TSoldier*)
Q_DECLARE_METATYPE(UI::TPlayer*)
Q_DECLARE_METATYPE(UI::Command*)
Q_DECLARE_METATYPE(UI::TTower*)

void loadSetting(QApplication*a);

int haveFrame = 0; //
QPoint mapCenterPoint = QPoint(450,450);

int main(int argc, char *argv[])
{
    qRegisterMetaType<UI::TPlayer*>("UI::TPlayer*");
    qRegisterMetaType<UI::TTower*>("UI::TTower*");
    qRegisterMetaType<UI::TSoldier*>("UI::TSoldier*");
    qRegisterMetaType<UI::Command*>("UI::Command*");
    QApplication a(argc, argv);
    QFile file(":/FC16UIResource/steinway.qss");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet= filetext.readAll();
    a.setStyleSheet(stylesheet);
    a.setFont(QFont("Microsoft YaHei", 8, 50, false));

    loadSetting(&a);

    UI::MainLogic mainlogic;
    UI::MainLogic::m_pInstance = &mainlogic;
    UI::MainLogic::m_pInstance->app = &a;

    return a.exec();
}

void loadSetting(QApplication*a)
{
    std::ifstream settingFile("C:/Users/CaiZhuo/Desktop/FC16UI1.2.3/settings.txt");
    if(!settingFile.is_open())
    {
        qDebug()<<"File open error";

    }

    std::string temp;
    QString lineTemp, stringTemp;
    while(std::getline(settingFile, temp))
    {
        lineTemp = QString::fromStdString(temp);
        qDebug()<<lineTemp;
        stringTemp = lineTemp.split(" ").at(0);
        if(stringTemp.toLower() == "fontsize")
        {
            int fontSize = lineTemp.split(" ").at(1).toInt();
            qDebug()<<"font size = "<<fontSize;
            a->setFont(QFont("Microsoft YaHei", fontSize, 50, false));
        }

        if(stringTemp.toLower() == "haveframe")
        {
            haveFrame = lineTemp.split(" ").at(1).toInt();
            qDebug()<<"haveFrame = "<<haveFrame;
        }

        if(stringTemp.toLower() == "mapcenterpoint")
        {
            mapCenterPoint.setX(lineTemp.split(" ").at(1).toInt());
            mapCenterPoint.setY(lineTemp.split(" ").at(2).toInt());
        }
    }
    settingFile.close();
}
