#include <QApplication>
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
    UI::MainLogic mainlogic;
    UI::MainLogic::m_pInstance = &mainlogic;
    return a.exec();
}
