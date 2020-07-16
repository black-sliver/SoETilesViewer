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
    virtual void resizeEvent(QResizeEvent*);
    virtual void paintEvent(QPaintEvent*);

private:
    QList<SpriteBlock> _spriteBlocks;
    QVector<ColorMap> _colorMaps;
    QMap<int,int> _colorMapMap;
    bool _layoutChanged = false;
    QRgb* _pixels = NULL;
    QImage* _image = NULL;
};

#endif // TILEVIEW_H
