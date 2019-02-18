#ifndef STARTSCENE_H
#define STARTSCENE_H

#include <QMainWindow>
#include <QPushButton>

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
};

#endif // STARTSCENE_H
