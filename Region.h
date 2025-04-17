#ifndef REGION_H
#define REGION_H

#include <QGraphicsRectItem>
#include <qDebug>

class RegionA : public QGraphicsRectItem
{
public:
    RegionA(const QRectF &rect) : QGraphicsRectItem(rect) {
        setFlag(QGraphicsItem::ItemIsSelectable);
        setFlag(QGraphicsItem::ItemIsMovable);
        setAcceptHoverEvents(true);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        qDebug() << "Kliknięto prostokąt!";
        QGraphicsRectItem::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
        qDebug() << "Przeciągasz prostokąt!";
        QGraphicsRectItem::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        qDebug() << "Puszczono przycisk myszy!";
        QGraphicsRectItem::mouseReleaseEvent(event);
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override {
        qDebug() << "Najechano myszką!";
        QGraphicsRectItem::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override {
        qDebug() << "Opuściliśmy prostokąt!";
        QGraphicsRectItem::hoverLeaveEvent(event);
    }
};

#endif // REGION_H
