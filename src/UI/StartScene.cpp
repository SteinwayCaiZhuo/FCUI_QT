#include "StartScene.h"
#include "ui_StartScene.h"
#include <QFileDialog>
#include "../Logic/MainLogic.h"

StartScene::StartScene(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StartScene)
{
    ui->setupUi(this);
    this->setWindowTitle("FC16UI-START");


    openFileButton = new QPushButton("Select a File", this);
    openFileButton->setGeometry(QRect(QPoint(100, 100),
                                QSize(200, 50)));
    connect(openFileButton, SIGNAL(clicked(bool)),
            this, SLOT(openFileButtonClicked()));

}

StartScene::~StartScene()
{
    delete ui;
    if(openFileButton!=nullptr)
        delete openFileButton;
}

void StartScene::openFileButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select Result File"), "", tr("text file(*.txt)"));
    if(fileName.isEmpty())
        return;
    UI::MainLogic::GetInstance()->loadFileName = fileName;
    UI::MainLogic::GetInstance()->GameStart();
}

