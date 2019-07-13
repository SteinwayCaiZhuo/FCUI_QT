#ifndef STARTSCENE_H
#define STARTSCENE_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QTextBrowser>
namespace Ui {
class StartScene;
}

class StartScene : public QMainWindow
{
    Q_OBJECT

public:
    explicit StartScene(QWidget *parent = 0);
    ~StartScene();

private slots:
    void openFileButtonClicked();
private:
    Ui::StartScene *ui;
    QPushButton* openFileButton;
    QTextBrowser* introLabel;
    QLabel* imgGround;
};

#endif // STARTSCENE_H
