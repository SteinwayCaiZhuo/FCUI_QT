#ifndef PLAYSCENE_H
#define PLAYSCENE_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QImage>
#include <atomic>
#include <QThread>
#include <QWheelEvent>
#include <QPointF>
#include <QAbstractAnimation>
#include <QTableWidget>
#include <QListWidget>
#include <QTextBrowser>
#include <QSet>
#include<QTimer>
#include <QDebug>
#include <QTextEdit>
#include <QComboBox>
#include <utility>
#include "../Data/DataDef.h"
#include "../Logic/MainLogic.h"

namespace Ui {
class PlayScene;
}



class PlayScene : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlayScene(QWidget *parent = 0);
    void gameInit();
    ~PlayScene();

    enum PlayState
    {
        NOTSTART,   //the state when playScene is constructed
                    //click start, then goes to NOPAUSERUNNING,
        GAMEPAUSE,  //If in single move mode && click pause
                    //Goes to NOPAUSERUNNING if click resume in single move mode
                    //      or switch to continuous move mode.
        GAMESTOP,   //should go to NOTSTART to safely start a new game

        NOPAUSERUNNING, //Running and there is no action.
                        //Exist for both single and continuous.
        THREADPAUSE,    //Running and an action is going on.


    };
public:
    PlayState playState;
    QLabel* playStateLabel;
    UI::MainLogic* mainLogic;

//Settings of map
public:
    QSizeF mapSize;
    const QSizeF autoViewPixelSize = QSizeF(25,25);
    const QSizeF defaultPixelSize = QSizeF(18,18);
    QSizeF pixelSize;
    QPointF originPoint;
    bool autoView;
    bool singleMode;
    QPoint mapToGeo(const QPoint& pos);
    QPoint mapToGeo(const QPointF& pos);

signals:
    void runThreadStart();

private slots:
    void startGameButtonClicked();
    void resumeGameButtonClicked();
    void speedUpButtonClicked();
    void speedDownButtonClicked();
    void goBackButtonClicked();
    void autoViewButtonClicked();
    void singleContinuousButtonClicked();
    void changeViewPlayerCallback();
    void changeFontSizeCallback();

public slots:
    void playerUpdate(UI::TPlayer*player);
    void towerUpdate(UI::TTower*tower);
    void soldierUpdate(UI::TSoldier*soldier);
    void commandUpdate(UI::Command*command);
    void roundUpdate();
    void clearTowers();
    void clearSoldiers();
    void resumeThread();
    void resumeThreadAutoView();
    void goToLoopBeginCallback();
    void opacityUpdate();
    void myUpdateGeometry();
    void viewUpdate();
    void paintEvent(QPaintEvent*event);
    void focusOn(const QPoint&point);
    void focusOn(const QPointF&point);
    void autoViewAdjust();
    void updateTowerInfo(UI::TTower*);
    void updateSoldierInfo(UI::TSoldier*);
    void raiseWidgetss();
    void raiseWidgets2();
    virtual void mousePressEvent(QMouseEvent*event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void keyPressEvent(QKeyEvent*event);
    virtual void closeEvent(QCloseEvent*event);
    void showStatusWindow();
    friend class Worker;
    friend class MoveSoldier;
private:
    Ui::PlayScene *ui;
    QWidget* statusWindow;
    QPushButton* startGameButton;
    QPushButton* resumeGameButton;
    QPushButton* goBackButton;
    QPushButton* speedUpButton;
    QPushButton* speedDownButton;
    QPushButton* autoViewButton;
    QPushButton* singleContinuousButton;
    QComboBox* roundComboBox;
    QComboBox* viewPlayerBox;
    QComboBox* fontSizeBox;

    QLabel* mapBackGround;
    QLabel* rightBackGround;
    QTextBrowser* roundInfo;
    QVector<QLabel*>opacityLabels;
    QTableWidget* playerInfoTable;
    QListWidget* commandInfoList;
    QTextBrowser* towerInfo;
    QTextBrowser* soldierInfo;

    QTextBrowser* briefInfo;


    std::atomic<bool>exit_thread_flag;
    std::atomic<bool>thread_pause;
    std::atomic<bool>goToLoopBegin_flag;

    float playSpeed;
    const float minimumPlaySpeed = 0.002;
    QTimer * opacityTimer;
    QTimer* updateGeometryTimer;
    QTimer* autoViewTimer;
    QTimer* raiseTimer;
    QTimer* raiseTimer2;
    QTimer* viewTimer;
    QThread workThread;

    QMap<int, QLabel*>soldiers;
    QMap<int, QLabel*>towers;
    QMap<int, QLabel*>towerBars;
    bool useView;
    int viewID;
    QVector<QAbstractAnimation*>animations;
    float wheelScaleRate;
    float translateScaleRate;

    static int commandsCount;

    //autoView:
    int autoViewMaxStep;
    int autoViewInterval;
    int autoViewCurrentStep;
    QSizeF pixelSizeDiff;
    QPointF targetFocusPoint;

    float focusTime;
};

class Worker: public QObject
{
    Q_OBJECT

public slots:
    void doWork();
signals:
    void soldierUpdateSignal(UI::TSoldier*soldier);
    void finishedWork();
public:
    PlayScene* playScene;
    UI::MainLogic* mainLogic;

    void checkState();
};

class MoveSoldier:public QObject
{
    Q_OBJECT
public:
    using ValueType = QPair<float, QPointF>;
public:
    MoveSoldier(QPointF*moveObject = nullptr, int duration = 1000);
    ~MoveSoldier();
public:
    QPointF* moveObject;
    QVector<ValueType>values;
    QTimer* moveTimer;
    int maxStep;//=duration/interval
    float currStep;
    int duration;
    void setValue(float step, const QPointF&value);
    void startMove();
    QPair<ValueType, ValueType>getNeighbor();
public slots:
    void updateValue();

    //Call MoveSoldier* = new MoveSoldier(moveSoldier, duration);MoveSoldier->setValue(.,.);MoveSoldier->start();
    //static QTimer* deleteTimer;
public slots:

public:
   // static void deleteLater(MoveSoldier*);
      static QSet<MoveSoldier*>moveToDelete;
public:
    int interval;
    PlayScene* playScene;
    UI::MainLogic* mainLogic;
};


#endif // PLAYSCENE_H
