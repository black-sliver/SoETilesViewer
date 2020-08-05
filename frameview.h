#ifndef FRAMEVIEW_H
#define FRAMEVIEW_H

#include <QObject>
#include <QWidget>
#include "tileview.h"
#include "spriteinfo.h"

class FrameView : public TileView
{
    Q_OBJECT
public:
    FrameView(QWidget *parent = 0);
    ~FrameView();

    virtual void clear();
    virtual void add(const SpriteBlock& block);
    virtual void add(const SpriteChunk& chunk, const SpriteBlock& block);
    virtual void set(int index, const SpriteBlock& block);
    virtual void set(int index, const SpriteChunk& chunk, const SpriteBlock& block);

    virtual int itemX(int index) const;
    virtual int itemY(int index) const;
    virtual int itemIndex(int x, int y) const;

protected:
    virtual void paintEvent(QPaintEvent*);
    QList<SpriteChunk> _spriteChunks;
};

#endif // FRAMEVIEW_H
