#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QSettings>
#include <QFontDatabase>
#include <QMessageBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QListWidget>
#include "colormap.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lstBlocks->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    QSettings settings;
    _file = settings.value("lastfile", "").toString();
    _lastopen = settings.value("lastopen", "").toString();
    if (_lastopen.isEmpty()) _lastopen = settings.value("lastdir", "").toString();
    if (!_file.isEmpty()) {
        if (! loadRom()) {
            _file = "";
            settings.setValue("lastfile", _file);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_tiles_selectionChanged(int index)
{
    ui->lstBlocks->setCurrentRow(index);
}

void MainWindow::on_btnLoad_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, "Load ROM", _lastopen, "ROM (*.sfc);;All Files (*)");
    if (f.isEmpty()) return;
    _file = f;
    _lastopen = _file;
    if (!loadRom()) _file = "";
    QSettings settings;
    settings.setValue("lastfile", _file);
    settings.setValue("lastopen", _lastopen);
}

bool MainWindow::loadRom()
{
    //return false; // error
    if (_rom) delete _rom;
    _rom = new Rom(_file);

    ui->lstBlocks->clear();

    if (!_rom->isOpen()) {
        QMessageBox::critical(this, "Could not open ROM", "Error");
        return false;
    }
    uint16_t boyPalette[16] = { 0, 0x7fbc, 0x4edf, 0x2dba, 0x1913, 0x1489, 0x7eaa, 0x7923, 0x5465, 0x02bf, 0x059c, 0x08f6, 0x07ea, 0x07ea, 0x1885, 0x0000 };
    ui->tiles->addColorMap(ColorMap::FromSnes(boyPalette));

    for (int i=0; i<4854; i++) { // TODO: how many are there actually?
        SpriteBlock block(i, _rom);
        if (_rom->mapaddr(block.dataaddr) >= 0x300000-8 /*3MB*/) continue;
        _spriteBlocks.append(block);
        ui->lstBlocks->addItem(block.toString());
        ui->tiles->add(block);
    }
    ui->lstBlocks->adjustSize();

    return true; // OK
}

void MainWindow::on_lstBlocks_currentRowChanged(int currentRow)
{
    ui->tiles->setSelected(currentRow);
    int tileY1 = ui->tiles->getY(currentRow);
    int tileY2 = tileY1+34; // both borders visible
    int viewY1 = ui->scrollArea->verticalScrollBar()->value();
    int viewY2 = viewY1+ui->scrollArea->viewport()->height();
    if (tileY1<viewY1) {
        // scroll up until tile is visible
        ui->scrollArea->ensureVisible(0, tileY2, 0, 34);
    } else if (tileY2>viewY2) {
        // scroll down until tile is visible
        ui->scrollArea->ensureVisible(0, tileY1, 0, 34);
    }
}
