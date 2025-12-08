#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "cameracontrolshelp.h"
#include "playerinfo.h"


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionQuit_triggered();
    void on_actionLoad_Greyscale_Heightmap_triggered();
    void on_actionLoad_Colored_Heightmap_triggered();
    void on_actionPlay_Custom_Song_triggered();

    void on_actionCamera_Controls_triggered();


private:
    Ui::MainWindow *ui;
    CameraControlsHelp cHelp;
    PlayerInfo playerInfoWindow;
};


#endif // MAINWINDOW_H
