#ifndef QTABLEVIEWEXT_H
#define QTABLEVIEWEXT_H

#include <QObject>
#include <QtWidgets>

class QTableViewExt : public QTableView
{
    Q_OBJECT
public:
    QTableViewExt(QWidget *parent = 0);
private:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    //void keyPressEvent(QKeyEvent *event);
    void dropEvent(QDropEvent * event);
};

#endif // QTABLEVIEWEXT_H
