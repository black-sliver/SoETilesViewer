#include "frameview.h"
#include <QPainter>
#include <QPaintEvent>

FrameView::FrameView(QWidget *parent)
    : TileView(parent)
{
}
FrameView::~FrameView()
{
}

void FrameView::clear()
{
    _selected = -1;
    _spriteChunks.clear();
    TileView::clear();
}
void FrameView::add(const AbstractTile& block)
{
    add({},block);
}
void FrameView::add(const SpriteChunk& chunk, const AbstractTile& block)
{
    _spriteChunks.append(chunk);
    _tiles.append(block);
    _layoutChanged = true;
}
void FrameView::set(int index, const AbstractTile& block)
{
    _tiles[index] = block;
    _layoutChanged = true;
}
void FrameView::set(int index, const SpriteChunk& chunk, const AbstractTile& block)
{
    _spriteChunks[index] = chunk;
    _tiles[index] = block;
    _layoutChanged = true;
}


int FrameView::itemX(int index) const
{
    if (index<0) return 0;
    int minx = 0, maxx = 0;
    for (const auto& chunk: _spriteChunks) {
        if (chunk.x < minx) minx = chunk.x;
        if (chunk.x+15 > maxx) maxx = chunk.x+15;
    }
    return ((int)_spriteChunks[index].x-minx)*2;
}
int FrameView::itemY(int index) const
{
    if (index<0) return 0;
    int miny = 0, maxy = 0;
    for (const auto& chunk: _spriteChunks) {
        if (chunk.y < miny) miny = chunk.y;
        if (chunk.y+15 > maxy) maxy = chunk.y+15;
    }
    return ((int)_spriteChunks[index].y-miny)*2;
}
int FrameView::itemIndex(int x, int y) const
{
    int minx = 0, maxx = 0, miny = 0, maxy = 0;
    for (const auto& chunk: _spriteChunks) {
        if (chunk.x < minx) minx = chunk.x;
        if (chunk.x+15 > maxx) maxx = chunk.x+15;
        if (chunk.y < miny) miny = chunk.y;
        if (chunk.y+15 > maxy) maxy = chunk.y+15;
    }
    for (int8_t priotiy=3; priotiy>=0; priotiy--) {
        for (int i=0; i<_spriteChunks.size(); i++) {
            const auto& chunk = _spriteChunks[i];
            if (((chunk.flags&0x30)>>4) != (uint8_t)priotiy) continue;
            int chunkx = ((int)chunk.x-minx)*2;
            int chunky = ((int)chunk.y-miny)*2;
            int chunksize = chunk.flags&1? TILE_SIZE : TILE_SIZE/2;
            if (x>=chunkx && x<chunkx+chunksize && y>=chunky && y<chunky+chunksize) return i;
        }
    }
    return -1;
}


void FrameView::paintEvent(QPaintEvent* ev)
{
    if (_colorMaps.empty() || _tiles.empty()) {
        QPainter painter(this);
        painter.eraseRect(ev->rect());
        return;
    }

    int minx = 0, maxx = 0, miny = 0, maxy = 0;
    for (const auto& chunk: _spriteChunks) {
        if (chunk.x < minx) minx = chunk.x;
        if (chunk.x+15 > maxx) maxx = chunk.x+15;
        if (chunk.y < miny) miny = chunk.y;
        if (chunk.y+15 > maxy) maxy = chunk.y+15;
    }

    if (!_pixels || !_image || _layoutChanged || _mapsChanged) {
        int w = (maxx-minx+1)*2;
        int h = (maxy-miny+1)*2;

        qDebug("reshape: %dx%d\n", w, h);

        if (h > this->minimumHeight()) {
            this->setMinimumHeight(h);
            repaint();
            return;
        } else {
            this->setMinimumHeight(h);
        }
        this->setMinimumWidth(w);

        _layoutChanged = false;
        _mapsChanged = false;

        if (_pixels) delete[] _pixels;
        _pixels = new QRgb[w*h];

        QRgb* pixels = _pixels;
        memset(pixels, 0, w*h*sizeof(QRgb));
        const ColorMap& map = _colorMaps[_colorMapMap[0]];
        for (uint8_t priority = 0; priority<4; priority++) {
            for (int i=_tiles.size()-1; i>=0; i--) {
                const SpriteChunk& chunk = _spriteChunks[i];
                if (((chunk.flags&0x30)>>4) != priority) continue;
                int x = 2*((int)chunk.x - minx);
                int y = 2*((int)chunk.y - miny);

                const AbstractTile& block = _tiles[i];
                QByteArray src = block.getPixels();
                //if (chunk.flags&1) {
                    for (int ys=0; ys<block.size; ys++) {
                        for (int xs=0; xs<block.size; xs++) {
                            int xd = x + 2*(xs);
                            int yd = y + 2*(ys);
                            if (chunk.flags&0x40) // invert X coords
                                xd = x + 2*(block.size-1-xs);
                            if (chunk.flags&0x80) // invert Y coords
                                yd = y + 2*(block.size-1-ys);
                            uint8_t c = src[xs+ys*block.size];
                            if (c==0) continue; // transparent
                            pixels[(xd+0) + (yd+0)*w] = map.c[c];
                            pixels[(xd+0) + (yd+1)*w] = map.c[c];
                            pixels[(xd+1) + (yd+0)*w] = map.c[c];
                            pixels[(xd+1) + (yd+1)*w] = map.c[c];
                        }
                    }
                //} else { /* 8x8 chunk */ }
            }
        }
        if (_image) delete _image;
        _image = new QImage((uchar*)_pixels, w, h, QImage::Format_ARGB32);
    }
    QPainter painter(this);

    if (_bgColor>>24) painter.fillRect(0,0,width(),height(),_bgColor);
    if (_selected>=0) {
        const auto& chunk = _spriteChunks[_selected];
        int chunksize = chunk.flags&1? TILE_SIZE : TILE_SIZE/2;
        int x1=((int)chunk.x-minx)*2, x2=x1+chunksize-1,
            y1=((int)chunk.y-miny)*2, y2=y1+chunksize-1;
        painter.setPen(QColor(0xff66aaff));
        painter.fillRect(x1, y1, chunksize, chunksize,
                         QColor(0xff66aaff));
        painter.drawImage(0,0, *_image);
        painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
        painter.drawLine(x1, y1, x2, y1);
        painter.drawLine(x1, y2, x2, y2);
        painter.drawLine(x1, y1, x1, y2);
        painter.drawLine(x2, y1, x2, y2);
        painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
        painter.drawLine(x1, y1, x2, y1);
        painter.drawLine(x1, y2, x2, y2);
        painter.drawLine(x1, y1, x1, y2);
        painter.drawLine(x2, y1, x2, y2);
    } else {
        painter.drawImage(0,0, *_image);
    }
}
