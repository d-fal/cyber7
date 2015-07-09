#include "sessionmanagement.h"
#include "ui_sessionmanagement.h"
#include "database.h"

int sessionManagement::selectedRow;

sessionManagement::sessionManagement(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sessionManagement)
{
    ui->setupUi(this);
    model = new QStandardItemModel(1,3,this);
    this->setWindowTitle("Session manager");
    updateList();
    ui->sessionsInfo->setModel(model);
    ui->sessionsInfo->setColumnWidth(1,200);

}

sessionManagement::~sessionManagement()
{
    delete ui;
}

void sessionManagement::on_sessionsInfo_clicked(const QModelIndex &index)
{

    QModelIndexList selected = ui->sessionsInfo->selectionModel()->selectedIndexes();
    firstItem=model->item(selected[0].row(),0)->text().toInt();
    if(selected.length()>1){
    secondItem=model->item(selected[1].row(),0)->text().toInt();

    secondItem=qMax(firstItem,secondItem);
    }

    //if(selected.length()>2) updateList();

    if(index.column()>=0){
    QVariant v((model->itemData(index))[0]);
    selectedRow=model->item(index.row(),0)->text().toInt();

    //qDebug()<<v.toString();
    }
}

void sessionManagement::on_saveBtn_clicked()
{

    QString values = "update sessioninfo set sessionTitle=case " ;
    for ( int i = 0 ; i < model->rowCount() ; ++i )
    {
      // Get item at row i, col 0.

        values+= "when sessionID="+model->index( i, 0 ).data( Qt::DisplayRole ).toString()
                +" then '"+model->index( i, 1 ).data( Qt::DisplayRole ).toString()+"' ";

    }
    values=values.left(values.length()-1)+" end";
    QSqlQuery query;
    //qDebug()<<values;
    query.exec(values);


}

void sessionManagement::on_sessionsInfo_doubleClicked(const QModelIndex &index)
{
     database::updateSessionData(model->item(index.row(),0)->text().toInt());
     updateList();
     QMessageBox::StandardButton reply;
     reply = QMessageBox::question(this, "Activate", database::sessionTitle,
                                     QMessageBox::No|QMessageBox::Yes);
     if(reply==QMessageBox::Yes) this->close();
}

void sessionManagement::on_removeBtn_clicked()
{
    QMessageBox::StandardButton reply;
         reply = QMessageBox::question(this, "Are you sure?",
                                       "The corresponding votes and activities will be lost."
                                       " Do you still want to do it?",
                                         QMessageBox::No|QMessageBox::Yes);
         if(reply==QMessageBox::No) return;
    QModelIndexList selected = ui->sessionsInfo->selectionModel()->selectedIndexes();
    for(int i=0;i<selected.length();++i){
       database::removeItem(model->item(selected[i].row(),0)->text().toInt());
    }
    //database::removeItem(selectedRow);
    updateList();
}

void sessionManagement::updateList(){
    QSqlQuery query;
    model->clear();
    int ind=0;

   ui->sessionsInfo->setSelectionMode(QAbstractItemView::ExtendedSelection);
    model->setHorizontalHeaderItem(0, new QStandardItem("Session ID"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Session Title"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Date"));
    model->setHorizontalHeaderItem(3, new QStandardItem("questions"));
    ui->sessionsInfo->setColumnWidth(1,200);
    query.exec("select  distinct A.sessionID,A.sessionTitle,A.time,B.cnt from sessioninfo as A"
               " left join (select sessionID,count(questionID) as cnt from sessionquestions "
               " where parent=0 group by sessionID) AS B "
               "on A.sessionID=B.sessionID group by A.sessionID order by A.time desc");
    while (query.next()) {
        QStandardItem *rowContent = new QStandardItem[20];
        for (int i=0;i<4;++i){
        rowContent= new QStandardItem(query.value(i).toString());
        model->setItem(ind,i,rowContent);
        }
           ind++;
    }
}

void sessionManagement::on_addBtn_clicked()
{
    database::addItem();
    updateList();
}

void sessionManagement::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::RightButton)
        {
   updateList();
        }
        }

void sessionManagement::on_mergeBtn_clicked()
{
    database::mergeItems(firstItem,secondItem);
    updateList();
}
