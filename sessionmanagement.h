#ifndef SESSIONMANAGEMENT_H
#define SESSIONMANAGEMENT_H

#include <QDialog>
#include <QStandardItemModel>
#include <QMouseEvent>
namespace Ui {
class sessionManagement;
}

class sessionManagement : public QDialog
{
    Q_OBJECT

public:
    explicit sessionManagement(QWidget *parent = 0);
    ~sessionManagement();
    static int selectedRow;
    void updateList();
    int firstItem,secondItem;


private slots:
    void on_sessionsInfo_clicked(const QModelIndex &index);

    void on_saveBtn_clicked();

    void on_sessionsInfo_doubleClicked(const QModelIndex &index);

    void on_removeBtn_clicked();

    void on_addBtn_clicked();
    void mousePressEvent(QMouseEvent *event);
    void on_mergeBtn_clicked();

private:
    QStandardItemModel *model;
    Ui::sessionManagement *ui;
};

#endif // SESSIONMANAGEMENT_H
