#ifndef QUESTIONPANEL_H
#define QUESTIONPANEL_H

#include <QDialog>
#include <QStandardItemModel>
#include <QMouseEvent>
#include "home.h"
namespace Ui {
class questionPanel;
}

class questionPanel : public QDialog
{
    Q_OBJECT

public:
    explicit questionPanel(QWidget *parent = 0);
    ~questionPanel();
    static int mode;
    static QString questionTitle;
    static int numberOfOptions;
    static int availableChoices;
    static bool isQuestionSelected;
    static QString mode_text;
    static int activeQuestionID;
    static int questionLength;
    static int minValue;
    static int maxValue;

private slots:
    void on_commandLinkButton_clicked();

    void on_saveChoice_clicked();

    void on_questionsTable_clicked(const QModelIndex &index);

    void on_edit_clicked();

    void on_delete_2_clicked();

    void on_choicesTable_clicked(const QModelIndex &index);

    void on_addnewChoice_clicked();

    void on_deleteChoice_clicked();

    void on_goUp_clicked();

    void on_goDown_clicked();

    void on_questionsTable_doubleClicked(const QModelIndex &index);

    void on_manualValidityTime_clicked();

    void on_autoValidityTime_clicked();

    void on_oneTo100Avg_clicked();

    void on_oneTo100_clicked();

    void on_afMultiple_clicked();

    void on_afSingle_clicked();

    void on_maxValue_valueChanged(int arg1);

    void on_minValue_valueChanged(int arg1);

private:
     int questionID;
    int choiceno;
    void updateList();
    void updateChoices();

    QStandardItemModel *questionsModel;
    QStandardItemModel *choicesModel;
    Ui::questionPanel *ui;

signals:
    void changeModeStatus(int);
    void setQuestionActive();
};

#endif // QUESTIONPANEL_H
