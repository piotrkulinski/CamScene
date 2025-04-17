#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include "RegionA.h"

class CameraView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CameraView(QWidget *parent = nullptr);
    ~CameraView() override;
    void setImageItem(QGraphicsPixmapItem* item) {
        filmFrame = item;
        fitInView(filmFrame, Qt::KeepAspectRatio);
    }

signals:
    void onAddRegion(const QRectF* rect);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_drawing = false;
    bool m_dragging = false;  // Flaga przesuwania obiektu
    QPoint m_startDrawPos;
    QPoint m_endDrawPos;
    RegionA *m_dragItem = nullptr;
    QPoint m_dragStartPos;
    QGraphicsPixmapItem* filmFrame;
};

#endif // CAMERAVIEW_H
