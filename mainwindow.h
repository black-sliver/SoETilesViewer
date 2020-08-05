#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "rom.h"
#include "spriteblock.h"
#include "spriteinfo.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnLoad_clicked();

    void on_lstBlocks_currentRowChanged(int currentRow);
    void on_tiles_selectionChanged(int index);
    void on_tiles_customContextMenuRequested(const QPoint &pos);

    void on_lstSprites_currentRowChanged(int currentRow);
    void on_lstSpriteChunks_currentRowChanged(int currentRow);
    void on_sprite_selectionChanged(int index);

    void on_cbxDefaultColorMap_activated(int index);
    void on_cbxDefaultColorMap2_activated(int index);


    void on_cbxBackground_activated(int index);

    void on_cbxBackground2_activated(int index);

private:
    Ui::MainWindow *ui;
    QString _file;
    QString _lastopen;
    QString _exportdir;
    QStringList _workingCopies;
    QStringList _warnedCopies;
    bool _willBecomeWorkingCopy = false;
    Rom* _rom = NULL;
    QList<SpriteBlock> _spriteBlocks;
    int _largeBlocksCount=0;
    QList<SpriteInfo> _spriteInfos;
    QString _baseTitle;
    bool loadRom();
};
#endif // MAINWINDOW_H
