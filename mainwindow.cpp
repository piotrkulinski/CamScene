#include "RegionA.h"
#include "cameraview.h"
#include "mainwindow.h"
#include "polygonitem.h"
#include "ui_mainwindow.h"


#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QImage>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //ui->graphicsView->setStyleSheet("background-color: red;");

    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::white);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setFrameStyle(QFrame::NoFrame);

    pixmapItem = new QGraphicsPixmapItem();
    scene->addItem(pixmapItem);

    std::string rtspUrl = "rtsp://admin:piotr123@192.168.1.154:554/onvif1";
    cap.open(rtspUrl);
    // 🔽 Otwórz wideo
    //cap.open("video.mp4"); // Podmień ścieżkę!

    if (!cap.isOpened()) {
        qWarning("Nie można otworzyć wideo!");
        return;
    }
    // connect(scene, &QGraphicsScene::sceneRectChanged, this, [this]() {
    //     if (!pixmapItem->pixmap().isNull()) {
    //         ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
    //     }
    // });

    fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::nextFrame);
    timer->start(1000 / fps);

    rect = new RegionA(QRectF(150, 150, 150, 150));
    rect->setPen(QPen(Qt::red));
    rect->setBrush(QBrush(QColor(255, 0, 0, 30)));
    //rect->setBrush(QBrush(Qt::transparent));
    rect->setZValue(1); // ponad wideo
    scene->addItem(rect);

    // QGraphicsEllipseItem *ellipse = scene->addEllipse(50, 50, 100, 60, QPen(Qt::blue), QBrush(Qt::cyan));
    // ellipse->setFlag(QGraphicsItem::ItemIsMovable);

    // QGraphicsTextItem *label = scene->addText("To jest elipsa");
    // label->setDefaultTextColor(Qt::black);
    // label->setPos(60, 55);
    // label->setZValue(2);

    // // Tworzymy punkty trójkąta
    // QPolygonF triangle;
    // triangle << QPointF(100, 100)   // Punkt A
    //          << QPointF(150, 50)    // Punkt B
    //          << QPointF(200, 100)   // Punkt C
    //          << QPointF(100, 100);  // Zamknięcie konturu

    // // Tworzymy obiekt i dodajemy do sceny
    // QGraphicsPolygonItem *polygonItem = new QGraphicsPolygonItem(triangle);
    // polygonItem->setBrush(QBrush(Qt::yellow));
    // polygonItem->setPen(QPen(Qt::black, 2));
    // polygonItem->setFlag(QGraphicsItem::ItemIsMovable); // Można przesuwać
    // polygonItem->setFlag(QGraphicsItem::ItemIsSelectable); // Można zaznaczyć
    // polygonItem->setZValue(3);
    // scene->addItem(polygonItem);

    // QPolygonF polygon;
    // polygon << QPointF(100, 100) << QPointF(150, 80) << QPointF(200, 120);
    // auto* polyItem = new PolygonItem(polygon);
    // scene->addItem(polyItem);

    ui->rightMenu->setIconSize(QSize(32, 32));
    ui->rightMenu->addItem("Śledź region");
    ui->rightMenu->addItem("Wyłącz region");
    ui->rightMenu->addItem(new QListWidgetItem(QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon), "Strona główna"));
    auto icon = QIcon(":/icons/multi.png"); //qDebug() << "Ikona załadowana?" << !icon.isNull();
    ui->rightMenu->addItem(new QListWidgetItem(icon, "Opcja"));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // if (!pixmapItem->pixmap().isNull()) {
    //     ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
    // }
}

void MainWindow::nextFrame()
{
    cv::Mat frame;
    if (!cap.read(frame)) {
        timer->stop();
        return;
    }

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image);

    pixmapItem->setPixmap(pixmap);
    ui->graphicsView->setImageItem(pixmapItem);
    //ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
 }

MainWindow::~MainWindow()
{
    delete ui;
}
