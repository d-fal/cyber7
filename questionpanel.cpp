#include "questionpanel.h"
#include "ui_questionpanel.h"
#include "database.h"

int questionPanel::mode;
int questionPanel::numberOfOptions;
QString questionPanel::questionTitle;
int questionPanel::availableChoices;
bool questionPanel::isQuestionSelected=false;
QString questionPanel::mode_text;
int questionPanel::activeQuestionID;
int questionPanel::questionLength;
int questionPanel::minValue;
int questionPanel::maxValue;

questionPanel::questionPanel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::questionPanel)
{
     questionsModel= new QStandardItemModel(1,3,this);
     choicesModel  = new QStandardItemModel(1,3,this);
    choiceno=0;
    ui->setupUi(this);
    this->setWindowTitle(database::sessionTitle);
    ui->choicesTable->setModel(choicesModel);
    ui->questionsTable->setModel(questionsModel);
    updateList();


}

questionPanel::~questionPanel()
{
    delete ui;
}

void questionPanel::on_commandLinkButton_clicked()
{
    minValue=ui->minValue->value();
    maxValue=ui->maxValue->value();
    if(ui->afSingle->isChecked())         mode=0;
    else if(ui->afMultiple->isChecked())  mode=1;
    else if(ui->oneTo100->isChecked())    mode=2;
    else if(ui->oneTo100Avg->isChecked()) mode=3;

    if(ui->autoValidityTime->isChecked()) questionLength=ui->questionLength->value()*60;
    else if (ui->manualValidityTime->isChecked()) {
        questionLength=-1;
    }

    QSqlQuery query;
    query.exec("select max(questionID)+1 from sessionQuestions where sessionID="
               +QString::number(database::sessionID)+" and parent=0");
    while(query.next()) questionID=query.value(0).toInt();

    database::addQuestion(ui->questionTitle->text(),questionID,0,0,mode,questionLength);
    updateList();
    ui->delete_2->setEnabled(false);
    ui->edit->setEnabled(false);
}

void questionPanel::updateList(){
     questionsModel->clear();

     QSqlQuery query;


     //ui->questionsTable->setSelectionMode(QAbstractItemView::MultiSelection);
     questionsModel->setHorizontalHeaderItem(0, new QStandardItem("#no"));
     questionsModel->setHorizontalHeaderItem(1, new QStandardItem("title"));
     questionsModel->setHorizontalHeaderItem(2, new QStandardItem("Mode"));
     questionsModel->setHorizontalHeaderItem(3, new QStandardItem("#options"));
     questionsModel->setHorizontalHeaderItem(4, new QStandardItem("#length"));
     ui->questionsTable->setColumnWidth(0,30);

     ui->questionsTable->setColumnWidth(1,120);
     ui->questionsTable->setColumnWidth(3,60);

     query.exec("select  A.questionID,A.text,A.mode , B.cnt,A.validityTime from "
                "sessionquestions  as A "
                "left join (select questionID,sessionID,count(choiceno) as cnt "
                " from sessionquestions where parent=1 group by questionID,sessionID) as B "
                " on A.sessionID=B.sessionID and A.questionID=B.questionID "
                " where A.parent=0 and A.sessionID="+
                QString::number(database::sessionID)+" group by A.questionID,A.sessionID");
     int ind=0;
     while (query.next()) {
         QStandardItem *rowContent = new QStandardItem[20];
         for (int i=0;i<5;++i){
         rowContent= new QStandardItem(query.value(i).toString());
         questionsModel->setItem(ind,i,rowContent);
         }
            ind++;
     }


     //ui->questionsTable->setSelectionMode(QAbstractItemView::MultiSelection);
    updateChoices();

}

void questionPanel::updateChoices(){
     choicesModel->clear();
    QSqlQuery query;
    choicesModel->setHorizontalHeaderItem(0, new QStandardItem("choice"));
    choicesModel->setHorizontalHeaderItem(1, new QStandardItem("title"));
    choicesModel->setHorizontalHeaderItem(2, new QStandardItem("#votes"));
    ui->choicesTable->setColumnWidth(0,80);
    ui->choicesTable->setColumnWidth(1,200);

    query.exec("select A.choiceno,A.text,B.b from sessionquestions as A left join ("
               " select vote,count(vote) as b,questionID,sessionID from sessionvotes "
               "where sessionID="+QString::number(database::sessionID)+" group by vote,questionID "
               ") as B "
               "on A.choiceno=B.vote and A.sessionID=B.sessionID and A.questionID=B.questionID "
               "where  A.parent=1 and A.sessionID="+QString::number(database::sessionID)
               +" and A.questionID="+QString::number(questionID)+
               " order by A.questionID asc");

    int ind=0;
    while (query.next()) {
        QStandardItem *rowContent = new QStandardItem[20];
        for (int i=0;i<3;++i){
        rowContent= new QStandardItem(query.value(i).toString());
        choicesModel->setItem(ind,i,rowContent);
        }
           ind++;
    }
}

void questionPanel::on_saveChoice_clicked()
{
    if(ui->autoValidityTime->isChecked()) questionLength=ui->questionLength->value()*60;
    else if (ui->manualValidityTime->isChecked()) {
        questionLength=-1;
    }

    if(mode<2 && choiceno>6){
        qDebug()<<"choices exeed from permitted number";
        return;

    }
    database::addQuestion(ui->choiceText->text(),questionID,choiceno,1,mode,questionLength);
    choiceno++;
    updateList();
}

void questionPanel::on_questionsTable_clicked(const QModelIndex &index)
{

    questionID=questionsModel->item(index.row(),0)->text().toInt();
    QSqlQuery query;
    query.exec("select text,mode,validityTime,minVal,maxVal from sessionquestions where questionID="
               +QString::number(questionID)+" and sessionID="+QString::number(database::sessionID)+
               " and parent=0");
    while(query.next()){
        ui->questionTitle->setText(query.value(0).toString());
        mode=query.value(1).toInt();
        questionLength=query.value(2).toInt();
        minValue=query.value(3).toInt();
        maxValue=query.value(4).toInt();
        ui->minValue->setValue(minValue);
        ui->maxValue->setValue(maxValue);
    }
    switch (mode) {
    case 0:
        ui->afSingle->setChecked(true);
        ui->maxValue->setEnabled(false);
        ui->minValue->setEnabled(false);
        break;
    case 1:
        ui->afMultiple->setChecked(true);
        ui->maxValue->setEnabled(false);
        ui->minValue->setEnabled(false);
        break;
    case 2:
        ui->oneTo100->setChecked(true);
        ui->maxValue->setEnabled(false);
        ui->minValue->setEnabled(false);
        break;
    case 3:
        ui->oneTo100Avg->setChecked(true);
        ui->maxValue->setEnabled(true);
        ui->minValue->setEnabled(true);
        break;
    default:
        break;
    }
    if(questionLength!=-1){
        ui->autoValidityTime->setChecked(true);
        ui->questionLength->setValue((double)questionLength/60);
        ui->questionLength->setEnabled(true);
    } else{
        ui->manualValidityTime->setChecked(true);
        ui->questionLength->setEnabled(false);
    }

    updateChoices();
    ui->delete_2->setEnabled(true);
    ui->edit->setEnabled(true);
}

void questionPanel::on_edit_clicked()
{
    minValue=ui->minValue->value();
    maxValue=ui->maxValue->value();
    if(ui->autoValidityTime->isChecked()) questionLength=ui->questionLength->value()*60;
    else if (ui->manualValidityTime->isChecked()) {
        questionLength=-1;
    }

    if(ui->afSingle->isChecked()) mode=0;
    else if(ui->afMultiple->isChecked()) mode=1;
    else if(ui->oneTo100->isChecked()) mode=2;
    else if(ui->oneTo100Avg->isChecked()) mode=3;

    database::addQuestion(ui->questionTitle->text(),questionID,0,0,mode,questionLength);
    updateList();
}

void questionPanel::on_delete_2_clicked()
{
    database::deleteQuestion(questionID,database::sessionID);
    updateList();
    ui->questionTitle->clear();
    ui->delete_2->setEnabled(false);
    ui->edit->setEnabled(false);
}

void questionPanel::on_choicesTable_clicked(const QModelIndex &index)
{

    QSqlQuery query;
    choiceno = choicesModel->item(index.row(),0)->text().toInt();
     ui->choiceno->setText("option: "+QString::number(choiceno));
    query.exec("select text from sessionquestions where questionID="
               +QString::number(questionID)+" and sessionID="+QString::number(database::sessionID)+
               " and parent=1 and choiceno="
               +choicesModel->item(index.row(),0)->text());
    while(query.next()){
        QString text=query.value(0).toString();
        ui->choiceText->setText(text);

    }
    ui->deleteChoice->setEnabled(true);
    ui->saveChoice->setEnabled(true);
}

void questionPanel::on_addnewChoice_clicked()
{
    if(ui->autoValidityTime->isChecked()) questionLength=ui->questionLength->value()*60;
    else if (ui->manualValidityTime->isChecked()) {
        questionLength=-1;
    }

    choiceno=database::returnumberofchoices(database::sessionID,questionID);
    qDebug()<<choiceno<<" :: choiceno";
    if(mode!=2 && choiceno>=6){
        QMessageBox msgBox;
        msgBox.setText("Error!\nChoices are exceeding from permitted sum."
                       "\nyou cannot add new choices anymore.\n"
                       "you can delete or edit existing choices or you can change question"
                       "mode to 1-100 to cover more options.");
        msgBox.exec();
        return;

    }
    ui->deleteChoice->setEnabled(false);
    ui->saveChoice->setEnabled(false);
    database::addQuestion(ui->choiceText->text(),questionID,-1,1,mode,questionLength);

    updateList();
}

void questionPanel::on_deleteChoice_clicked()
{
    database::deleteChoice(questionID,database::sessionID,choiceno);
    updateList();
    ui->deleteChoice->setEnabled(false);
    ui->saveChoice->setEnabled(false);
}

void questionPanel::on_goUp_clicked()
{

    database::transferOneStep(questionID,1);
    updateList();
    questionID++;
}

void questionPanel::on_goDown_clicked()
{

    if(questionID<=1) return;
    database::transferOneStep(questionID,-1);
    updateList();
     questionID--;
}

void questionPanel::on_questionsTable_doubleClicked(const QModelIndex &index)
{
    QMessageBox::StandardButton reply;
    int questionIDtmp=questionsModel->item(index.row(),0)->text().toInt();

    int modetmp=questionsModel->item(index.row(),2)->text().toInt();
    int availableChoicestmp=questionsModel->item(index.row(),3)->text().toInt();
    if(availableChoicestmp<1 && modetmp<3){
        QMessageBox msgBox;
        msgBox.setText("This question is not in favor of any choices.");
        msgBox.exec();
        return;

    }

    QString ModeText;
    switch (modetmp) {
    case 0:
        ModeText="A-F single";
        break;
    case 1:
        ModeText="A-F multiple";
        break;
    case 2:
        ModeText="1-100";
    case 3:
        ModeText="Average";
        break;
    default:
        break;
    }

    reply = QMessageBox::question(this, "Activate this question?",
                                  "Do yo want to active this question?\n"+questionTitle
                                  +"\nThis question benefits from "+QString::number(availableChoicestmp)+
                                  " options in "+ModeText+" mode.",
                                    QMessageBox::Yes|QMessageBox::No);
    if(reply==QMessageBox::Yes){
        questionTitle=questionsModel->item(index.row(),1)->text();
        questionID=questionIDtmp;
        mode=modetmp;
        isQuestionSelected=true;
        mode_text=ModeText;
        activeQuestionID=questionIDtmp;
        availableChoices=availableChoicestmp;
        emit changeModeStatus(mode);
        if(questionLength!=-1) emit setQuestionActive();
        this->close();
    }
}

void questionPanel::on_manualValidityTime_clicked()
{
    ui->questionLength->setEnabled(false);
}

void questionPanel::on_autoValidityTime_clicked()
{
    ui->questionLength->setEnabled(true);
}

void questionPanel::on_oneTo100Avg_clicked()
{
    ui->minValue->setEnabled(true);
    ui->maxValue->setEnabled(true);
}

void questionPanel::on_oneTo100_clicked()
{
    ui->minValue->setEnabled(false);
    ui->maxValue->setEnabled(false);
}

void questionPanel::on_afMultiple_clicked()
{
    ui->minValue->setEnabled(false);
    ui->maxValue->setEnabled(false);
}

void questionPanel::on_afSingle_clicked()
{
    ui->minValue->setEnabled(false);
    ui->maxValue->setEnabled(false);
}

void questionPanel::on_maxValue_valueChanged(int arg1)
{
    minValue=ui->minValue->value();
    maxValue=ui->maxValue->value();
}

void questionPanel::on_minValue_valueChanged(int arg1)
{
    minValue=ui->minValue->value();
    maxValue=ui->maxValue->value();
}
