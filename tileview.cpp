#include "tileview.h"
#include <QWidget>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QResizeEvent>
#include <QPaintEvent>

TileView::TileView(QWidget *parent)
    : QWidget(parent)
{
    this->setMinimumWidth(33);
}

TileView::~TileView()
{
    if (_pixels) delete[] _pixels;
    _pixels = NULL;
    if (_image) delete _image;
    _image = NULL;
}

void TileView::add(const SpriteBlock& block)
{
    // TODO: extract data from block instead of storing the block
    _spriteBlocks.append(block);
    _layoutChanged = true;
}

int TileView::addColorMap(const ColorMap &map)
{
    int idx = _colorMaps.length();
    _colorMaps.append(map);
    return idx;
}

void TileView::setSelected(int index)
{
    _selected = index;
}

int TileView::getY(int index) const
{
    int w = width();
    int cols = (w-1)/(33);
    int row = index/cols;
    return row*33;
}


void TileView::resizeEvent(QResizeEvent *ev)
{
    //if (ev->size().width() != width())
    _layoutChanged = true;
}
void TileView::paintEvent(QPaintEvent* ev)
{
    //QScrollArea* scroll = dynamic_cast<QScrollArea*>(this->parentWidget()); // this returns NULL
    int off = 0;
    //if (scroll && scroll->verticalScrollBar()) off = scroll->verticalScrollBar()->value();
    int w = width();
    int h = height();
    int cols = (w-1)/(33);

    if (!_pixels || !_image || _layoutChanged) {
        qDebug("reshape: %dx%d, scroll %d\n", w, h, off);

        int rows =(_spriteBlocks.count()+cols-1)/cols;
        this->setMinimumHeight(1+33*rows);
        _layoutChanged = false;

        //QWidget::paintEvent(ev);
        int x = 1;
        int y = 1;
        if (_pixels) delete[] _pixels;
        _pixels = new QRgb[w*h];

        QRgb* pixels = _pixels;
        memset(pixels, 0, w*h*sizeof(QRgb));
        const ColorMap& map = _colorMaps[_colorMapMap[0]];
        for (int i=0; i<_spriteBlocks.size(); i++) {
            const SpriteBlock& block = _spriteBlocks[i];
            QByteArray src = block.getPixels();
            for (int ys=0; ys<16; ys++) {
                for (int xs=0; xs<16; xs++) {
                    int xd = x + 2*(xs);
                    int yd = y + 2*(ys);
                    uint8_t c = src[xs+ys*16];
                    pixels[(xd+0) + (yd+0)*w] = map.c[c];
                    pixels[(xd+0) + (yd+1)*w] = map.c[c];
                    pixels[(xd+1) + (yd+0)*w] = map.c[c];
                    pixels[(xd+1) + (yd+1)*w] = map.c[c];
                }
            }
            x += 33;
            if (x+32 > w) {
                x = 1;
                y += 33;
            }
            if (y+32 > h) break; // out of space
        }
        if (_image) delete _image;
        _image = new QImage((uchar*)_pixels, w, h, QImage::Format_ARGB32);
    }
    QPainter painter(this);
    qDebug("redraw: %dx%d, scroll %d\n", w, h, off);
    if (_selected>=0) {
        int row = _selected/cols;
        int col = _selected%cols;
        int x1=col*33, x2=x1+33, y1=row*33, y2=y1+33;
        painter.setPen(QColor(0xff66aaff));
        painter.fillRect(x1, y1, 34, 34, QColor(0xff66aaff));
        painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
        painter.drawLine(x1, y1, x2, y1);
        painter.drawLine(x1, y2, x2, y2);
        painter.drawLine(x1, y1, x1, y2);
        painter.drawLine(x2, y1, x2, y2);
    }
    painter.drawImage(0,0, *_image);
}
