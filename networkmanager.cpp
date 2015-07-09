#include "networkmanager.h"
#include "ui_networkmanager.h"
#include "syncdata.h"
QString networkManager::serverAddress;

networkManager::networkManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::networkManager)
{

    ui->setupUi(this);
    this->setWindowTitle("network manager");
    QSettings settings("Cyber 7", "Polling device");
    serverAddress=settings.value("server address").toString();
    ui->serverAddress->setText(serverAddress);
    if(settings.value("genuine").toString().toInt()>0){
        ui->activate->setText("This distribution is activated!.\n Your activation code is "
                              +home::systemCode);
        ui->activateBtn->setEnabled(false);
    }else{
        ui->activate->setText("This distribution is not active!.\n Press \"Activate\" button "
                              "if you have obtained one.\n For more information "
                              "contact cyber 7 technical team.");

    }
}

networkManager::~networkManager()
{
    delete ui;
}

void networkManager::on_pushButton_2_clicked()
{

    QSettings settings("Cyber 7", "Polling device");

    serverAddress=ui->serverAddress->text();
    syncData *sync = new syncData;
    QMessageBox msgBox;
    msgBox.setText("Server address :\n"+serverAddress+"\nsaved successfully.\n"
                   "Syncing procedure is about to begin automatically.");
    msgBox.exec();
    settings.setValue("server address",serverAddress);
    sync->sendVotersList();
}

void networkManager::on_activateBtn_clicked()
{
    emit activateDistro();
    this->close();
}
