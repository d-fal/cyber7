#ifndef SUBMITTEDPEOPLE_H
#define SUBMITTEDPEOPLE_H
#include <QLabel>
#include <QDialog>
#include "home.h"
namespace Ui {
class submittedPeople;
}

class submittedPeople : public QDialog
{
    Q_OBJECT

public:
    explicit submittedPeople(QWidget *parent = 0);
    ~submittedPeople();

private:
    QPixmap *pix;
    QPainter *paint;
    QPixmap *pixTop;
    QPainter *paintTop;
    Ui::submittedPeople *ui;
    static int width;
    static int height;
    static int totalReadies;
    static int totalQuestionVotes;
    static int screenWidth;
    static int screenHeight;
    int maxVoterno;
    QWidget *win ;
    int page;
    QTimer *timer4;

    QLabel *readiesBoard;
private slots:
    void update();
    void on_pushButton_clicked();
    void nextPage();
};

#endif // SUBMITTEDPEOPLE_H
