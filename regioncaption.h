#ifndef REGIONCAPTION_H
#define REGIONCAPTION_H

#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QPainter>

class RegionCaption : public QGraphicsItem
{
public:
    RegionCaption(const QString &text, QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
    {
        m_text = new QGraphicsTextItem(text, this);
        m_text->setDefaultTextColor(Qt::white);
        m_text->setFont(QFont("Arial", 12, QFont::Bold));

        m_margin = 6;
    }

    QRectF boundingRect() const override {
        QRectF textRect = m_text->boundingRect();
        return QRectF(textRect.topLeft() - QPointF(m_margin, m_margin),
                      textRect.size() + QSizeF(2 * m_margin, 2 * m_margin));
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override {
        //painter->setBrush(QColor(30, 30, 150));  // Tło
        painter->setBrush(QBrush(QColor(30, 30, 150, 90)));
        painter->setPen(QPen(Qt::yellow, 1));    // Obramowanie
        painter->drawRoundedRect(boundingRect(), 6, 6);
    }

    void setText(const QString &text) {
        m_text->setPlainText(text);
        prepareGeometryChange();
    }

private:
    QGraphicsTextItem *m_text;
    qreal m_margin;
};

#endif // REGIONCAPTION_H
