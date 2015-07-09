#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QDialog>

namespace Ui {
class networkManager;
}

class networkManager : public QDialog
{
    Q_OBJECT

public:
    explicit networkManager(QWidget *parent = 0);
    ~networkManager();
    static QString serverAddress;

private slots:
    void on_pushButton_2_clicked();

    void on_activateBtn_clicked();

private:
    Ui::networkManager *ui;

signals:
    void activateDistro();
};

#endif // NETWORKMANAGER_H
