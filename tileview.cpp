#include "tileview.h"
#include <QWidget>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QResizeEvent>
#include <QPaintEvent>


int TileView::_cols(int w) {
    return (w-TILE_SPACE)/(TILE_PITCH);
}
int TileView::_col(int x) {
    // this includes left border/space and excludes right
    return x/TILE_PITCH;
}
int TileView::_row(int y) {
    // this includes top border/space and excludes bottom
    return y/TILE_PITCH;
}

TileView::TileView(QWidget *parent)
    : QWidget(parent)
{
    this->setMinimumWidth(MIN_H);
}

TileView::~TileView()
{
    clear();
}

void TileView::clear()
{
    _spriteBlocks.clear();
    _colorMaps.clear();
    _colorMapMap.clear();

    if (_pixels) delete[] _pixels;
    _pixels = NULL;
    if (_image) delete _image;
    _image = NULL;

    this->repaint();
    this->setMinimumWidth(MIN_H);
}

void TileView::add(const SpriteBlock& block)
{
    // TODO: extract data from block instead of storing the block
    _spriteBlocks.append(block);
    _layoutChanged = true;
}
void TileView::set(int index, const SpriteBlock& block)
{
    // TODO: extract data from block instead of storing the block
    _spriteBlocks[index] = block;
    _layoutChanged = true;
}

int TileView::addColorMap(const ColorMap &map)
{
    int index = _colorMaps.length();
    _colorMaps.append(map);
    if (index==0) _mapsChanged = true;
    return index;
}

ColorMap TileView::colorMap(int index) const
{
    if (index>_colorMaps.length()) return ColorMap();
    return _colorMaps[index];
}

ColorMap TileView::itemColorMap(int index) const
{
    if (index>_spriteBlocks.length()) return ColorMap();
    auto it = _colorMapMap.find(index);
    if (it == _colorMapMap.end())
        return colorMap(0);
    return colorMap(it.value());
}

void TileView::setColorMap(int index, const ColorMap &map)
{
    bool mapInUse = index<_colorMaps.length(); // TODO: actually find out if it's in use
    bool needRefresh = mapInUse && (map != _colorMaps[index]);
    if (index > _colorMaps.length()) return;
    else if (index == _colorMaps.length()) addColorMap(map);
    else _colorMaps[index] = map;
    if (needRefresh) {
        _mapsChanged = true;
        repaint();
    }
}

int TileView::selected() const
{
    return _selected;
}
void TileView::setSelected(int index)
{
    if (index == _selected) return;

    int newx = itemX(index);
    int newy = itemY(index);
    int oldx = itemX(_selected);
    int oldy = itemY(_selected);
    if (_selected>=0 && index>=0) {
        int minx = ((oldx<newx) ? oldx : newx);
        int miny = ((oldy<newy) ? oldy : newy);
        int maxx = ((oldx>newx) ? oldx : newx)+TILE_OUTER_SIZE;
        int maxy = ((oldy>newy) ? oldy : newy)+TILE_OUTER_SIZE;
        _selected = index;
        repaint(minx, miny, maxx-minx, maxy-miny);
    } else if (index >= 0) {
        _selected = index;
        repaint(newx, newy, TILE_OUTER_SIZE, TILE_OUTER_SIZE);
    } else if (_selected >= 0) {
        _selected = index;
        repaint(oldx, oldy, TILE_OUTER_SIZE, TILE_OUTER_SIZE);
    }
}

int TileView::itemX(int index) const
{
    int cols = _cols(width());
    int col = index%cols;
    return col*TILE_PITCH;
}
int TileView::itemY(int index) const
{
    int cols = _cols(width());
    int row = index/cols;
    return row*TILE_PITCH;
}
int TileView::itemIndex(int x, int y) const
{
    int cols = _cols(width());
    int row = _row(y);
    int col = _col(x);
    if (col>=cols) return -1;
    int idx = row*cols + col;
    if (idx>=_spriteBlocks.size()) return -1;
    return idx;
}
void TileView::setBackground(QRgb color)
{
    if (_bgColor==color || (_bgColor>>24==0 && color>>24==0)) return;
    _bgColor = color;
    repaint();
}

void TileView::mousePressEvent(QMouseEvent * ev)
{
    int index = itemIndex(ev->x(), ev->y());
    setSelected(index);
    emit(selectionChanged(index));
}
void TileView::resizeEvent(QResizeEvent *ev)
{
    int oldcols = _cols(ev->oldSize().width());
    int newcols = _cols(ev->size().width());
    if ( oldcols != newcols ) {
        int newrows =(_spriteBlocks.count()+newcols-1)/newcols;
        int newh = TILE_SPACE+TILE_PITCH*newrows;
        this->setMinimumHeight(newh);
        _layoutChanged = true;
    }
}
void TileView::paintEvent(QPaintEvent* ev)
{
    int w = width();
    int h = height();
    int cols = _cols(w);

    if (_colorMaps.empty() || _spriteBlocks.empty()) {
        QPainter painter(this);
        painter.eraseRect(ev->rect());
        return;
    }
    if (!_pixels || !_image || _layoutChanged || _mapsChanged) {
        qDebug("reshape: %dx%d\n", w, h);

        int rows =(_spriteBlocks.count()+cols-1)/cols;
        int newh = TILE_SPACE+TILE_PITCH*rows;
        if (newh < MIN_H) newh=MIN_H;
        if (newh > this->minimumHeight()) {
            this->setMinimumHeight(newh);
            repaint();
            return;
        } else {
            this->setMinimumHeight(newh);
        }
        _layoutChanged = false;
        _mapsChanged = false;

        int x = TILE_SPACE;
        int y = TILE_SPACE;
        if (_pixels) delete[] _pixels;
        _pixels = new QRgb[w*h];

        QRgb* pixels = _pixels;
        memset(pixels, 0, w*h*sizeof(QRgb));
        const ColorMap& map = _colorMaps[_colorMapMap[0]];
        for (int i=0; i<_spriteBlocks.size(); i++) {
            const SpriteBlock& block = _spriteBlocks[i];
            QByteArray src = block.getPixels();
            for (int ys=0; ys<block.size; ys++) {
                for (int xs=0; xs<block.size; xs++) {
                    int xd = x + 2*(xs);
                    int yd = y + 2*(ys);
                    uint8_t c = src[xs+ys*block.size];
                    pixels[(xd+0) + (yd+0)*w] = map.c[c];
                    pixels[(xd+0) + (yd+1)*w] = map.c[c];
                    pixels[(xd+1) + (yd+0)*w] = map.c[c];
                    pixels[(xd+1) + (yd+1)*w] = map.c[c];
                }
            }
            x += TILE_PITCH;
            if (x+TILE_PITCH > w) {
                x = TILE_SPACE;
                y += TILE_PITCH;
            }
            if (y+TILE_SIZE > h) break; // out of space
        }
        if (_image) delete _image;
        _image = new QImage((uchar*)_pixels, w, h, QImage::Format_ARGB32);
    }
    QPainter painter(this);
    if (_bgColor>>24) painter.fillRect(0,0,width(),height(),_bgColor);
    if (_selected>=0) {
        int row = _selected/cols;
        int col = _selected%cols;
        int x1=col*TILE_PITCH, x2=x1+TILE_PITCH, y1=row*TILE_PITCH, y2=y1+TILE_PITCH;
        painter.setPen(QColor(0xff66aaff));
        painter.fillRect(x1, y1, TILE_OUTER_SIZE, TILE_OUTER_SIZE,
                         QColor(0xff66aaff));
        painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
        painter.drawLine(x1, y1, x2, y1);
        painter.drawLine(x1, y2, x2, y2);
        painter.drawLine(x1, y1, x1, y2);
        painter.drawLine(x2, y1, x2, y2);
    }
    painter.drawImage(0,0, *_image);
}
