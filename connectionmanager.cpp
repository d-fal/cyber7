#include "connectionmanager.h"
#include "questionpanel.h"
#include "cserial.h"
#include <iomanip>
int connectionManager::mainPortno=0;

connectionManager::connectionManager(QObject *parent) :
    QThread(parent)
{
}

void connectionManager::initializeConnection(int port){
    portno=port;


    if(serial.Open(portno,115200)){

        isActiveDevice=true;
    //emit sendSignalToUi(0,"connection initialized successfully!");
    } else{
        isActiveDevice=false;
        return;}
    szMessage[0] = 0x70;
    szMessage[1] = 0x99;
    szMessage[2] = 0xE9;

    if(mainPortno!=0){
      szMessage[0]=0x40;
      szMessage[1]=0x99;
      szMessage[2]=0xD9;
    }


      serial.SendData(szMessage,3);
      Sleep(100);
      char *lpBuffer = new char[100];

      if(serial.ReadData(lpBuffer , 100)>0){
          if(szMessage[0]==0x70){
          isMain=true;
          extension=0;
          mainPortno=portno;
          emit sendSignalToUi(0,"Main Device(1-400): COM"+QString::number(mainPortno));
          } else if(szMessage[0]==0x40){
          extension=1;
          emit sendSignalToUi(2,"Extensive Device(401-800): COM"+QString::number(portno));
          }
      }

}

void connectionManager::run(){
    char *lpBuffer = new char[30];

    while(true){
        if(serial.ReadDataWaiting()){
            try{
        int bytesRead = serial.ReadData(lpBuffer,30);

        dataParser(lpBuffer,bytesRead);

         } catch(exception ex){
                cout<<ex.what()<<endl;
        }
    } else{
            Sleep(1);
        }
    }
    delete []lpBuffer;

    if(serial.IsOpened()) serial.Close();


}

    void connectionManager::close(){
    serial.Close();

    }

    void connectionManager::dataParser(char *lpBuffer,int bytesRead){
        //cout<<"In #"<<portno<<endl;
        int clientNo,choice;
        clientNo=400*extension;

        if(bytesRead>=4){
            clientNo+= (lpBuffer[1] & 0x0F) + (16*(lpBuffer[0] & 0xFF));
            switch (Mode) {
            case 0:
                choice  = lpBuffer [4] & 0x0F;

                break;
            case 1:
                choice = lpBuffer[4] & 0x7F;
                break;
            case 2:
                choice = lpBuffer[4] & 0x7F;
                break;
            case 3:
                choice = lpBuffer[4] & 0x7F;
                break;
            default:
                break;
            }

      emit sendRemoteCommand(clientNo,choice,Mode);

        }
    }
bool connectionManager::openVoting(int mode){

    int dataLength=0;
    char setOn[9];
    switch (mode) {
    case 0:
        setOn[0]=0x52;  setOn[1]=0x86;  setOn[2]=0xD4;  setOn[3]=0x5A;
        setOn[4]=0x80;  setOn[5]=0xDA;
        setOn[1]=(8<<4)+questionPanel::availableChoices;
        setOn[2]=((8+((setOn[0] & 0xF0)>>4))<<4)+((setOn[0] & 0x0F) ^ (setOn[1] & 0x0F));
        dataLength=6;
        break;
    case 1:
        setOn[0]=0x52;  setOn[1]=0x86;  setOn[2]=0xD4;  setOn[3]=0x5A;
        setOn[4]=0x81;  setOn[5]=0xDB;
        setOn[1]=(8<<4)+questionPanel::availableChoices;
        setOn[2]=((8+((setOn[0] & 0xF0)>>4))<<4)+((setOn[0] & 0x0F) ^ (setOn[1] & 0x0F));
        dataLength=6;
        break;

    case 2:
        setOn[0]=0x53;  setOn[1]=0x80;  setOn[2]=0xD3;  setOn[3]=0x54;
        setOn[4]=0xE4;  setOn[5]=0xB0;  setOn[6]=0x5A;  setOn[7]=0x82;  setOn[8]=0xD8;
        setOn[4]=questionPanel::availableChoices+(8*16);
        setOn[5]=((((setOn[4] & 0xF0)>>4)^((setOn[3] & 0xF0)>>4))<<4)
                 +((setOn[3] & 0x0F) ^ (setOn[4] & 0x0F));
        dataLength=9;
        break;
    case 3:
        setOn[0]=0x53;  setOn[1]=0x80;  setOn[2]=0xD3;  setOn[3]=0x54;
        setOn[4]=0xE4;  setOn[5]=0xB0;  setOn[6]=0x5A;  setOn[7]=0x82;  setOn[8]=0xD8;
        setOn[4]=questionPanel::maxValue+(8*16);
        setOn[5]=((((setOn[4] & 0xF0)>>4)^((setOn[3] & 0xF0)>>4))<<4)
                 +((setOn[3] & 0x0F) ^ (setOn[4] & 0x0F));
        setOn[1]=questionPanel::minValue+(8*16);
        setOn[2]=((((setOn[0] & 0xF0)>>4)^((setOn[1] & 0xF0)>>4))<<4)
                 +((setOn[0] & 0x0F) ^ (setOn[1] & 0x0F));
        dataLength=9;
        break;
    default:
        break;
    }
    Mode=mode;

    serial.SendData(setOn,dataLength);
    Sleep(10);

}

bool connectionManager::closeVoting(){
    char setOn[3] = {0x5B, 0x80 , 0xDB};

    serial.SendData(setOn,sizeof(setOn));
    Sleep(300);

}

void connectionManager::setRemoteID(int &id){
    id%=400;
    int idt=id;
    char command[3];

    for(int i=0;i<3;++i){
     command[i]=(id) & 0x0f;
     id=id>>4;
    }
    command[0]+=16*6;
    command[1]+=16*(8+command[2]);
    command[2]=((command[0] & 0x0f) ^ (command[1] & 0x0f))
            + 16*(6+8+command[2]);
    serial.SendData(command,3);

}

void connectionManager::CloseConnection(){
    serial.Close();
}
