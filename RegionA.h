#ifndef REGION_H
#define REGION_H

#include "regioncaption.h"

#include <QApplication>
#include <QBrush>
#include <QCursor>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QStyle>
#include <condition_variable>
#include <mutex>
#include <qgraphicsscene.h>
#include <qpainter.h>
#include <thread>

#include <opencv2/core/mat.hpp>

class RegionA : public QGraphicsRectItem {
public:
    void startMonitoring() {
        std::lock_guard<std::mutex> lock(m_mutex);  // chroni m_running i m_ready
        if (!m_running) {
            m_running = true;
            m_ready = false;
            m_thread = std::thread(&RegionA::run, this);
        }
    }

    void stopMonitoring() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_running = false;
            m_ready = true; // obudzenie wątku, żeby się zakończył
        }
        m_cv.notify_one();
        if (m_thread.joinable())
            m_thread.join();
    }

    void run() {

        // QRectF qRectF = this->rect(); // QRectF regionu w scenie
        // cv::Rect roiRect(
        //     static_cast<int>(qRectF.left()),
        //     static_cast<int>(qRectF.top()),
        //     static_cast<int>(qRectF.width()),
        //     static_cast<int>(qRectF.height())
        //     );

        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_running) {
            m_cv.wait(lock, [this] { return m_ready || !m_running; });

            if (!m_running) break;
            if (!m_currentFrame.empty()) {
                // ... analiza regionu roi ...

                // if (roiRect.x >= 0 && roiRect.y >= 0 &&
                //     roiRect.x + roiRect.width <= m_currentFrame.cols &&
                //     roiRect.y + roiRect.height <= m_currentFrame.rows) {

                //     cv::Mat roi = m_currentFrame(roiRect);
                //     // ... analiza regionu roi ...
                // }
            }

            m_ready = false;
        }
    }
    void setFrame(const cv::Mat &frame) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            frame.copyTo(m_currentFrame);  // zachowujemy kopię
            m_ready = true;
        }
        m_cv.notify_one();
    }

    void setFrameRegion(const cv::Mat &frame)
    {
        if (frame.empty())
            return;

        QRectF qRectF = this->rect();
        cv::Rect roiRect(
            static_cast<int>(qRectF.left()),
            static_cast<int>(qRectF.top()),
            static_cast<int>(qRectF.width()),
            static_cast<int>(qRectF.height())
            );

        // Zabezpieczenie przed wyjściem poza ramkę
        roiRect &= cv::Rect(0, 0, frame.cols, frame.rows);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_currentFrame = frame(roiRect).clone();  // kopiujemy tylko ROI
            m_ready = true;
        }
        m_cv.notify_one();
    }

public:
    RegionA(const QRectF &rect) : QGraphicsRectItem(rect), resizing(false) {
        setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
        caption = new RegionCaption("Region", this); // "this" oznacza, że tekst będzie dzieckiem RegionA

        QPixmap deleteIcon =
            QApplication::style()->standardPixmap(QStyle::SP_TabCloseButton);
        deleteIcon = deleteIcon.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmap editIcon = QApplication::style()->standardPixmap(QStyle::SP_DialogOpenButton);
        editIcon = editIcon.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        deleteButton = new QGraphicsPixmapItem(deleteIcon, this);
        deleteButton->setToolTip("Usuń region");
        deleteButton->setCursor(Qt::PointingHandCursor);
        deleteButton->setZValue(1); // Na wierzchu

        editButton = new QGraphicsPixmapItem(editIcon, this);

        // Ustawiamy rozmiar
        // deleteButton->setScale(0.75);
        // editButton->setScale(0.75);

        updateControlsPosition();
        setAcceptHoverEvents(true);
    }

protected:
    void updateControlsPosition() {
        QRectF r = rect();
        caption->setPos(r.left(), r.top() - caption->boundingRect().height() - 3);
        deleteButton->setPos(r.right() - deleteButton->boundingRect().width(),
                             r.top());
        editButton->setPos(r.right() - deleteButton->boundingRect().width() -
                               editButton->boundingRect().width(),
                           r.top());
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override {
        QGraphicsRectItem::paint(painter, option, widget);
        // QRectF r = rect();
        QRectF handle(handleRect());
        painter->setBrush(Qt::green);
        painter->drawRect(handle);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        if (deleteButton->contains(event->pos() - deleteButton->pos())) {
            qDebug() << "Kliknięto przycisk!";
            // Możesz np. emitować sygnał albo usunąć element:
            scene()->removeItem(this);
            delete this;
            return;
        }

        if (handleRect().contains(event->pos())) {
            resizing = true;
            event->accept();
        } else {
            QGraphicsRectItem::mousePressEvent(event);
        }
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
        if (resizing) {
            QPointF diff = event->pos();
            QRectF newRect = QRectF(rect().topLeft(), diff).normalized();
            if (newRect.width() >= 20 && newRect.height() >= 20) {
                setRect(newRect);
                updateControlsPosition();
                update();
            }
            event->accept();
        } else {
            QGraphicsRectItem::mouseMoveEvent(event);
        }
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        resizing = false;
        QGraphicsRectItem::mouseReleaseEvent(event);
    }

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override {
        if (handleRect().contains(event->pos())) {
            setCursor(Qt::SizeFDiagCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }

private:

#pragma region
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;

    bool m_running = false;
    bool m_ready = false;
    cv::Mat m_currentFrame;
#pragma endregion

    bool resizing;
    QGraphicsPixmapItem *deleteButton;
    QGraphicsPixmapItem *editButton;
    RegionCaption *caption;
    QRectF handleRect() const {
        const qreal size = 10.0;
        return QRectF(rect().bottomRight() - QPointF(size, size),
                      QSizeF(size, size));
    }
};
// auto *shadow = new QGraphicsDropShadowEffect;
// shadow->setBlurRadius(10);
// shadow->setOffset(5, 5);
// rect->setGraphicsEffect(shadow);
// auto *colorize = new QGraphicsColorizeEffect;
// colorize->setColor(Qt::red);
// rect->setGraphicsEffect(colorize);
// auto *opacity = new QGraphicsOpacityEffect;
// opacity->setOpacity(0.15);
// rect->setGraphicsEffect(opacity);
// rect->setRotation(45);
// rect->setScale(1.5);
// rect->setPos(100, 100);

// QLinearGradient gradient(0, 0, 100, 100);
// gradient.setColorAt(0, Qt::red);
// gradient.setColorAt(2, Qt::blue);

// rect->setBrush(QBrush(gradient));
// rect->setPen(QPen(Qt::black, 2));

// // 🔽 Dodaj tekst
// QGraphicsTextItem *text = scene->addText("Witaj!", QFont("Arial", 16));
// text->setDefaultTextColor(Qt::yellow);
// text->setPos(160, 155);
// text->setZValue(2);
#endif // REGION_H
