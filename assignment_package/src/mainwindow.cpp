#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"
#include "debug.h"
#include <QResizeEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QFileDialog>
#include <QProgressDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), cHelp()
{
    ui->setupUi(this);
    ui->mygl->setFocus();
    this->move(0,0);
    this->playerInfoWindow.show();
    // playerInfoWindow.move(QGuiApplication::primaryScreen()->availableGeometry().center() - this->rect().center() + QPoint(this->width() * 0.75, 0));
    playerInfoWindow.move(2000,0);

    connect(ui->mygl, SIGNAL(sig_sendPlayerPos(QString)), &playerInfoWindow, SLOT(slot_setPosText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerVel(QString)), &playerInfoWindow, SLOT(slot_setVelText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerAcc(QString)), &playerInfoWindow, SLOT(slot_setAccText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerLook(QString)), &playerInfoWindow, SLOT(slot_setLookText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerChunk(QString)), &playerInfoWindow, SLOT(slot_setChunkText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerTerrainZone(QString)), &playerInfoWindow, SLOT(slot_setZoneText(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    cHelp.show();
}

void MainWindow::on_actionPlay_Custom_Song_triggered() {
    QString path = QFileDialog::getOpenFileName(this, "Open Custom Song", QDir::homePath(), "Music (*.mp3)");
    if (path.isEmpty()) return;
    ui->mygl->playCustomSong(path);
}

void MainWindow::on_actionLoad_Greyscale_Heightmap_triggered() {
    QString path = QFileDialog::getOpenFileName(this, "Open Greyscale Heightmap", QDir::homePath(), "Images (*.png *.jpg *.bmp)");

    if (path.isEmpty()) return;

    QImage img = QImage(path);
    if (img.isNull()) {
        QMessageBox::warning(this, "Error", "Failed to load image.");
        return;
    }

    LOG("applying greyscale heightmap");
    QProgressDialog progress("Applying greyscale heightmap...", "Cancel",
                             0, img.height(), this);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(0);
    progress.show();

    ui->mygl->applyHeightmap(img, false, &progress);
}

void MainWindow::on_actionLoad_Colored_Heightmap_triggered() {
    QString path = QFileDialog::getOpenFileName(this, "Open Colored Heightmap", QDir::homePath(), "Images (*.png *.jpg *.bmp)");

    if (path.isEmpty()) return;

    QImage img = QImage(path);
    if (img.isNull()) {
        QMessageBox::warning(this, "Error", "Failed to load image.");
        return;
    }

    LOG("applying colored heightmap");
    QProgressDialog progress("Applying colored heightmap...", "Cancel",
                             0, img.height(), this);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(0);
    progress.show();

    ui->mygl->applyHeightmap(img, true, &progress);

}
