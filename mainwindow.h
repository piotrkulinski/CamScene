#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <qevent.h>
#pragma once

#include "RegionA.h"

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <ui_CameraSetup.h>
#include <QListWidget>
#include <qmessagebox.h>
#include <opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void resizeEvent(QResizeEvent *event);

    void showCameraSetupDialog();
    void onRightMenuItemClicked(QListWidgetItem *item);

    void saveConfiguration(Ui::CameraSetup *setupUi);
    void updateFrameForAllRegion(cv::Mat &frame);

    void closeEvent(QCloseEvent *event) override {
        auto reply = QMessageBox::question(this, "Zamknij", "Na pewno chcesz wyjść?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            event->accept();  // pozwól na zamknięcie
        } else {
            event->ignore();  // zablokuj zamknięcie
        }
    }
private slots:
    void nextFrame();

private:
    Ui::MainWindow *ui;
    Ui::CameraSetup *setup;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixmapItem;
    QTimer *timer;
    cv::VideoCapture cap;
    int fps;
    QAbstractGraphicsShapeItem *rect;
    QString rtspUrl;
};

#endif // MAINWINDOW_H
