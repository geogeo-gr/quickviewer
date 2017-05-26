#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QtCore>
#include <QtWidgets>

#include "filevolume.h"
#include "exifdialog.h"
#include "pagemanager.h"
#include "shadermanager.h"

class SavedPoint : public QPoint
{
public:
    void start(QPoint ptStart) {
        m_ptStart = ptStart;
        m_ptSaved = QPoint(x(),y());
    }
    void step(QPoint ptStep) {
//        qDebug("before: pt:(%d,%d) step:(%d,%d) start:(%d,%d)",
//               x(),y(),
//               ptStep.x(),ptStep.y(),
//               m_ptStart.x(),m_ptStart.y() );
        setX(m_ptSaved.x() + ptStep.x() - m_ptStart.x());
        setY(m_ptSaved.y() + ptStep.y() - m_ptStart.y());
//        qDebug("after:  pt:(%d,%d)", x(),y());
    }
    QPoint pt() const { return QPoint(x(), y()); }
    void reset() { setX(0);setY(0);}

private:
    QPoint m_ptStart;
    QPoint m_ptSaved;
};

/**
 * @brief The PageGraphicsItem struct
 * contains the informations of a Page
 */
struct PageGraphicsItem
{
    ImageContent Ic;
    /**
     * @brief GrItem
     * Page image is used as a QGraphicsItem. it will be registed to the scene
     */
    QGraphicsItem* GrItem;
    /**
     * @brief Rotate: rotation as digrees
     */
    int Rotate;
    enum Fitting {
        FitCenter,
        FitLeft,
        FitRight
    };
    PageGraphicsItem()
     : Ic(), GrItem(nullptr), Rotate(0){}
    PageGraphicsItem(ImageContent ic, QGraphicsItem* item, int rotate)
        : Ic(ic), GrItem(item), Rotate(rotate){}
    PageGraphicsItem(const PageGraphicsItem& rhs)
        : Ic(rhs.Ic), GrItem(rhs.GrItem), Rotate(rhs.Rotate) {}
    QPoint Offset(int rotateOffset=0) {
        int rot = (Rotate+rotateOffset) % 360;
        switch(rot) {
        case 90:  return QPoint(Ic.Image.height(), 0);
        case 180: return QPoint(Ic.Image.width(), Ic.Image.height());
        case 270: return QPoint(0, Ic.Image.width());
        default:  return QPoint();
        }
    }
    QSize CurrentSize(int rotateOffset=0) {
        int rot = (Rotate+rotateOffset) % 360;
        return rot==90 || rot==270 ? QSize(Ic.Image.height(), Ic.Image.width()) : Ic.Image.size();
    }

    /**
     * @brief setPageLayout set each image on the page
     * @param viewport: the image must be inscribed in the viewport area
     */
    QRect setPageLayoutFitting(QRect viewport, Fitting fitting, int rotateOffset=0) {
        QSize currentSize = CurrentSize(rotateOffset);
        QSize newsize = currentSize.scaled(viewport.size(), Qt::KeepAspectRatio);
        qreal scale = 1.0*newsize.width()/currentSize.width();
        GrItem->setScale(scale);
        QPoint of = Offset(rotateOffset);
        of *= scale;
        GrItem->setRotation(Rotate+rotateOffset);
        QRect drawRect;
        if(newsize.height() == viewport.height()) { // fitting on upper and bottom
            int ofsinviewport = fitting==FitLeft ? 0 : fitting==FitCenter ? (viewport.width()-newsize.width())/2 : viewport.width()-newsize.width();
            drawRect = QRect(QPoint(of.x() + viewport.x() + ofsinviewport, of.y()), newsize);
            GrItem->setPos(drawRect.topLeft());
        } else { // fitting on left and right
            drawRect = QRect(QPoint(of.x() + viewport.x(), of.y() + (viewport.height()-newsize.height())/2), newsize);
            GrItem->setPos(drawRect.topLeft());
        }
        return drawRect;
    }
    QRect setPageLayoutManual(QRect viewport, Fitting fitting, qreal scale, int rotateOffset=0) {
        QSize size = viewport.size();
        QSize currentSize = CurrentSize(rotateOffset);
        QSize newsize = currentSize * scale;
//        newsize *= scale;
        GrItem->setScale(scale);
        QPoint of = Offset(rotateOffset);
        of *= scale;
        GrItem->setRotation(Rotate+rotateOffset);
        int ofsinviewport = fitting==FitLeft ? 0 : fitting==FitCenter ? (viewport.width()-newsize.width())/2 : viewport.width()-newsize.width();
        QRect drawRect(QPoint(of.x() + viewport.x() + ofsinviewport, of.y()), newsize);
        GrItem->setPos(drawRect.topLeft());
        return drawRect;
    }
};

/**
 * @brief The ImageView class
 * It provides to show 1 or 2 images once, using OpenGL.
 * It is made on QGraphicView, each images is used as QGraphicsItem
 */
class ImageView : public QGraphicsView
{
    Q_OBJECT
public:
    enum RendererType { Native, OpenGL, Image };
    explicit ImageView(QWidget *parent = Q_NULLPTR);
    void setRenderer(RendererType type = Native);
    void setPageManager(PageManager* manager);
    /**
     * @brief currentViewSize returns current manual resizing magnification value
     * @return
     */
    int currentViewSize() { return viewSizeList[viewSizeIdx]; }
    Qt::AnchorPoint hoverState() const { return m_hoverState; }
    void skipRisizeEvent(bool skipped) { m_skipResizeEvent = skipped; }
    bool isSlideShow() const { return m_slideshowTimer != nullptr; }
    void toggleSlideShow();
    void setWillFullscreen(bool fullscreen) { m_isFullScreen = fullscreen; }

signals:
    /**
     * @brief anchorHovered a signal when the mouse moved to one of 4 edges on this widget
     */
    void anchorHovered(Qt::AnchorPoint anchor) const;
//    void pageChanged() const;

    void fittingChanged(bool fitting) const;

protected:
//    void paintEvent( QPaintEvent *event );
    void mouseMoveEvent(QMouseEvent *event);
//    void mousePressEvent(QMouseEvent *event);
//    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event) override;
//    void dragEnterEvent(QDragEnterEvent *event) { event->accept(); qDebug() << "ImageView::dragEnterEvent"; }
//    void dropEvent( QDropEvent *e ) {qDebug() << "ImageView::dropEvent";}
//    void dragMoveEvent( QDragMoveEvent *e ) {qDebug() << "ImageView::dragMoveEvent";}
//    void dragLeaveEvent( QDragLeaveEvent * e ) {qDebug() << "ImageView::dragLeaveEvent";}

public slots:
    void on_volumeChanged_triggered();
    bool on_addImage_triggered(ImageContent image, bool pageNext);
    void on_clearImages_triggered();
    void readyForPaint();

    // Page
    void on_nextPage_triggered();
    void on_prevPage_triggered();
    void on_fastForwardPage_triggered();
    void on_fastBackwardPage_triggered();
    void on_firstPage_triggered();
    void on_lastPage_triggered();
    void on_nextOnlyOnePage_triggered();
    void on_prevOnlyOnePage_triggered();
    void on_rotatePage_triggered();

    // SlideShow
    void on_slideShowChanging_triggered();

    // Volume
    void on_nextVolume_triggered();
    void on_prevVolume_triggered();

    void on_fitting_triggered(bool maximized);
    void on_dualView_triggered(bool viewdual);
    void on_rightSideBook_triggered(bool rightSideBook);
    void on_wideImageAsOneView_triggered(bool wideImage);
    void on_firstImageAsOneView_triggered(bool firstImage);

    void on_scaleUp_triggered();
    void on_scaleDown_triggered();
    void on_openFiler_triggered();
    void on_openExifDialog_triggered();
    void on_copyPage_triggered();
    void on_copyFile_triggered();


private:

    RendererType m_renderer;
    QVector<PageGraphicsItem> m_pages;

    SavedPoint m_ptLeftTop;
    QGraphicsScene* m_scene;
    bool m_isMouseDown;
    Qt::AnchorPoint m_hoverState;
    /**
     * @brief for manual ZoomIn or ZoomOut
     */
    QList<int> viewSizeList;
    QVector<int> m_pageRotations;
    int viewSizeIdx;
    QFont m_font;
    bool m_wideImage;
    bool m_skipResizeEvent;

    ExifDialog exifDialog;

    PageManager* m_pageManager;
    ShaderManager m_effectManager;
    QTimer* m_slideshowTimer;

    bool m_isFullScreen;
};



#endif // IMAGEVIEW_H