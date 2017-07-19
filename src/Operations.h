#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <QObject>

class Graph;

class Operations : public QObject
{
    Q_OBJECT

public:
    explicit Operations(QObject *parent = 0);

    void makeRandomSample() const;
    void makeGraphFromClipboard() const;

signals:
    void graphCreated(Graph* g) const;
};

#endif // OPERATIONS_H