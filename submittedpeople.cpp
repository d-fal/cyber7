

#include "submittedpeople.h"
#include "ui_submittedpeople.h"
#include "database.h"
#include "questionpanel.h"
int submittedPeople::width;
int submittedPeople::height;
int submittedPeople::totalReadies;
int submittedPeople::totalQuestionVotes;
int submittedPeople::screenHeight;
int submittedPeople::screenWidth;
submittedPeople::submittedPeople(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::submittedPeople)
{
    ui->setupUi(this);
    page=0;

    this->setWindowFlags(Qt::FramelessWindowHint );

    width=ui->graph->width();
    height=ui->graph->height();
    screenWidth = QApplication::desktop()->geometry().width()*.99;
    screenHeight = QApplication::desktop()->geometry().height()/4;

    pix = new QPixmap(width,height);
    paint = new QPainter(pix);
    pixTop = new QPixmap(screenWidth,screenHeight);
    paintTop = new QPainter(pixTop);
    QGridLayout *layout = new QGridLayout;
     readiesBoard= new QLabel;
     win= new QWidget;
    layout->addWidget(readiesBoard,0,0,-1,-1,Qt::AlignCenter);
    win->setLayout(layout);
    win->setWindowFlags(Qt::FramelessWindowHint |Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint);

    win->setStyleSheet("background-color:#fef;");
    win->setGeometry(0,0,screenWidth,screenHeight);
    win->show();
    timer4= new QTimer(this);
    timer4->start(5000);
    connect(timer4,SIGNAL(timeout()),this,SLOT(nextPage()));
    update();
}

submittedPeople::~submittedPeople()
{
    win->close();
    delete ui;

}

void submittedPeople::update(){
    int colItemsX=10;
    int colItemsY=5;
    int colWidth=screenWidth/colItemsX;
    int colHeight=screenHeight/colItemsY;
    std::vector<int> voters;
    database::selectVotersToCurrentQuestion(questionPanel::activeQuestionID,voters);


    database::returnNumberOfReadies(database::sessionID,totalReadies,
                                    questionPanel::activeQuestionID,totalQuestionVotes);

    paint->fillRect(QRect(0,0,width,height),QColor(220,215,255));
    paintTop->fillRect(QRect(0,0,screenWidth,screenHeight),QColor(220,215,255));
    // set grid
    QColor color;
    QFont fontTop=paintTop->font();
    fontTop.setPointSize(24);
    paintTop->setFont(fontTop);
     for(int j=0;j<colItemsY;++j){
         for(int i=0; i<colItemsX; ++i){


        color=(j)%2==0?QColor(255,220,240):QColor(255,240,220);
        paintTop->fillRect(QRect(i*colWidth,j*colHeight,colWidth,colHeight),color);
            if(i==0 && j==0 && page==0) continue;
        paintTop->drawText(QRect(i*colWidth,j*colHeight,colWidth,colHeight),Qt::AlignCenter,
                           QString::number(page+j*colItemsX+i));
         }
     }

     paintTop->setPen(QPen(QColor(255,255,255),2));
     maxVoterno=0;
     for(int k=0;k<voters.size();++k){
         color=QColor(0,100,189);
         maxVoterno=voters[k]>maxVoterno?voters[k]:maxVoterno;

         if(voters[k]>page+50 || voters[k]<page){
             continue;
         }
         int j=((voters[k])/colItemsX)%colItemsY;
         int i=(voters[k])%colItemsX;

         paintTop->fillRect(QRect(i*colWidth,j*colHeight,colWidth,colHeight),color);
         paintTop->drawText(QRect(i*colWidth,j*colHeight,colWidth,colHeight),Qt::AlignCenter,
                            QString::number(page+j*colItemsX+i));
     }
    paintTop->setPen(QPen(QColor(0,0,0),2));

    // end grid

    QFont font=paint->font();
    font.setPointSize(18);
    font.setBold(true);
    paint->setFont(font);
    paint->drawText(QRectF(0,0,width,height/2),Qt::AlignCenter,"There are "+
                    QString::number(totalReadies)+" voters who voted for this question.");
    paint->drawText(QRectF(0,height/2,width,height/2),Qt::AlignCenter,
                    " And there are "+QString::number(totalQuestionVotes)+" voters "
                    "who votes for question no "
                    +QString::number(questionPanel::activeQuestionID)+".");
    ui->graph->setPixmap(*pix);
    readiesBoard->setPixmap(*pixTop);
}
void submittedPeople::nextPage(){
    page+=50;

    if(page>maxVoterno) page=0;
    update();
}

void submittedPeople::on_pushButton_clicked()
{
       win->close();
       timer4->stop();
       this->close();
}
