#ifndef TILEVIEW_H
#define TILEVIEW_H

#include <QWidget>
#include <QList>
#include <QVector>
#include <QMap>
#include "abstracttile.h"
#include "colormap.h"

class TileView : public QWidget
{
    Q_OBJECT
public:
    TileView(QWidget *parent = 0);
    ~TileView();

    virtual void clear();
    virtual void add(const AbstractTile& block);
    virtual void set(int index, const AbstractTile& block);

    int addColorMap(const ColorMap& map);
    void setColorMap(int index, const ColorMap& map);
    ColorMap colorMap(int index) const;
    ColorMap itemColorMap(int index) const;

    void setSelected(int index);
    int selected() const;
    virtual int itemX(int index) const;
    virtual int itemY(int index) const;
    virtual int itemIndex(int x, int y) const;

    void setBackground(QRgb color);

    static constexpr int TILE_SIZE = 32;
    static constexpr int TILE_SPACE = 1;
    static constexpr int TILE_PITCH = TILE_SIZE+TILE_SPACE;
    static constexpr int TILE_OUTER_SIZE = TILE_SIZE+2*TILE_SPACE;

signals:
    void selectionChanged(int index);

protected:
    virtual void resizeEvent(QResizeEvent*);
    virtual void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent*);

    QList<AbstractTile> _tiles;
    QVector<ColorMap> _colorMaps;
    QMap<int,int> _colorMapMap;
    bool _layoutChanged = false;
    bool _mapsChanged = false;
    QRgb* _pixels = NULL;
    QImage* _image = NULL;
    int _selected = -1;
    QRgb _bgColor = 0;

    static constexpr int MIN_H = TILE_OUTER_SIZE;
    static int _cols(int w);
    static int _col(int x);
    static int _row(int y);
};

#endif // TILEVIEW_H
