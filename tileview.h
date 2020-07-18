#ifndef TILEVIEW_H
#define TILEVIEW_H

#include <QWidget>
#include <QList>
#include <QVector>
#include <QMap>
#include "spriteblock.h"
#include "colormap.h"

class TileView : public QWidget
{
    Q_OBJECT
public:
    TileView(QWidget *parent = 0);
    ~TileView();

    void clear();

    void add(const SpriteBlock& block);
    void set(int index, const SpriteBlock& block);

    int addColorMap(const ColorMap& map);
    void setColorMap(int index, const ColorMap& map);
    ColorMap colorMap(int index) const;
    ColorMap itemColorMap(int index) const;

    void setSelected(int index);
    int selected() const;
    int itemX(int index) const;
    int itemY(int index) const;
    int itemIndex(int x, int y) const;

    static constexpr int TILE_SIZE = 32;
    static constexpr int TILE_SPACE = 1;
    static constexpr int TILE_PITCH = TILE_SIZE+TILE_SPACE;
    static constexpr int TILE_OUTER_SIZE = TILE_SIZE+2*TILE_SPACE;

signals:
    void selectionChanged(int index);

private:
    virtual void resizeEvent(QResizeEvent*);
    virtual void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent*);

    QList<SpriteBlock> _spriteBlocks;
    QVector<ColorMap> _colorMaps;
    QMap<int,int> _colorMapMap;
    bool _layoutChanged = false;
    bool _mapsChanged = false;
    QRgb* _pixels = NULL;
    QImage* _image = NULL;
    int _selected = -1;

    static constexpr int MIN_H = TILE_OUTER_SIZE;
    static int _cols(int w);
    static int _col(int x);
    static int _row(int y);
};

#endif // TILEVIEW_H
