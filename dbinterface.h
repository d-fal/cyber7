#ifndef DBINTERFACE_H
#define DBINTERFACE_H

#include <QThread>

class dbInterface : public QThread
{
    Q_OBJECT
public:
    explicit dbInterface(QObject *parent = 0);

signals:

public slots:

};

#endif // DBINTERFACE_H
