#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "rom.h"
#include "spriteblock.h"

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

private:
    Ui::MainWindow *ui;
    QString _file;
    QString _lastopen;
    Rom* _rom = NULL;
    QList<SpriteBlock> _spriteBlocks;
    bool loadRom();
};
#endif // MAINWINDOW_H