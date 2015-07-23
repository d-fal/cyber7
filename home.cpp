#include "home.h"
#include "ui_home.h"
#include <QInputDialog>
#include "connectionmanager.h"
#include "sessionmanagement.h"
#include "questionpanel.h"
#include "cserial.h"
#include "database.h"
#include "showmessage.h"

#include <QTimer>
#include <ctime>
#define _PI 3.141592

using namespace std;
/* Dictionary:
 * SLOT : showSignalOnUi(int index,QString message)
 *  SIGNAL: sendSignalToUi(int index, QString message);
 *  index dictionary: 0 -> show on connection status
 * #72#83#F1 : check the main receiver connection
 * #70#99#E9 : ask the main receiver to get ready
 * #40#99#D9 : ask the first extended receiver to get ready
 */
connectionManager connection1[5];
database *newDb = new database;
syncData *sync = new syncData;

int home::xOffset=10;
int home::yOffset,home::heightX,home::widthX;
QPointF home::prevPoint;
bool home::isVotingOpen;
int home::mode=0;
QString home::modeTitle;
CSerial serial;
QString home::systemCode;
bool home::isGenuine;
QSettings settings("Cyber 7", "Polling device");
home::home(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::home)
{
    ui->setupUi(this);
    //

    networkManager::serverAddress=settings.value("server address").toString();

    QMenu *fileMenu = menuBar()->addMenu(tr("&Session management"));
    QMenu *editMenu = menuBar()->addMenu(tr("&Show statistics"));
    QMenu *networkMenu = menuBar()->addMenu(tr("&Network"));
    QMenu *attendantsMenu = menuBar()->addMenu(tr("&Attendants"));

    fileMenu->addAction("Activate/Create a session",this,
                        SLOT(sessionManager()),Qt::CTRL+Qt::Key_N);
    fileMenu->addSeparator();
    fileMenu->addAction("Question management",this,
                        SLOT(questionManager()),Qt::CTRL+Qt::Key_Q);
    editMenu->addAction("Draw statistics",this,SLOT(showVotingStatistics()),Qt::CTRL+Qt::Key_T);
    networkMenu->addAction("server address",this,SLOT(setServerAddress()),Qt::CTRL+Qt::Key_F);
    attendantsMenu->addAction("Attendants Info",this,SLOT(showSubmitedPeople()),Qt::CTRL+Qt::Key_M);
    //
     this->setFixedSize(this->width(),this->height()); // fix the main window size



    for(int i=0;i<5;++i){
    connect(&connection1[i],SIGNAL(sendSignalToUi(int,QString))
            ,this,SLOT(showSignalOnUi(int,QString)));
    connect(&connection1[i],SIGNAL(sendRemoteCommand(int,int,int)),
            this,SLOT(recieveRemoteCommand(int,int,int)));
    connect(&connection1[i],SIGNAL(sendRemoteCommand(int,int,int))
            ,newDb,SLOT(saveVote(int,int,int)));
    }
    connect(newDb,SIGNAL(illegalVote(int,int)),this,SLOT(illegalVote(int,int)));
    connect(newDb,SIGNAL(masterCommand(int)),this,SLOT(masterCommand(int)));
    newDb->startSession();


    xOffset=10;
    yOffset=80;
    heightX=ui->graph->height();
    widthX=ui->graph->width();

    pix = new QPixmap(widthX,heightX);
    paint = new QPainter(pix);
    slotno=xOffset;

    makeConnection();

    QTimer *timer = new QTimer(this);
    //QTimer *timer2 = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(drawStatistics()));
    //connect(timer2, SIGNAL(timeout()), sync, SLOT(sendVotersList()));

    timer->start(1000);
    //timer2->start(60000);

    activateThisDistro();
    win = new QWidget;
    layout = new QGridLayout;
    label = new QLabel;
    win->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

}

home::~home()
{

    newDb->endSession();


    for(int i=0;i<5;++i){
        if(connection1[i].isActiveDevice){

        connection1[i].closeVoting();
        connection1[i].terminate();
        }
    }
    delete ui;
}

void home::syncDataWithServer(){

    timer3 = new QTimer(this);

    connect(timer3,SIGNAL(timeout()),win,SLOT(close()));
    connect(timer3,SIGNAL(timeout()),timer3,SLOT(stop()));
    label->setText("Syncing...");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    int screenWidth = QApplication::desktop()->geometry().width();
    int screenHeight = QApplication::desktop()->geometry().height();
    win->setGeometry(screenWidth-200,screenHeight-200,200,200);
    win->setStyleSheet("background-color:#fef;");
    win->setLayout(layout);
    win->show();

    if(syncData::sendVotersList()){
    label->setText("Data synced");
    } else{
        label->setText("Syncying failed!");
    }
     timer3->start(3000);
      sync->sendQuestionsToserver(questionPanel::activeQuestionID,database::sessionID);
}

void home::activateThisDistro(){
    isGenuine=settings.value("genuine").toString().toInt()>0;
    if(!isGenuine){
        bool ok;
            QString text = QInputDialog::getText(this, tr("Activate the program"),
                                                 tr("This distribution is not registered yet\n"
                                                    "Some features such as networking would not be able"
                                                    " in trial mode.\n"
                                                    " Activation code should have been sent "
                                                    "to you when you purchased the cyber7 system.\n"
                                                    " If you can not find the code call +98(21)xxxxx .\n"
                                                    "Enter your activation code:"), QLineEdit::Normal,
                                                 "Activation code", &ok);
            if (ok && !text.isEmpty()){
                if(sync->checkGenuiness(text)){
                    systemCode=syncData::serverResponseCode;
                    settings.setValue("systemCode",systemCode);
                    settings.setValue("genuine","1");
                    QMessageBox msgBox;
                    msgBox.setText("Congratulations!\nYou have successfully enabled your ARS system.\n"
                                   "Now you will be able to enjoy from networking services.");
                    isGenuine=true;
                    msgBox.exec();
                }else{
                    QMessageBox msgBox;
                    msgBox.setText("Error!\nYou have entered an incorrect activation code.\n"
                                   "This code may have been used by another system.\n"
                                   "Please call our technical team if you don't know what to do.");
                    msgBox.exec();
                }
            }
            else{
                qDebug()<<"This distro is not active";
            }
    }else{
    systemCode=settings.value("systemCode").toString();
    }
}

void home::showSignalOnUi(int index, QString message){

    switch (index) {
    case 0:
        ui->connectionStatus->setText(message);
        ui->connectionStatus->setStyleSheet("color: green;");
        break;
    case 1:
        break;
    case 2: //extensive alarm
        ui->connectionStatus->setText(ui->connectionStatus->text()+", "+message);
        break;
    default:
        break;
    }
}

void home::on_pushButton_clicked(){

    if(!questionPanel::isQuestionSelected){
        QMessageBox msgBox;
        msgBox.setText("Error!\nyou are not allowed to start voting now.\n"
                       "you should select a question before start voting...\n"
                       "Please select or make a question from 'question management' menu.");
        msgBox.exec();
        return;
    }
    isVotingOpen=true;
    ui->pushButton_3->setEnabled(false);

    if(ui->afsingle->isChecked()) {
        mode=0;
        modeTitle="A-F mode";
    }
    else if(ui->afmulti->isChecked()){
        mode=1;
        modeTitle="A-F multi-choice";
    }
    else if(ui->oneto100->isChecked()){
        mode=2;
        modeTitle="1-100 numerical mode";
    } else if(ui->oneto100Avg->isChecked()){
        mode=3;
        modeTitle="average mode";
    }
    //connectionManager *connection= new connectionManager(this);
    for(int i=0;i<5;++i){
        if(connection1[i].isActiveDevice){
            connection1[i].openVoting(mode);
    Sleep(200);
        }
    }
}

void home::on_pushButton_2_clicked()
{// ----------------------------------- stop voting
    elapsedTime=0;
    ui->pushButton_3->setEnabled(true);
    isVotingOpen=false;
    for(int i=0;i<5;++i){
        if(connection1[i].isActiveDevice){
            connection1[i].closeVoting();
        }
    }

 syncDataWithServer();
 sync->sendQuestionsToserver(questionPanel::activeQuestionID,database::sessionID);
 //-------------------------------------------------
}

void home::on_pushButton_3_clicked()
{
    int id=ui->spinBox->value();
    if((id>400) && (id<800) ){
    connection1[1].setRemoteID(id);
    } else if((id<400) && (id>0)){
    connection1[0].setRemoteID(id);
    }
}


int votesCount=0;
int counterBlink=0;
void home::drawStatistics(){
    ui->resourceMonitor->setTitle("Device resource usage: "+QString::number(votesCount%100)+"%");
    ui->graph->setPixmap(*pix);
    if(slotno!=xOffset){
        paint->drawLine(prevPoint,
                         QPointF(slotno,heightX-yOffset-5*votesCount));
           }
    if(slotno>widthX-xOffset || slotno==xOffset){
        slotno=xOffset;
        paint->fillRect(QRect(QPoint(0,0),QPoint(widthX,heightX)),QColor(0,0,50));
        paint->setPen(QPen(QColor(255,255,255),1,Qt::DashLine));
        paint->drawLine(QPointF(xOffset,heightX-yOffset-50),
                        QPointF(widthX-xOffset,heightX-yOffset-50));
        paint->drawText(QPointF(xOffset,heightX-yOffset-52),"10 votes");
        paint->drawLine(QPointF(xOffset,heightX-yOffset-100),
                        QPointF(widthX-xOffset,heightX-yOffset-100));
        paint->drawText(QPointF(xOffset,heightX-yOffset-102),"20 votes");
        paint->drawLine(QPointF(),QPointF());
        paint->drawText(QPointF(widthX-12*xOffset,20),"Genuine distribution");

    }
prevPoint=QPointF(slotno,heightX-yOffset-5*votesCount);
slotno+=4;

if(isVotingOpen){

    paint->fillRect(QRect(QPoint(0,heightX-yOffset),QPoint(widthX,heightX)),QColor(0, 0, 100));
    paint->setPen(QPen(QColor(0,255,100),2));
    if(counterBlink%2==0){
    paint->setPen(QPen(Qt::white,4));
    counterBlink=0;
    }
    paint->drawText(QPointF(xOffset,heightX-3*yOffset/4),"Voting is active in "+modeTitle+
                        "     elapsed time: "+QString::number((elapsedTime/3600)%24)+":"+
                    QString::number((elapsedTime/60)%60)+":"+
                    QString::number(elapsedTime%60));
    paint->setPen(QPen(QColor(0,255,100),2));
    counterBlink++;
    elapsedTime+=1;
}
else{
    paint->fillRect(QRect(QPoint(0,heightX-yOffset),QPoint(widthX,heightX)),QColor(50, 0, 0));
    paint->setPen(QPen(QColor(255,0,100),2));
    paint->drawText(QPointF(xOffset,heightX-3*yOffset/4),"Voting is not active ");

}
    paint->drawText(QPointF(xOffset,heightX-yOffset/2),"session title: "+database::sessionTitle);
    if(questionPanel::isQuestionSelected){
    paint->drawText(QPointF(xOffset,heightX-yOffset/2+20),"question title: "+questionPanel::questionTitle);
    paint->drawText(QPointF(xOffset,heightX-yOffset/2+35),"mode: "+questionPanel::mode_text);



}
    paint->drawText(QPointF(widthX/2+xOffset+20,heightX-3*yOffset/4),"server: "+networkManager::serverAddress);

    votesCount=0;

}
void home::recieveRemoteCommand(int remoteId, int choice, int mode){
     time_t now2 = time(0);
     tm *ltm = localtime(&now2);
    if(remoteId!=0){
    ui->communicationLog->append(QString::number(remoteId)+" -> "+QString::number(choice)+
                                 " in mode "+QString::number(mode)
                                 +" at: "+QString::number(1900+ltm->tm_year)+"-"+
                                 QString::number(ltm->tm_mon)+"-"+
                                 QString::number(ltm->tm_mday)+" "
                                +QString::number(ltm->tm_hour)+":"
                                +QString::number(ltm->tm_min)+":"
            +QString::number(ltm->tm_sec));
    } else{
        ui->communicationLog->append("Master: -> "+QString::number(choice)+
                                     " at: "+QString::number(1900+ltm->tm_year)+"-"+
                                     QString::number(ltm->tm_mon)+"-"+
                                     QString::number(ltm->tm_mday)+" "
                                    +QString::number(ltm->tm_hour)+":"
                                    +QString::number(ltm->tm_min)+":"
                +QString::number(ltm->tm_sec));

    }
    //paint->drawText(QPointF(20,100),QString::number(remoteId)+" voted");
    votesCount++;


}

void home::sessionManager(){
    sessionManagement *session = new sessionManagement(this);
    session->show();

}

void home::questionManager(){

    questionPanel *question = new questionPanel(this);
    connect(question,SIGNAL(setQuestionActive()),this,SLOT(activateVotingCounter()));
    connect(question,SIGNAL(changeModeStatus(int)),this,SLOT(changeModeStatus(int)));
    question->show();
}

void home::makeConnection(){
    int id=0;
    for(int i=0 ; i<5 ;++i){
        if(serial.Open(i,115200)){
            serial.Close();

        if(!connection1[i].isActiveDevice){
    connection1[id].initializeConnection(i);
    connection1[id].start();

    Sleep(500);
    id++;
        }
    }
    } //
}

void home::on_commandLinkButton_clicked()
{
    makeConnection();
}

void home::changeModeStatus(int mode){
    switch (mode) {
    case 0:
        ui->afsingle->setChecked(true);
        break;
    case 1:
        ui->afmulti->setChecked(true);
        break;
    case 2:
        ui->oneto100->setChecked(true);
        break;
    case 3:
        ui->oneto100Avg->setChecked(true);
    default:
        break;
    }
}
void home::illegalVote(int id, int choice){
    qDebug()<<"voter's "<<id<<" vote is not acceptable";
}

void home::showVotingStatistics(){
    drawVotingStatistics *statistics = new drawVotingStatistics(this);
    statistics->show();
    connect(this,SIGNAL(closeAll()),statistics,SLOT(close()));

}

void home::setServerAddress(){
    networkManager *net = new networkManager(this);
    connect(net,SIGNAL(activateDistro()),this,SLOT(activateThisDistro()));
    net->show();

}

void home::activateVotingCounter(){
     timer3= new QTimer(this);
    connect(timer3,SIGNAL(timeout()),this,SLOT(setVotingCounter()));
    timer3->start(1000);
    idTimer=0;
    on_pushButton_2_clicked();
    on_pushButton_clicked();
}

void home::setVotingCounter(){
    if(questionPanel::questionLength-idTimer>30){
        ui->countDown->setStyleSheet("background-color:green;color:white");

    }else{
         ui->countDown->setStyleSheet("background-color:red;color:white");
    }
    ui->countDown->setText(QString::number(questionPanel::questionLength-idTimer)+" seconds");
    idTimer++;
    if (idTimer>=questionPanel::questionLength){
        timer3->stop();
         ui->countDown->setStyleSheet("background-color:#f0f0f0;");
         ui->countDown->setText("");
         on_pushButton_2_clicked();

    }
}

void home::showSubmitedPeople(){
    if(!questionPanel::isQuestionSelected){
        QMessageBox msgBox;
        msgBox.setText("Error!\nThere is nothing to see about.\n"
                       "you should select a question before performing this action...\n"
                       "Please select or make a question from 'question management' menu.");
        msgBox.exec();
        return;
    }
    submittedPeople *submitted;
    submitted= new submittedPeople();
    connect(this,SIGNAL(closeAll()),submitted,SLOT(on_pushButton_clicked()));
    connect(this,SIGNAL(endIt()),submitted,SLOT(on_pushButton_clicked()));
    connect(newDb,SIGNAL(updateDemonstrativePanels()),submitted,SLOT(update()));
    submitted->show();

}

void home::on_closeBtn_clicked()
{
    emit endIt();
   this->close();
}

void home::questionUp(){
    showMessage *show = new showMessage(this);
    show->display("This message");
}

void home::masterCommand(int code){


    //emit getMasterCommand(command);
    switch (code) {
    case 0:
        cout<<"Up"<<endl;
        questionUp();
        break;
    case 1:
        cout<<"OK"<<endl;

        break;
    case 2:
        cout<<"power"<<endl;
        emit closeAll();

        break;
    case 3:
        cout<<"chart"<<endl;
        showVotingStatistics();
        break;
    case 4:
        cout<<"voters list"<<endl;
        showSubmitedPeople();
        break;
    case 5:
        cout<<"play/pause"<<endl;
        on_pushButton_clicked();
        break;
    case 6:
        on_pushButton_2_clicked();
        cout<<"Stop"<<endl;
        break;
    case 7:

        cout<<"Down"<<endl;
        break;
    case 8:

        cout<<"send report"<<endl;
        questionManager();

        break;
    case 9:

        cout<<"Question Mark"<<endl;
        break;
    case 10:

        cout<<"F1"<<endl;
        break;
    case 11:

        cout<<"F2"<<endl;
        break;
    default:
        break;
    }

}
