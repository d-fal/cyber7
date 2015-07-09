#include "database.h"
#include "questionpanel.h"
#include <QDir>

using namespace std;
QSqlDatabase database::db;
int database::sessionID=0;
QString database::sessionTitle;

database::database(QObject *parent) :
    QThread(parent)
{
    string pPath;
    pPath =getenv ("USERPROFILE");
    QString imgPath=QString::fromUtf8(pPath.c_str())+"\\cyber7\\";
    if(!QDir(imgPath).exists()) QDir().mkdir(imgPath);

    db =QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(imgPath+"cyber7.db");
    if (!db.open()) {
        qDebug()<<db.lastError();
    }else{
        cout<<"DB is openned!"<<endl;
    }
    QSqlQuery query;
    if(!query.exec("create table if not exists sessionvotes (voterID int, questionID int , "
                   "sessionID "
               ",questionMode int,vote int , submissionTime datetime, "
                   "primary key(voterID,questionID))")){
        qDebug()<<db.lastError();
    }
    if(!query.exec("create table if not exists sessioninfo (sessionID int , "
                   "sessionTitle varchar(100), "
                   " time datetime "
                   " , primary key(sessionID,sessionTitle,time))")){
        qDebug()<<db.lastError();
    }
    if(!query.exec("create table if not exists sessionquestions (ID int ,"
                   "questionID int,"
                   " sessionID int , choiceno int ,"
                   "text varchar(100), "
                   " parent int ,mode int, validityTime int, minVal int, maxVal int"
                   " , primary key(questionID,sessionID,choiceno))")){
        qDebug()<<"create sessionquestions: "<<db.lastError();
    }
    if(!query.exec("create table if not exists attendantsInfo (uniqueID varchar(100) ,"
                   "firstname varchar(30),"
                   " lastname varchar(30) , sessionID int, time datetime ,"
                   " primary key(uniqueID,sessionID))")){
        qDebug()<<"create sessionquestions: "<<db.lastError();
    }
    if(!query.exec("create table if not exists attendantsInfo (uniqueID varchar(100) ,"
                   "firstname varchar(30),"
                   " lastname varchar(30) , sessionID int, time datetime ,"
                   " primary key(uniqueID,sessionID))")){
        qDebug()<<"create sessionquestions: "<<db.lastError();
    }
    if(!query.exec("create table if not exists sessionAttendants (id INTEGER, "
                   "attendantID int  primary key, "
                  " isReady int)")){

        qDebug()<<query.lastError();

    }
    query.exec("select max(sessionID),sessionTitle from sessioninfo");
    while (query.next()) {
        sessionID=query.value(0).toInt();
        sessionTitle=query.value(1).toString();
    }
    sessionID++;

}

void database::run(){

}

void database::saveVote(int voterId, int choice, int mode){
    QSqlQuery query;

    if(choice>questionPanel::availableChoices && mode<3 && voterId!=0) {
        emit illegalVote(voterId,choice);
        return;
    }

    if(voterId!=0 ){
    if(!query.exec("insert or replace into sessionvotes (voterID,questionID,sessionID, questionMode,"
               "vote,submissionTime) VALUES"
               "("+QString::number(voterId)+","+QString::number(questionPanel::activeQuestionID)
                   +","+QString::number(sessionID)+","+QString::number(mode)
               +","+QString::number(choice)+",datetime(CURRENT_TIMESTAMP,'localtime'))"))
            qDebug()<<"vote insertion: "<<db.lastError();
} else{
     // master command
        emit masterCommand(choice);
    }

    emit updateDemonstrativePanels();
}

void database::startSession(){
    QSqlQuery query;
    if(!query.exec("insert or replace into sessioninfo (sessionID,sessionTitle,time) VALUES "
               "("+QString::number(sessionID)+",'temporary stack_"+QString::number(sessionID)+
                   "',datetime(CURRENT_TIMESTAMP,'localtime'))")){
        qDebug()<<"start: "<<db.lastError();
    }
}

void database::endSession(){
    QSqlQuery query;
    if(!query.exec("insert or replace into sessioninfo (sessionID,sessionTitle,time) VALUES "
               "("+QString::number(sessionID)+",'"+sessionTitle+
                   "',datetime(CURRENT_TIMESTAMP,'localtime'))")){
        qDebug()<<db.lastError();
    }
}

void database::updateSessionData(int id){
    QSqlQuery query;
    query.exec("select sessionID,sessionTitle from sessioninfo where sessionID="
               +QString::number(id));
    while (query.next()) {
        sessionID=query.value(0).toInt();
        sessionTitle=query.value(1).toString();
    }


}

void database::removeItem(int id){
    QSqlQuery query;
    query.exec("delete from sessioninfo where sessionID="
               +QString::number(id));
}

void database::addItem(){
    QSqlQuery query;
    query.exec("insert into sessioninfo (sessionID,sessionTitle,time) VALUES"
               "((select max(sessionID)+1 from sessionInfo),'new session',"
               "datetime(CURRENT_TIMESTAMP,'localtime'))");

}

void database::mergeItems(int firstItem, int secondItem){
    QSqlQuery query;
    query.exec("update sessionvotes set sessionID="+QString::number(secondItem)+
               " where sessionID="+QString::number(firstItem));
    removeItem(firstItem);

}

void database::addQuestion(QString title, int qId , int choiceId, int parent,int mode,int length){
    QSqlQuery query;
 //   qDebug()<<questionPanel::minValue<<" ---- "<<questionPanel::maxValue;
    if(choiceId!=-1){
    query.exec("insert or replace into sessionQuestions (ID,questionID,sessionID,"
               "choiceno,text,parent,mode,validityTime,minVal,maxVal) "
               "VALUES((select count(ID)+1 from sessionquestions),"
               +QString::number(qId)+","+
               QString::number(database::sessionID)+","+
               QString::number(choiceId)+",'"+title+"',"+QString::number(parent)+
               +","+QString::number(mode)+","+QString::number(length)+","
               +QString::number(questionPanel::minValue)+","
               +QString::number(questionPanel::maxValue)+")");
    } else{
        query.exec("insert or replace into sessionQuestions (ID,questionID,sessionID,"
                   "choiceno,text,parent,mode,validityTime,minVal,maxVal) VALUES((select count(ID)+1 from sessionquestions),"
                   +QString::number(qId)+","+
                   QString::number(database::sessionID)+","+
                   "(select max(choiceno)+1 from sessionquestions where questionID="+QString::number(qId)
                   +" and sessionID="+QString::number(sessionID)+" ),'"+title+"',"+QString::number(parent)+
                   +","+QString::number(mode)+","+QString::number(length)+","
                   +QString::number(questionPanel::minValue)+","+QString::number(questionPanel::maxValue)
                   +")");
        }
}

void database::deleteQuestion(int qId, int sId){
    QSqlQuery query;
    query.exec("delete from sessionQuestions where questionID="+QString::number(qId)+
               " and sessionID="+QString::number(sId));
}

void database::deleteChoice(int qId, int sId, int choiceno){
    QSqlQuery query;
    query.exec("delete from sessionQuestions where questionID="+QString::number(qId)+
               " and sessionID="+QString::number(sId)+" and parent=1 and choiceno="
               +QString::number(choiceno));
}

void database::transferOneStep(int id, int dir){
    QSqlQuery query;
    if(dir==1){
        id++;
    query.exec("update sessionquestions set questionID=(select max(questionID)+1 "
               "from sessionquestions where "
               " sessionID="+QString::number(database::sessionID)+") where sessionID="+
               QString::number(database::sessionID)+" and questionID="+QString::number(id));

    query.exec("update sessionquestions set questionID="+QString::number(id)+" where questionID="
               +QString::number(id-1)+" and sessionID="+
               QString::number(database::sessionID));

    if(!query.exec("update sessionquestions set questionID="+QString::number(id-1)+" where "
               "questionID=(select max(questionID) from sessionquestions where sessionID="+
               QString::number(database::sessionID)+") and sessionID="
               +QString::number(database::sessionID))){
     }


    } else if(dir==-1){
        id--;
    query.exec("update sessionquestions set questionID=(select max(questionID)+1 "
               "from sessionquestions where "
               " sessionID="+QString::number(database::sessionID)+") where sessionID="+
               QString::number(database::sessionID)+" and questionID="+QString::number(id));

    query.exec("update sessionquestions set questionID="+QString::number(id)+" where questionID="
               +QString::number(id+1)+" and sessionID="+
               QString::number(database::sessionID));
    query.exec("update sessionquestions set questionID="+QString::number(id+1)+" where "
               "questionID=(select max(questionID) from sessionquestions where sessionID="+
               QString::number(database::sessionID)+") and sessionID="
               +QString::number(database::sessionID));

    }
}

int database::returnumberofchoices(int sessionID, int questionID){
    QSqlQuery query;
    query.exec("select count(choiceno) from sessionquestions where sessionID="
               +QString::number(sessionID)+" and questionID="+QString::number(questionID)+
               " and parent=1");
    while(query.next()){

        return query.value(0).toInt();
    }
}

void database::returnNumberOfReadies(int id, int &readies, int qId, int &answerers){

    QSqlQuery query;
    query.exec("select count(distinct voterID) from sessionvotes where sessionID="
               +QString::number(id));
    while(query.next()){

        readies=query.value(0).toInt();
    }
    query.exec("select count(distinct voterID) from sessionvotes where sessionID="
               +QString::number(id)+" and questionID="+QString::number(qId));
    while(query.next()){

        answerers=query.value(0).toInt();
    }
}

void database::selectVotersToCurrentQuestion(int qID, std::vector<int> &voters){
    QSqlQuery query;
    query.exec("select distinct voterID from sessionvotes where questionID="+QString::number(qID)
               +" and sessionID="+QString::number(sessionID));
    while(query.next()){
        voters.push_back(query.value(0).toInt());
    }
}
