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
#include "../Data/DataDef.h"

namespace Ui {
class PlayScene;
}

class PlayScene : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlayScene(QWidget *parent = 0);
    void init();
    ~PlayScene();

//Settings of map
public:
    QSizeF mapSize;
    const QSizeF autoViewPixelSize = QSizeF(25,25);
    const QSizeF defaultPixelSize = QSizeF(9,9);
    QSizeF pixelSize;
    QPointF originPoint;
    bool autoView;
    bool singleMode;
    QPoint mapToGeo(const QPoint& pos);

signals:
    void runThreadStart();

private slots:
    void startGameButtonClicked();
    void resumeGameButtonClicked();
    void speedUpButtonClicked();
    void speedDownButtonClicked();
    void goBackButtonClicked();
    void autoViewButtonClicked();
    void singleContinousButtonClicked();

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
    void opacityUpdate();
    void myUpdateGeometry();
    void focusOn(const QPoint&point);
    void autoViewAdjust();
    void updateTowerInfo(UI::TTower*);
    void updateSoldierInfo(UI::TSoldier*);
    void raiseWidgetss();
    virtual void mousePressEvent(QMouseEvent*event);
    virtual void wheelEvent(QWheelEvent *event);
    friend class Worker;
private:
    Ui::PlayScene *ui;
    QPushButton* startGameButton;
    QPushButton* resumeGameButton;
    QPushButton* goBackButton;
    QPushButton* speedUpButton;
    QPushButton* speedDownButton;
    QPushButton* autoViewButton;
    QPushButton* singleContinousButton;
    QLabel* mapBackGround;
    QLabel* rightBackGround;
    QLabel* roundInfo;
    QVector<QLabel*>opacityLabels;
    QTableWidget* playerInfoTable;
    QListWidget* commandInfoList;
    QTextBrowser* towerInfo;
    QTextBrowser* soldierInfo;


    std::atomic<bool>exit_thread_flag;
    std::atomic<bool>thread_pause;

    QTimer * opacityTimer;
    QTimer* updateGeometryTimer;
    QTimer* autoViewTimer;
    QTimer* raiseTimer;
    QThread workThread;

    QMap<int, QLabel*>soldiers;
    QMap<int, QLabel*>towers;
    QVector<QAbstractAnimation*>animations;
    float wheelScaleRate;

    static int commandsCount;

    //autoView:
    int autoViewMaxStep;
    int autoViewInterval;
    int autoViewCurrentStep;
    QSizeF pixelSizeDiff;
    QPoint targetFocusPoint;

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

};
#endif // PLAYSCENE_H
