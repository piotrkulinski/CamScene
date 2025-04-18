#include "mainwindow.h"
#include "RegionA.h"
#include "cameraview.h"
#include "polygonitem.h"
#include "ui_CameraSetup.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QImage>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    showMaximized();
    // ui->graphicsView->setStyleSheet("background-color: red;");

    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::white);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setFrameStyle(QFrame::NoFrame);

    pixmapItem = new QGraphicsPixmapItem();
    scene->addItem(pixmapItem);

    rtspUrl = "rtsp://admin:piotr123@192.168.1.154:554/onvif1";
    cap.open(rtspUrl.toUtf8().constData());
    // 🔽 Otwórz wideo
    // cap.open("video.mp4"); // Podmień ścieżkę!

    if (!cap.isOpened()) {
        qWarning("Nie można otworzyć wideo!");
        return;
    }

    fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::nextFrame);
    timer->start(1000 / fps);

    ui->rightMenu->setIconSize(QSize(32, 32));
    // ui->rightMenu->addItem("Konfiguruj");
    ui->rightMenu->addItem(new QListWidgetItem(
        QApplication::style()->standardIcon(QStyle::SP_BrowserReload),
        "Konfiguruj\npołączenie"));
    ui->rightMenu->addItem(new QListWidgetItem(
        QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton),
        "Włącz"));
    ui->rightMenu->addItem(new QListWidgetItem(
        QApplication::style()->standardIcon(QStyle::SP_DockWidgetCloseButton),
        "Wyłącz"));

    // auto icon = QIcon(":/icons/multi.ico"); // qDebug() << "Ikona załadowana?" <<
    // ui->rightMenu->addItem(new QListWidgetItem(icon, "Wyłącz"));

    connect(ui->rightMenu, &QListWidget::itemClicked, this,
            &MainWindow::onRightMenuItemClicked);

    // QListWidget {
    //     background-color: #0078d7;
    // color: white;
    // border: none;
    // padding: 6px 12px;
    //     border-radius: 4px;
    // }

    setStyleSheet(R"(
        QWidget {
            font-family: Segoe UI;
            font-size: 14px;
        }
        QHBoxLayout {
            background-color: #0078d7;
            color: white;
            border: none;
            padding: 6px 12px;
            border-radius: 4px;
        }
        QGridLayout {
            background-color: #0078d7;
            color: white;
            border: none;
            padding: 6px 12px;
            border-radius: 4px;
        }
        QPushButton {
            background-color: #0078d7;
            color: white;
            border: none;
            padding: 6px 12px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #005a9e;
        }
        QPushButton:pressed {
            background-color: #003f6b;
        }
        QDialog {
            background-color: #f5f5f5;
            border: 1px solid #ccc;
        }
)");
}
void MainWindow::onRightMenuItemClicked(QListWidgetItem *item) {
    if (item->text() == "Konfiguruj\npołączenie") {
        showCameraSetupDialog();
    } else if (item->text() == "Wyłącz") {
        if (cap.isOpened()) {
            timer->stop();
            cap.release();
        }
    }else if (item->text() == "Włącz") {
        if (!cap.isOpened()) {
            timer->stop();
            cap.open(rtspUrl.toUtf8().constData());
            if (cap.isOpened()) {
                timer->start(1000 / fps);
            } else {
                QMessageBox::critical(this, "Błąd", "Nie mo żna połączyć się z kamerą");
            }
        }
    }
}
void MainWindow::saveConfiguration(Ui::CameraSetup *setupUi) {
    rtspUrl = "";
    QTextStream stream(&rtspUrl);
    stream << "rtsp://" << setupUi->user->text() << ":"
           << setupUi->password->text() << "@" << setupUi->ip->text() << ":"
           << setupUi->port->text() << "/" << setupUi->stream->text();
    if (cap.isOpened()) {
        cap.release();
        timer->stop();
    }
    cap.open(rtspUrl.toUtf8().constData());
    if (cap.isOpened()) {
        timer->start(1000 / fps);
    } else {
        QMessageBox::critical(this, "Błąd", "Nie mo żna połączyć się z kamerą");
    }
    qDebug() << "rtspUrl: " << rtspUrl;
}

void MainWindow::showCameraSetupDialog() {
    QDialog *dialog = new QDialog(this);
    Ui::CameraSetup *setupUi = new Ui::CameraSetup();

    setupUi->setupUi(dialog); // ładuje UI do QDialoga
    setupUi->udp->setChecked(true);;
    setupUi->ip->setText("192.168.1.154");
    setupUi->port->setText("554");
    setupUi->user->setText("admin");
    setupUi->password->setText("piotr123");
    setupUi->password->setEchoMode(QLineEdit::Password);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(setupUi->buttonBox, &QDialogButtonBox::accepted, this, [this, dialog, setupUi]() {
        saveConfiguration(setupUi);  // Przekazujemy setupUi
    });

    dialog->exec(); // lub dialog->exec() jeśli ma być modalne
}
void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    // if (!pixmapItem->pixmap().isNull()) {
    //     ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
    // }
}

void MainWindow::nextFrame() {
    cv::Mat frame;
    if (!cap.read(frame)) {
        timer->stop();
        return;
    }

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage image(frame.data, frame.cols, frame.rows, frame.step,
                 QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image);

    pixmapItem->setPixmap(pixmap);
    ui->graphicsView->setImageItem(pixmapItem);
    updateFrameForAllRegion(frame);
}

void MainWindow::updateFrameForAllRegion(cv::Mat& frame) {
    // if (!ui->graphicsView->scene()) {
    //     qWarning() << "Brak sceny w QGraphicsView!";
    //     return;
    // }

    // Iteracja po wszystkich elementach w scenie
    for (QGraphicsItem* item : ui->graphicsView->scene()->items()) {
        // Sprawdź, czy element jest typu RegionA
        RegionA* region = dynamic_cast<RegionA*>(item);
        if (region && region->isActive()) {
            region->setFrameRegion(frame);
        }
    }
}
MainWindow::~MainWindow() { delete ui; }
