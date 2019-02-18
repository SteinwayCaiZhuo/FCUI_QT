#include <QApplication>
#include <QStyle>
#include <QFile>
#include <QTextStream>
#include<QString>
#include "src/Logic/MainLogic.h"

Q_DECLARE_METATYPE(UI::TSoldier*)
Q_DECLARE_METATYPE(UI::TPlayer*)
Q_DECLARE_METATYPE(UI::Command*)
Q_DECLARE_METATYPE(UI::TTower*)
int main(int argc, char *argv[])
{
    qRegisterMetaType<UI::TPlayer*>("UI::TPlayer*");
    qRegisterMetaType<UI::TTower*>("UI::TTower*");
    qRegisterMetaType<UI::TSoldier*>("UI::TSoldier*");
    qRegisterMetaType<UI::Command*>("UI::Command*");
    QApplication a(argc, argv);
    QFile file(":/FC16UIResource/csdn.qss");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet= filetext.readAll();
    //a.setStyleSheet(stylesheet);
    a.setFont(QFont("Microsoft YaHei", 8, 50, false));
    UI::MainLogic mainlogic;
    UI::MainLogic::m_pInstance = &mainlogic;
    return a.exec();
}
