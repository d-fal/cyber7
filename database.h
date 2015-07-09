#ifndef DATABASE_H
#define DATABASE_H

#include <QThread>
#include <QSqlDatabase>
#include <QtSql>
#include <QSqlQuery>
#include <QDir>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include "home.h"
class database : public QThread
{
    Q_OBJECT
private:

public:
    explicit database(QObject *parent = 0);
    static QSqlDatabase db;
    void run();
    void startSession();
    void endSession();
    static int sessionID;
    static QString sessionTitle;
    static void updateSessionData(int id);
    static void removeItem(int id);
    static void addItem();
    static void mergeItems(int firstItem,int secondItem);
    static void addQuestion(QString title, int qId, int choiceId, int parent, int mode, int length);
    static void deleteQuestion(int qId, int sId);
    static void deleteChoice(int qId, int sessionID, int choiceno);
    static void transferOneStep(int id, int dir);
    static int returnumberofchoices(int sessionID,int questionID);
    static void returnNumberOfReadies(int id, int &readies, int qId, int &answerers);
    static void selectVotersToCurrentQuestion(int qID, std::vector<int> &voters);
signals:
    void illegalVote(int,int);
    void updateDemonstrativePanels();
    void masterCommand(int);
public slots:
    void saveVote(int voterId, int choice, int mode);

};

#endif // DATABASE_H
