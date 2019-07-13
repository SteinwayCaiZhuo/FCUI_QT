#include "StartScene.h"
#include "ui_StartScene.h"
#include <QFileDialog>
#include "../Logic/MainLogic.h"
#include "PlayScene.h"
#include <QPalette>
#include <QGraphicsEffect>
#include <QGraphicsOpacityEffect>

StartScene::StartScene(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StartScene)
{
    ui->setupUi(this);
    this->setWindowTitle("FC16UI-START");
    this->setGeometry(QRect(QPoint(720, 405), QSize(480, 270)));
    this->setFixedSize(QSize(480, 270));
    this->setWindowIcon(QIcon(":/FC16UIResource/Logo1.PNG"));
    //this->setStyleSheet(QString("background: rgba(0,255,187,50%)"));

    QPalette pal(this->palette());
    pal.setColor(QPalette::Background,QColor(255,255,255));
    this->setPalette(pal);

    QImage backImg;
    if(backImg.load(":/FC16UIResource/Logo2.PNG"))
    {
        imgGround = new QLabel(this);
        imgGround->setGeometry(QRect(QPoint(0,0),this->size()));
        imgGround->setScaledContents(true);
        imgGround->setPixmap(QPixmap::fromImage(backImg));
        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(0.5);
        imgGround->setGraphicsEffect(effect);
    }

    introLabel = new QTextBrowser(this);
    introLabel->setText("文件路径不要包含中文字符\n文件命名最好按照C语言命名标准"
                        "\n可在settings.txt内设置字体大小");
    introLabel->setGeometry(QRect(QPoint(130, 40),
                                  QSize(240, 80)));
    introLabel->setReadOnly(true);


    openFileButton = new QPushButton("Select a File", this);
    openFileButton->setGeometry(QRect(QPoint(160, 190),
                                QSize(160, 50)));
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
    UI::MainLogic::GetInstance()->clearData();
    UI::MainLogic::GetInstance()->LoadData();
}

