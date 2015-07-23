#ifndef HOME_H
#define HOME_H

#include <QMainWindow>
#include <QPainter>
#include <iostream>
#include <QMenuBar>
#include <QMessageBox>
#include "drawvotingstatistics.h"
#include "syncdata.h"
#include "networkmanager.h"
#include <stdexcept>
#include <QSettings>
#include <QDesktopWidget>
#include <QGridLayout>
#include "submittedpeople.h"
namespace Ui {
class home;
}

class home : public QMainWindow
{
    Q_OBJECT

public:
    explicit home(QWidget *parent = 0);
    ~home();
    QPixmap *pix;
    QPainter *paint;
    static int xOffset,yOffset,heightX,widthX;
    static QPointF prevPoint;
    static bool isVotingOpen;
    int slotno;
    static int mode;
    static QString modeTitle;
    static QString systemCode;
    static bool isGenuine;
    QWidget *win;
    QGridLayout *layout;
    QLabel *label;
    int elapsedTime;
    QTimer *timer3;
    int idTimer;


private:
    Ui::home *ui;

private slots:
    void showSignalOnUi(int,QString);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void drawStatistics();
    void recieveRemoteCommand(int, int, int mode);
    void sessionManager();
    void questionManager();
    void makeConnection();
    void changeModeStatus(int);
    void on_commandLinkButton_clicked();
    void showVotingStatistics();
    void illegalVote(int,int);
    void setServerAddress();
    void activateVotingCounter();
    void setVotingCounter();
    void showSubmitedPeople();
    void masterCommand(int);
    void on_closeBtn_clicked();
    void activateThisDistro();
    void syncDataWithServer();
    void questionUp();
signals:
    void endIt();
    void closeAll();

};

#endif // HOME_H
