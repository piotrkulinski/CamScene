#ifndef POLYGONITEM_H
#define POLYGONITEM_H

#include <QGraphicsPolygonItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QBrush>
#include <QVector>
#include <QDebug>

class PolygonHandle : public QGraphicsEllipseItem
{
public:
    PolygonHandle(QGraphicsItem* parent = nullptr)
        : QGraphicsEllipseItem(-4, -4, 8, 8, parent) {
        setBrush(Qt::blue);
        setFlag(ItemIsMovable);
        setFlag(ItemSendsGeometryChanges);
        setZValue(1);
    }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};

class PolygonItem : public QGraphicsPolygonItem
{
public:
    PolygonItem(const QPolygonF& polygon);
    void updatePolygon();
private:
    QVector<PolygonHandle*> handles;
    const QPolygonF &QGraphicsPolygonItem;
};

#endif // POLYGONITEM_H
