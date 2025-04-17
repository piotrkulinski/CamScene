#include "PolygonItem.h"
#include "polygonitem.h"
#include <qpen.h>
#include <qpolygon.h>

QVariant PolygonHandle::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange && parentItem()) {
        auto* polygonItem = dynamic_cast<PolygonItem*>(parentItem());
        if (polygonItem)
            polygonItem->updatePolygon();
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}

PolygonItem::PolygonItem(const QPolygonF& polygon)
    : QGraphicsPolygonItem(polygon)
{
    setPen(QPen(Qt::red, 2));
    setBrush(QBrush(QColor(255, 0, 0, 50)));

    for (const QPointF& point : polygon) {
        auto* handle = new PolygonHandle(this);
        handle->setPos(point);
        handles.append(handle);
    }
}

void PolygonItem::updatePolygon()
{
    QPolygonF newPoly;
    for (auto* handle : handles)
        newPoly << handle->pos();

    setPolygon(newPoly);
}
