#ifndef SYNCDATA_H
#define SYNCDATA_H

#include "home.h"

class syncData : public QObject
{
    Q_OBJECT
public:
    explicit syncData(QObject *parent = 0);
    static QString serverResponseCode;
    static bool isGenuineServer;
signals:

public slots:
    static bool sendVotersList();
    void syncAttendants(QString list);
    void syncQuestions();
    void syncRemotes();
    bool checkGenuiness(QString code);
    bool sendQuestionsToserver(int qID, int sessionID);
};

#endif // SYNCDATA_H
