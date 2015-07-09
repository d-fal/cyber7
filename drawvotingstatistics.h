#ifndef DRAWVOTINGSTATISTICS_H
#define DRAWVOTINGSTATISTICS_H

#include <QDialog>
#include "home.h"
namespace Ui {
class drawVotingStatistics;
}

class drawVotingStatistics : public QDialog
{
    Q_OBJECT

public:
    explicit drawVotingStatistics(QWidget *parent = 0);
    ~drawVotingStatistics();



private:
    QPixmap *pix;
    QPainter *paint;
    Ui::drawVotingStatistics *ui;
    int heightX;
    int widthX;
    int voteBuffer[100];
    QString choiceTexts[100];
private slots:
    void updatedrawing();
};

#endif // DRAWVOTINGSTATISTICS_H
