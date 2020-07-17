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

    void add(const SpriteBlock& block);
    int addColorMap(const ColorMap& map);
    void setSelected(int index);
    int getX(int index) const;
    int getY(int index) const;
    int getIndex(int x, int y) const;

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
    QRgb* _pixels = NULL;
    QImage* _image = NULL;
    int _selected = -1;
};

#endif // TILEVIEW_H
