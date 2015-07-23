#include "drawvotingstatistics.h"
#include "ui_drawvotingstatistics.h"
#include "database.h"
#include "questionpanel.h"

drawVotingStatistics::drawVotingStatistics(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::drawVotingStatistics)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
    this->setWindowTitle("Statistics");
    heightX=ui->graph->height();
    widthX=ui->graph->width();
    pix = new QPixmap(ui->graph->width(),ui->graph->height());
    paint = new QPainter(pix);
    updatedrawing();

}

drawVotingStatistics::~drawVotingStatistics()
{
    delete ui;
}

void drawVotingStatistics::updatedrawing(){
    QSqlQuery query;
    QMessageBox msgBox;
    query.exec("select A.questionMode,A.vote,A.voterID,B.text from sessionvotes as A "
               "inner join sessionquestions as B on A.sessionID=B.sessionID and "
               "A.questionID=B.questionID "
               "where A.sessionID="+QString::number(database::sessionID)+
               " and B.questionID="+QString::number(questionPanel::activeQuestionID)
               +" and B.parent=0");
    int vote,mode,columnWidth,totalVoters=0;


        /*init v seting 0 in each position of array*/

    if(questionPanel::mode<3){
        try{
            if(questionPanel::availableChoices==0){
                throw std::overflow_error("Divide by zero exception!");
            }
    columnWidth=widthX/(2*questionPanel::availableChoices);
    memset(voteBuffer, 0, sizeof(voteBuffer)); //clear buffer

    while(query.next()){
    vote=query.value(1).toInt();
    mode=query.value(0).toInt();

    totalVoters++;
    if(mode!=3){
        voteBuffer[vote]++;
        choiceTexts[vote]=query.value(3).toString();
    }
    }

    paint->fillRect(QRect(QPoint(0,0),QPoint(widthX,heightX)),QColor(0,0,50));
    paint->setPen(QPen(QColor(255,255,255),1,Qt::DashLine));

    int xoffset=20,yoffset=30;
    if(totalVoters==0){
        msgBox.setText("oops!\nThere is no data to be shown at this moment.\n"
                       "you might have not enabled voting or the voting is yet to be done.");
        msgBox.exec();
        throw std::overflow_error("no vote exists!");
    }
    //questionPanel::availableChoices;
    for (int i=1; i<questionPanel::availableChoices+1; ++i){
        //qDebug()<<i<<" - "<<voteBuffer[i];
        paint->drawLine(QPointF(xoffset,heightX-yoffset),QPointF(widthX-xoffset,heightX-yoffset));
        if(questionPanel::mode==2){
        paint->fillRect(QRectF(QPointF(xoffset+(i-1)*(widthX-xoffset)/(questionPanel::availableChoices),
                                heightX-yoffset)
                        ,QPointF(xoffset+columnWidth+(i-1)*(widthX-xoffset)/(questionPanel::availableChoices),
                                 heightX-5-yoffset-0.8*(heightX)*voteBuffer[i]/totalVoters))
                        ,QColor(255,200,120));
        paint->drawText(QPointF(xoffset+columnWidth/2+(i-1)*(widthX-xoffset)/(questionPanel::availableChoices),
                                heightX-yoffset/3),QString::number(i));
        paint->drawText(xoffset+(i-1)*(widthX-xoffset)/(questionPanel::availableChoices),
                                heightX-20-yoffset-0.8*(heightX)*voteBuffer[i]/totalVoters,columnWidth
                        ,15,Qt::AlignCenter
                                ,QString::number(100*voteBuffer[i]/totalVoters)+"% ("
                        +QString::number(voteBuffer[i])
                        +"/"+QString::number(totalVoters)+")"
                        );
        } else{
            char optX='a'+i-1;
            paint->fillRect(QRectF(QPointF(xoffset+(i-1)*(widthX-xoffset)/(questionPanel::availableChoices),
                                    heightX-yoffset)
                            ,QPointF(xoffset+columnWidth+(i-1)*(widthX-xoffset)/(questionPanel::availableChoices),
                                     heightX-5-yoffset-0.8*(heightX)*voteBuffer[i-1]/totalVoters))
                            ,QColor(255,200,120));
            paint->drawText(QPointF(xoffset+columnWidth/2+(i-1)*(widthX-xoffset)/(questionPanel::availableChoices),
                                    heightX-yoffset/3),(QChar)optX);
            paint->drawText(xoffset+(i-1)*(widthX-xoffset)/(questionPanel::availableChoices),
                                    heightX-20-yoffset-0.8*(heightX)*voteBuffer[i-1]/totalVoters,columnWidth
                            ,15,Qt::AlignCenter
                                    ,QString::number(100*voteBuffer[i-1]/totalVoters)+"% ("
                            +QString::number(voteBuffer[i-1])
                            +"/"+QString::number(totalVoters)+")"
                            );
        }

    }

    ui->graph->setPixmap(*pix);

        } catch(std::overflow_error e){
            std::cout<<e.what()<<std::endl;

        }
    }
}
