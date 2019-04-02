#ifndef STARTSCENE_H
#define STARTSCENE_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>

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
    QLabel* introLabel;
};

#endif // STARTSCENE_H
