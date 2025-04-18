#ifndef REGION_H
#define REGION_H

#include <cfloat>
#include "opencv2/opencv.hpp"
#include "IFrameAnalyzer.h"
#include "MotionDetector.h"
#include "regioncaption.h"

#include <QApplication>
#include <QBrush>
#include <QCursor>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
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
        std::unique_lock <std::timed_mutex> lock(m_mutex); // chroni m_running i m_ready
        if (!m_running) {
            m_running = true;
            m_ready = false;
            if (m_thread.joinable()) {
                m_thread.join();
            }
            m_thread = std::thread(&RegionA::run, this);
            setActive(m_running);
        }
    }

    void stopMonitoring() {
        {
            std::unique_lock <std::timed_mutex> lock(m_mutex);
            m_running = false;
            m_ready = true; // obudzenie wątku, żeby się zakończył
            setActive(m_running);
        }
        m_cv.notify_one();
        if (m_thread.joinable()) {
            m_thread.join();
        };
    }

    void run() {
        std::unique_lock<std::timed_mutex> lock(m_mutex);
        while (m_running) {
            m_cv.wait(lock, [this] { return m_ready || !m_running; });

            if (!m_running)
                break;

            if (!m_currentFrame.empty()) {
                analyzer->analyze(base_frame,m_currentFrame);
                base_frame = m_currentFrame.clone(); // kopiujemy tylko ROI
            }

            m_ready = false;
        }
    }

    // Ustawienie rodzica, który może zmieniać rozmiar RegionA
    void setParentItem(QGraphicsPixmapItem *parent) {
        QGraphicsRectItem::setParentItem(parent); // Ustawiamy rodzica

        // Teraz możemy uzyskać rozmiar rodzica
        if (parent && parent->type() == QGraphicsPixmapItem::Type) {
            QGraphicsPixmapItem *pixmapItem =
                qgraphicsitem_cast<QGraphicsPixmapItem *>(parent);
            if (pixmapItem) {
                QSizeF parentSize = pixmapItem->pixmap().size();
                qDebug() << parentSize;
                // Możesz teraz używać parentSize, aby obliczać coś w oparciu o ten
                // rozmiar
            }
        }
    }

    QGraphicsPixmapItem *getPixmapItem() {
        // Uzyskanie obiektu pixmapy, na którym znajduje się RegionA
        QGraphicsItem *parentItem = this->parentItem();

        // Sprawdzenie, czy to faktycznie QGraphicsPixmapItem
        if (parentItem && parentItem->type() == QGraphicsPixmapItem::Type) {
            return qgraphicsitem_cast<QGraphicsPixmapItem *>(parentItem);
        }

        return nullptr; // Jeśli nie jest to QGraphicsPixmapItem
    }

    QRectF getImageRegion(const cv::Mat &image) {
        QGraphicsPixmapItem *pixmapItem = getPixmapItem();

        if (!pixmapItem) {
            qWarning() << "Nie znaleziono pixmapy na scenie!";
            return QRectF(); // Zwróć pusty prostokąt, jeśli pixmapa nie istnieje
        }

        // Rozmiar pixmapy na scenie
        QSizeF sceneSize = pixmapItem->boundingRect().size();

        // Rozmiar oryginalnego obrazu (cv::Mat)
        QSizeF imageSize(image.cols, image.rows);

        // Oblicz skale
        qreal scaleX = imageSize.width() / sceneSize.width();
        qreal scaleY = imageSize.height() / sceneSize.height();

        // Uzyskaj prostokąt regionu z RegionA
        QRectF regionRect = this->mapRectToScene(this->rect());
        // qDebug() << "Region: " << this->caption << " rozmiar: " << regionRect;

        // Oblicz przeskalowany region w obrazie
        QRectF imageRect(regionRect.x() * scaleX, regionRect.y() * scaleY,
                         regionRect.width() * scaleX, regionRect.height() * scaleY);

        return imageRect;
    }

    // void setFrame(const cv::Mat &frame) {
    //     {
    //         std::unique_lock<std::timed_mutex> lock(m_mutex);
    //         frame.copyTo(m_currentFrame); // zachowujemy kopię
    //         m_ready = true;
    //     }
    //     m_cv.notify_one();
    // }

    void setFrameRegion(const cv::Mat &frame) {
        if (frame.empty())
            return;

        std::unique_lock<std::timed_mutex> lock(m_mutex, std::defer_lock);
        if (!lock.try_lock_for(std::chrono::milliseconds(10))) {
            std::cout << "Ramka w trakcie pracy, pomija aktualizację ramki!\n";
            return;
        }

        QRectF qRectF = getImageRegion(frame);
        cv::Rect roiRect(
            static_cast<int>(qRectF.left()), static_cast<int>(qRectF.top()),
            static_cast<int>(qRectF.width()), static_cast<int>(qRectF.height()));

        // Zabezpieczenie przed wyjściem poza ramkę
        roiRect &= cv::Rect(0, 0, frame.cols, frame.rows);
        m_currentFrame = frame(roiRect).clone(); // kopiujemy tylko ROI
        if (base_frame.empty()) {
            base_frame = frame(roiRect).clone(); // kopiujemy tylko ROI
        }
        m_ready = true;
        m_cv.notify_one();
    }

public:
    RegionA(const QRectF &rect, const QString title) : RegionA(rect) {
        caption->setText(title);
    }
    RegionA(const QRectF &rect) : QGraphicsRectItem(rect), resizing(false) {
        setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
        // "this" oznacza, że tekst będzie dzieckiem RegionA
        caption = new RegionCaption("Region", this);

        QPixmap deleteIcon =
            QApplication::style()->standardPixmap(QStyle::SP_TabCloseButton);
        deleteIcon = deleteIcon.scaled(24, 24, Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
        QPixmap editIcon =
            QApplication::style()->standardPixmap(QStyle::SP_DialogOpenButton);
        editIcon =
            editIcon.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        deleteButton = new QGraphicsPixmapItem(deleteIcon, this);
        deleteButton->setToolTip("Usuń region");
        deleteButton->setCursor(Qt::PointingHandCursor);
        deleteButton->setZValue(1); // Na wierzchu

        editButton = new QGraphicsPixmapItem(editIcon, this);

        QPixmap youtubeIcon(":/icons/youtube.ico");

        // QPixmap youtubeIcon =
        // QApplication::style()->standardPixmap(QStyle::SP_MediaPlay);
        youtubeIcon = youtubeIcon.scaled(16, 16, Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
        youtubeButton = new QGraphicsPixmapItem(youtubeIcon, this);
        youtubeButton->setToolTip("Wysyłaj film do youtube");
        youtubeButton->setCursor(Qt::PointingHandCursor);
        youtubeButton->setZValue(1);

        monitorIcon = monitorIcon.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        monitorButton = new QGraphicsPixmapItem(monitorIcon, this);
        monitorButton->setToolTip("Kliknij aby rozpocząć monitorowanie regionu");
        monitorButton->setCursor(Qt::PointingHandCursor);
        monitorButton->setZValue(1); // Na wierzchu
        monitorButton->setData(0,0);

        updateControlsPosition();
        setAcceptHoverEvents(true);

        setActive(false);
    }

    RegionCaption *getCaption() const { return caption; }

protected:
    QInputDialog *editYoutubeKey(QString key = "") {
        QInputDialog *dialog = new QInputDialog((QWidget *)scene()->parent());
        dialog->setLabelText("Podaj klucz strumienia YouTube");
        dialog->setWindowTitle("YouTube - strumieniowanie - " + caption->getText());
        dialog->setTextValue(key);
        dialog->resize(400, 200); // opcjonalnie
        // opcjonalnie ikona
        dialog->setWindowIcon(QIcon(":/icons/youtube.png"));
        // np. styl
        dialog->setStyleSheet(R"(
    QDialog {
        background-color: #2e2e2e;
    }
    QLabel {
        color: white;
        font: 12pt 'Segoe UI';
    }
    QLineEdit {
        background-color: #444;
        font: 12pt 'Segoe UI';
        color: white;
        padding: 4px;
        border-radius: 5px;
        border: 2px solid #666;
    }
    QPushButton {
        background-color: #3a9ad9;
        color: white;
        padding: 5px 10px;
        border-radius: 4px;
    }
    QPushButton:hover {
        background-color: #5bb5f9;
    }
)");
        return dialog;
    }

    void updateControlsPosition() {
        const auto btWidth = deleteButton->boundingRect().width();
        QRectF r = rect();
        caption->setPos(r.left(), r.top() - caption->boundingRect().height() - 3);
        deleteButton->setPos(r.right() - btWidth, r.top());
        editButton->setPos(r.right() - (btWidth * 2), r.top());
        youtubeButton->setPos(r.right() - (btWidth * 3) - 5, r.top());
        monitorButton->setPos(r.right() - (btWidth * 4) - 5, r.top());
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
        QGraphicsRectItem::paint(painter, option, widget);
        // QRectF r = rect();
        QRectF handle(handleRect());
        painter->setBrush(Qt::green);
        painter->drawRect(handle);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        if (youtubeButton->contains(event->pos() - youtubeButton->pos())) {
            IconShade(youtubeButton,true);
            QInputDialog *dialog = editYoutubeKey("82j7-u64q-yvaf-kg0u-760q");
            if (dialog->exec() == QDialog::Accepted) {
                QString value = dialog->textValue();
                qDebug() << "Wartość:" << value;
            }
            return;
        }
        if (editButton->contains(event->pos() - editButton->pos())) {
            setBrush(QBrush(QColor(255, 0, 0, 80)));
            return;
        }
        if (monitorButton->contains(event->pos() - monitorButton->pos())) {
            QVariant d = monitorButton->data(0);
            if (d.toInt() == 0) {
                monitorButton->setData(0,1);
                monitorButton->setToolTip("Trwa monitorowanie regionu, kliknij aby zatrzymać");
                IconShade(monitorButton, monitorIcon, true);
                setPen(QPen(Qt::yellow, 2));
                setBrush(QBrush(QColor(0, 255, 0, 40)));
                {
                    std::unique_lock<std::timed_mutex> lock(m_mutex);
                    analyzer = std::make_shared<MotionDetector>();
                    if (auto md = std::dynamic_pointer_cast<MotionDetector>(analyzer)) {
                        QObject::connect(md.get(), &MotionDetector::onMotionDetected, [&](bool movement) {
                            //qDebug() << "Wykryto ruch w regionie: " << this->getCaption()->getText();
                            this->caption->setPen(QPen(Qt::red, 2));
                            this->caption->setBrush(QBrush(QColor(250,0,0,200)));
                        });
                    }
                }
                startMonitoring();
            } else if (d.toInt() == 1) {
                stopMonitoring();
                monitorButton->setData(0,0);
                monitorButton->setToolTip("Kliknij aby rozpocząć monitorowanie regionu");
                IconShade(monitorButton,monitorIcon,false);
                setPen(QPen(Qt::blue, 2));
                setBrush(QBrush(QColor(255, 0, 0, 30)));
                {
                    std::unique_lock<std::timed_mutex> lock(m_mutex);
                    analyzer.reset();
                }
                this->caption->setDefault();
            }
            return;
        }

        if (deleteButton->contains(event->pos() - deleteButton->pos())) {
            stopMonitoring();
            qDebug() << "Kliknięto przycisk!";
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
        caption->setDefault();
        QGraphicsRectItem::mouseReleaseEvent(event);
    }

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override {
        if (handleRect().contains(event->pos())) {
            setCursor(Qt::SizeFDiagCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }

    QPixmap matToPixmap(const cv::Mat &mat) {
        cv::Mat rgb;

        // Konwersja BGR → RGB (jeśli obraz ma 3 kanały)
        if (mat.type() == CV_8UC3) {
            cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
            return QPixmap::fromImage(QImage(rgb.data, rgb.cols, rgb.rows, rgb.step,
                                             QImage::Format_RGB888));
        }
        // Jeśli obraz ma 1 kanał (np. grayscale)
        else if (mat.type() == CV_8UC1) {
            return QPixmap::fromImage(QImage(mat.data, mat.cols, mat.rows, mat.step,
                                             QImage::Format_Grayscale8));
        }
        // Jeśli obraz ma 4 kanały (np. BGRA)
        else if (mat.type() == CV_8UC4) {
            return QPixmap::fromImage(QImage(mat.data, mat.cols, mat.rows, mat.step,
                                             QImage::Format_ARGB32));
        }

        // Nieobsługiwany typ
        return QPixmap();
    }

private:

    void IconShade(QGraphicsPixmapItem* button, bool isShade=true) {
        QPixmap pressedIcon = button->pixmap();
        QPainter painter(&pressedIcon);
        painter.fillRect(pressedIcon.rect(),QColor(0, 0, 0, (isShade ? 50 : 0))); // prosty efekt przyciemnienia
        button->setPixmap(pressedIcon);
    }
    void IconShade(QGraphicsPixmapItem* button, const QPixmap& originalIcon, bool isShade = true) {
        QPixmap shaded = originalIcon; // zaczynamy zawsze od oryginału
        if (isShade) {
            QPainter painter(&shaded);
            painter.fillRect(shaded.rect(), QColor(0, 0, 0, 50)); // efekt przyciemnienia
        }
        button->setPixmap(shaded);
    }

#pragma region
    std::shared_ptr<IFrameAnalyzer> analyzer;
    std::thread m_thread;
    std::timed_mutex m_mutex;
    std::condition_variable_any m_cv;

    bool m_running = false;
    bool m_ready = false;
    cv::Mat m_currentFrame, base_frame;
#pragma endregion

    bool resizing;
    QPixmap monitorIcon = QApplication::style()->standardPixmap(QStyle::SP_MediaPlay);
    QGraphicsPixmapItem *deleteButton, *youtubeButton, *editButton, *monitorButton;
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
