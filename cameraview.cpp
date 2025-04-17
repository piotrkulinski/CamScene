#include "cameraview.h"
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>

CameraView::CameraView(QWidget *parent) : QGraphicsView(parent) {
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::RubberBandDrag); // Umożliwia rysowanie prostokąta
        // przez przeciąganie
}

CameraView::~CameraView() {}

void CameraView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Przekształć pozycję kursora z widoku do sceny
        QPointF scenePos = mapToScene(event->pos());
        QGraphicsItem *item = scene()->itemAt(scenePos, transform());

        // Jeśli kliknięto w RegionA, nie przechwytuj zdarzenia — RegionA sobie
        // poradzi
        if (!item || item->type() != RegionA::Type) {
            m_drawing = true;
            m_startDrawPos = event->pos();
            m_endDrawPos = m_startDrawPos;
            update();
        }
    }

    // ZAWSZE wywołujemy bazową wersję, żeby zdarzenie mogło trafić do itemów
    QGraphicsView::mousePressEvent(event);
}

void CameraView::mouseMoveEvent(QMouseEvent *event) {
    if (m_drawing) {
        m_endDrawPos = event->pos();
        update();
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CameraView::mouseReleaseEvent(QMouseEvent *event) {
    if (m_drawing) {
        m_drawing = false;
        m_endDrawPos = event->pos();
        if (filmFrame) {
            QRectF rect(mapToScene(m_startDrawPos), mapToScene(m_endDrawPos));
            rect = rect.normalized(); // Zapewniamy poprawny kierunek prostokąta

            // Ogranicz do rozmiaru obrazu
            QRectF imageBounds = filmFrame->boundingRect();
            rect = rect.intersected(imageBounds);

            // Minimalny rozmiar
            if (rect.width() < 50 || rect.height() < 50)
                return; // Za mały - nie dodajemy

            // Minimalne wymiary 50x50
            // if (rect.width() >= 50 && rect.height() >= 50) {
            RegionA *region = new RegionA(rect);
            region->setPen(QPen(Qt::blue, 2));
            region->setBrush(QBrush(QColor(255, 0, 0, 30)));
            region->setZValue(1); // ponad wideo
            scene()->addItem(region);
            //}

            return; // Nie przekazujemy dalej
    }
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void CameraView::paintEvent(QPaintEvent *event) {
    QGraphicsView::paintEvent(event);

    if (m_drawing) {
        // Rysowanie konturu prostokąta na widoku w czasie rzeczywistym
        QPainter painter(this);
        painter.setPen(QPen(Qt::red, 2)); // Ustawienie koloru konturu
        QRectF rect(mapToScene(m_startDrawPos), mapToScene(m_endDrawPos));
        painter.drawRect(rect); // Rysowanie prostokąta
    }
}
