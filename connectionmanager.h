#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QThread>
#include "cserial.h"
class connectionManager : public QThread
{
    Q_OBJECT
public:
    explicit connectionManager(QObject *parent = 0);
    void run();
    void initializeConnection(int port);
    void dataParser(char *lpBuffer,int bytesRead);
    bool openVoting(int mode);
    bool closeVoting();
    void setRemoteID(int &id);
    bool isActiveDevice;
    char szMessage[3];
    int extension;
    CSerial serial;
    bool isMain;
    int Mode;
    static int mainPortno;
    void CloseConnection();
private:
    int portno;

signals:
    void sendSignalToUi(int,QString);
    void sendRemoteCommand(int , int , int);

public slots:

protected:
    void close();

};

#endif // CONNECTIONMANAGER_H
