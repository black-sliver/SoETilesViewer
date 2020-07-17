#include "tileview.h"
#include <QWidget>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QResizeEvent>
#include <QPaintEvent>


#define TILE_SIZE 32
#define TILE_SPACE 1
static int _cols(int w) {
    return (w-TILE_SPACE)/(TILE_SIZE+TILE_SPACE);
}
static int _col(int x) {
    // this includes left border/space and excludes right
    return x/(TILE_SIZE+TILE_SPACE);
}
static int _row(int y) {
    // this includes top border/space and excludes bottom
    return y/(TILE_SIZE+TILE_SPACE);
}

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
    if (index == _selected) return;

    int newx = getX(index);
    int newy = getY(index);
    int oldx = getX(_selected);
    int oldy = getY(_selected);
    if (_selected>=0 && index>=0) {
        int minx = (oldx<newx) ? oldx-1 : newx-1;
        int miny = (oldy<newy) ? oldy-1 : newy-1;
        int maxx = (oldx>newx) ? oldx+34 : newx+34;
        int maxy = (oldy>newy) ? oldy+34 : newy+34;
        _selected = index;
        repaint(minx, miny, maxx-minx, maxy-miny);
    } else if (index >= 0) {
        _selected = index;
        repaint(newx-1, newy-1, 35, 35);
    } else if (_selected >= 0) {
        _selected = index;
        repaint(oldx-1, oldy-1, 35, 35);
    }
}

int TileView::getX(int index) const
{
    int cols = _cols(width());
    int col = index%cols;
    return col*33;
}
int TileView::getY(int index) const
{
    int cols = _cols(width());
    int row = index/cols;
    return row*33;
}
int TileView::getIndex(int x, int y) const
{
    int cols = _cols(width());
    int row = _row(y);
    int col = _col(x);
    if (col>=cols) return -1;
    int idx = row*cols + col;
    if (idx>=_spriteBlocks.size()) return -1;
    return idx;
}
void TileView::mousePressEvent(QMouseEvent * ev)
{
    int index = getIndex(ev->x(), ev->y());
    setSelected(index);
    emit(selectionChanged(index));
}
void TileView::resizeEvent(QResizeEvent *ev)
{
    int oldcols = _cols(ev->oldSize().width());
    int newcols = _cols(ev->size().width());
    if ( oldcols != newcols ) {
        int newrows =(_spriteBlocks.count()+newcols-1)/newcols;
        int newh = 1+33*newrows;
        this->setMinimumHeight(newh);
        _layoutChanged = true;
    }
}
void TileView::paintEvent(QPaintEvent* ev)
{
    int w = width();
    int h = height();
    int cols = _cols(w);

    if (!_pixels || !_image || _layoutChanged) {
        qDebug("reshape: %dx%d\n", w, h);

        int rows =(_spriteBlocks.count()+cols-1)/cols;
        int newh = 1+33*rows;
        if (newh > this->minimumHeight()) {
            this->setMinimumHeight(newh);
            repaint();
            return;
        } else {
            this->setMinimumHeight(newh);
        }
        _layoutChanged = false;

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
            if (x+33 > w) {
                x = 1;
                y += 33;
            }
            if (y+32 > h) break; // out of space
        }
        if (_image) delete _image;
        _image = new QImage((uchar*)_pixels, w, h, QImage::Format_ARGB32);
    }
    QPainter painter(this);
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
