#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QMdiArea;
QT_END_NAMESPACE

class Graph;
class Operations;
class PlotWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void graphCreated(Graph* g) const;

private:
    QMdiArea* _mdiArea;
    Operations* _operations;

    void createMenu();
    void createDocks();
    void createToolBars();
    void createStatusBar();
    void saveSettings();
    void loadSettings();
    void newProject();
    void newPlot();

    PlotWindow* activePlot() const;
};

#endif // MAINWINDOW_H
