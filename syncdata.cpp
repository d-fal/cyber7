#include "syncdata.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGridLayout>
#include "database.h"
#include "networkmanager.h"

QString syncData::serverResponseCode;
bool syncData::isGenuineServer;
syncData::syncData(QObject *parent) :
    QObject(parent)
{

}

bool syncData::sendVotersList(){
    if(!home::isGenuine) {return false;}

    QSqlQuery query;
     QString data,votes;
    query.exec("select * from sessionAttendants where id="+QString::number(database::sessionID));

    while(query.next()){
        data+="(\""+query.value(1).toString()+"\" , "+QString::number(database::sessionID)
                +","+query.value(2).toString()+" , now()),";
    }
    data=data.left(data.length()-1);
    query.exec("select voterID,sessionID,questionID,vote,questionMode from sessionvotes where "
               "sessionID="+QString::number(database::sessionID));

    while(query.next()){
        votes+="(\""+query.value(0).toString()+"\" , "+QString::number(database::sessionID)
                +","+query.value(2).toString()+","+query.value(3).toString()+
                ","+query.value(4).toString()+" , now() ,\""+home::systemCode+"\"),";
    }
    votes=votes.left(votes.length()-1);
    //qDebug()<<votes;
    QEventLoop eventLoop;
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart textPart,textPart2;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"rollcallData\""));
    textPart.setBody(data.toLatin1());
    textPart2.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"voterData\""));
    textPart2.setBody(votes.toLatin1());

    multiPart->append(textPart);
    multiPart->append(textPart2);
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkRequest req( QUrl( networkManager::serverAddress+QString("/recieveVotersData.php") ) );
    QNetworkReply *reply = mgr.post(req,multiPart);


    multiPart->setParent(reply);
      eventLoop.exec(); // blocks stack until "finished()" has been called
    if (reply->error() == QNetworkReply::NoError) {
        //success
        QString strx=reply->readAll();
        //qDebug()<<strx;
        QStringList propertyNames;
        QStringList propertyKeys;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strx.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();
        QJsonArray jsonArray = jsonObject["APPLE"].toArray();

        foreach (const QJsonValue & value, jsonArray) {
            QJsonObject obj = value.toObject();
            propertyNames.append(obj["Name"].toString());
            propertyKeys.append(obj["ID"].toString());
        }
        delete reply;
        return true;
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;
        return false;

    }

}

bool syncData::checkGenuiness(QString code){

    QEventLoop eventLoop;
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart textPart,textPart2;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"isGenuine\""));
    textPart.setBody(code.toLatin1());

    multiPart->append(textPart);

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkRequest req( QUrl( networkManager::serverAddress+QString("/recieveVotersData.php") ) );
    QNetworkReply *reply = mgr.post(req,multiPart);
    multiPart->setParent(reply);
      eventLoop.exec(); // blocks stack until "finished()" has been called
    if (reply->error() == QNetworkReply::NoError) {
        //success
        QString strx=reply->readAll();
        //qDebug()<<strx;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(strx.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();

        QJsonObject obj=jsonObject["code"].toObject();


       serverResponseCode=obj.value("response").toString();
       isGenuineServer=obj.value("validity").toString().toInt()>0;

        return isGenuineServer;
     //   qDebug()<<propertyKeys;
        delete reply;
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;
        return false;
    }
}

void syncData::syncAttendants(QString list){
    QEventLoop eventLoop;
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart textPart,textPart2;

    textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"syncDataReq\""));
    textPart.setBody("000988427");
    multiPart->append(textPart);
    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkRequest req( QUrl( networkManager::serverAddress+QString("/syncAttendants.php") ) );
    QNetworkReply *reply = mgr.post(req,multiPart);
    multiPart->setParent(reply);
      eventLoop.exec(); // blocks stack until "finished()" has been called
    if (reply->error() == QNetworkReply::NoError) {
        //success
        QString strx=reply->readAll();
        //qDebug()<<strx;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strx.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();
        // uniqueID , sessionID , firstName, lastName, remoteID(attendantID) , level
        QJsonObject obj=jsonObject["list"].toObject();
        QString list;
       // qDebug()<<obj;

        for(int i=1; i<obj.size();++i){
                     list+="(\""+obj.value(QString::number(i+1)).toArray().at(2).toString()+"\","+
                    QString::number(database::sessionID)
                    +",\""+obj.value(QString::number(i+1)).toArray().at(0).toString()+"\",\""+
                      obj.value(QString::number(i+1)).toArray().at(1).toString()+"\"),";

        }//{"list":{"1":["22","32","18"],"2":["21","19","11"],"3":["12","2","9"]}}"
        delete reply;
        list=list.left(list.length()-1);
        list="insert or replace into attendantsInfo (uniqueID,sessionID,firstName,lastName) values "
                +list;
        qDebug()<<list;
        QSqlQuery query;
        if(!query.exec(list)){
            qDebug()<<query.lastError();
        }
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;
    }

}
bool syncData::sendQuestionsToserver(int qID,int sessionID){
    if(!home::isGenuine) {return false;}

    QSqlQuery query;
    QString data="",sessionInfo="";
    query.exec("select questionID,sessionID,choiceno,text,parent,mode,validityTime,minVal,maxVal "
               " from sessionquestions where "
               "sessionID="+QString::number(sessionID)
               );


    while(query.next()){
        data+="("+query.value(0).toString()+","+query.value(1).toString()+" , "
                +query.value(2).toString()+", \""+query.value(3).toString()+"\","
                +query.value(4).toString()+" , "+query.value(5).toString()+" , -1"+
                +",\""+home::systemCode+"\","+QString::number(query.value(7).toInt())+","
                +QString::number(query.value(8).toInt())+"),";
    }
    data=data.left(data.length()-1);

    query.exec("select * from sessionInfo where sessionID="+QString::number(sessionID));
    while(query.next()){
        sessionInfo+="("+query.value(0).toString()+",\""+query.value(1).toString()+"\" , \""
                +query.value(2).toString()+"\",\""+home::systemCode+"\"),";
    }
    sessionInfo=sessionInfo.left(sessionInfo.length()-1);
    //qDebug()<<sessionInfo;
    QEventLoop eventLoop;
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart textPart,textPart2;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"sendQuestions\""));
    textPart.setBody(data.toUtf8());

    textPart2.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"sendSessionInfo\""));
    textPart2.setBody(sessionInfo.toUtf8());

    multiPart->append(textPart);
    multiPart->append(textPart2);
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkRequest req( QUrl( networkManager::serverAddress+QString("/recieveVotersData.php") ) );
    QNetworkReply *reply = mgr.post(req,multiPart);


    multiPart->setParent(reply);
    eventLoop.exec(); // blocks stack until "finished()" has been called
    if (reply->error() == QNetworkReply::NoError) {
        //success
        QString strx=reply->readAll();
        qDebug()<<strx;
        QStringList propertyNames;
        QStringList propertyKeys;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strx.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();
        QJsonArray jsonArray = jsonObject["APPLE"].toArray();

        delete reply;
        return true;
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;
        return false;

    }

}

void syncData::syncQuestions(){
    QEventLoop eventLoop;
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart textPart;

    textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"syncQuestsReq\""));
    textPart.setBody("000988427");
    multiPart->append(textPart);
    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkRequest req( QUrl( QString("http://localhost/kanoon/project2/syncQuestions.php") ) );
    QNetworkReply *reply = mgr.post(req,multiPart);
    multiPart->setParent(reply);
      eventLoop.exec(); // blocks stack until "finished()" has been called
    if (reply->error() == QNetworkReply::NoError) {
        //success
        QString strx=reply->readAll();
       //qDebug()<<strx;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strx.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();
// uniqueID , sessionID , firstName, lastName, remoteID(attendantID) , level
        QJsonObject obj=jsonObject["list"].toObject();
        QString list;
        //qDebug()<<obj;
        for(int i=0; i<obj.size();++i){
            list+="("+obj.value(QString::number(i+1)).toArray().at(0).toString()+","+
           obj.value(QString::number(i+1)).toArray().at(1).toString()
           +","+obj.value(QString::number(i+1)).toArray().at(2).toString()
           +",\""+obj.value(QString::number(i+1)).toArray().at(3).toString()+"\",\""+
             obj.value(QString::number(i+1)).toArray().at(4).toString()+"\",\""+
             obj.value(QString::number(i+1)).toArray().at(5).toString()+"\",\""+
             obj.value(QString::number(i+1)).toArray().at(6).toString()+"\",\""+
             obj.value(QString::number(i+1)).toArray().at(7).toString()+"\",\""+
             obj.value(QString::number(i+1)).toArray().at(8).toString()+"\",\""+
             obj.value(QString::number(i+1)).toArray().at(9).toString()+"\""
                    "),";

        }//{"list":{"1":["22","32","18"],"2":["21","19","11"],"3":["12","2","9"]}}"
        delete reply;
        list=list.left(list.length()-1);
        list="insert or replace into sessionQuestions (questionID,sessionID,availableChoices"
                ",question,option1,option2,option3,option4,option5,option6) values "
                +list;
        qDebug()<<list;
        QSqlQuery query;
        if(!query.exec(list)){
            qDebug()<<query.lastError();
        }
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;
    }
}

void syncData::syncRemotes(){
    QEventLoop eventLoop;
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart textPart,textPart2;

    textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"syncRemotesBack\""));
    textPart.setBody("000988427");
    multiPart->append(textPart);
    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkRequest req( QUrl( QString("http://localhost/kanoon/project2/syncRemotes.php") ) );
    QNetworkReply *reply = mgr.post(req,multiPart);
    multiPart->setParent(reply);
      eventLoop.exec(); // blocks stack until "finished()" has been called
    if (reply->error() == QNetworkReply::NoError) {
        //success
        QString strx=reply->readAll();
       qDebug()<<strx;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strx.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();
// uniqueID , sessionID , firstName, lastName, remoteID(attendantID) , level
        QJsonObject obj=jsonObject["list"].toObject();
        QString list;
       // qDebug()<<obj;

        for(int i=0; i<obj.size();++i){
          list="update AttendantsInfo set attendantID="+
                  obj.value(QString::number(i+1)).toArray().at(1).toString()+
                  " where uniqueID=\""
          +obj.value(QString::number(i+1)).toArray().at(0).toString()+"\"";
          //qDebug()<<list;
          QSqlQuery query;
          if(!query.exec(list)){
              qDebug()<<query.lastError();
          }
          list.clear();

        }//{"list":{"1":["22","32","18"],"2":["21","19","11"],"3":["12","2","9"]}}"
        delete reply;
        list=list.left(list.length()-1);
        list="insert or replace into attendantsInfo (uniqueID,sessionID,firstName,lastName) values "
                +list;
        //qDebug()<<list;

    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;
    }
}
