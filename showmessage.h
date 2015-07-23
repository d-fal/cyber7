#ifndef SHOWMESSAGE_H
#define SHOWMESSAGE_H

#include <QObject>
#include <QTimer>

#include "home.h"
class showMessage : public QObject
{
    Q_OBJECT
public:
    explicit showMessage(QObject *parent = 0);
    bool display(QString msg);

    QPixmap *pix;
    QPainter *paint;

    QGridLayout *layout;
    QLabel *board;
    QWidget *win;
    QTimer *timer1;
    static int screenWidth;
    static int screenHeight;
signals:

public slots:

};

#endif // SHOWMESSAGE_H
