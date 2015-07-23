#include "showmessage.h"
#include <QApplication>
int showMessage::screenHeight;
int showMessage::screenWidth;
showMessage::showMessage(QObject *parent) :
    QObject(parent)
{
    layout = new QGridLayout;
    board = new QLabel;
    win= new QWidget;
    timer1 = new QTimer;
    connect(timer1,SIGNAL(timeout()),win,SLOT(close()));
}

bool showMessage::display(QString msg){
    int width=400;
    int height=400;
    int screenWidth = QApplication::desktop()->geometry().width();
    int screenHeight = QApplication::desktop()->geometry().height()*.98;


    pix = new QPixmap(width,height);
    paint = new QPainter(pix);


    paint->fillRect(QRect(0,0,width,height),QColor(220,215,255));
    paint->drawText(QRect(0,0,width,height),Qt::AlignCenter,msg);

    board->setPixmap(*pix);
    layout->addWidget(board);

    win->setLayout(layout);
    win->setGeometry(screenWidth/2-width/2,screenHeight/2-height/2,width/2,height/2);

    //
    QFont fontTop=paint->font();
    fontTop.setPointSize(24);
    paint->setPen(QPen(QColor(0,255,255),2));
    paint->setFont(fontTop);
    win->setWindowFlags(Qt::FramelessWindowHint |Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint);
    win->show();
    timer1->start(1000);


}
