#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QSettings>
#include <QFontDatabase>
#include <QMessageBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QListWidget>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QFileInfo>
#include <QKeyEvent>
#include "colormap.h"


struct PredefinedColorMap {
    const char* name;
    uint16_t snescolors[16];
};
struct PredefinedBackground {
    const char* name;
    QRgb color;
};


constexpr auto SFC_FILTER = "SNES ROM (*.sfc);;All Files (*)";
constexpr auto PNG_FILTER = "PNG Image (*.png);;All Files (*)";

constexpr PredefinedColorMap colorMaps[] = {
    { "Boy",        { 0, 0x7fbc, 0x4edf, 0x2dba, 0x1913, 0x1489, 0x7eaa, 0x7923, 0x5465, 0x02bf, 0x059c, 0x08f6, 0x07ea, 0x0287, 0x1885, 0x0000 } },
    { "Act0 Dog",   { 0, 0x7fde, 0x52d8, 0x4253, 0x35f0, 0x298d, 0x62f7, 0x4e73, 0x4210, 0x35ac, 0x4a10, 0x39ac, 0x2d29, 0x1cc6, 0x14a5, 0x0000 } },
    { "Act1 Dog",   { 0, 0x7fbc, 0x7eaa, 0x7923, 0x4eb6, 0x1d4d, 0x3e11, 0x212a, 0x0c86, 0x0c63, 0x4534, 0x2cf4, 0x24cd, 0x10aa, 0x1063, 0x0000 } },
    { "Act2 Dog",   { 0, 0x7ffe, 0x7b99, 0x6715, 0x5690, 0x39aa, 0x2927, 0x18a3, 0x5259, 0x41b6, 0x27ed, 0x27ed, 0x27ed, 0x27ed, 0x1884, 0x0000 } },
    { "Act3 Dog",   { 0, 0x6eff, 0x5a5b, 0x49d8, 0x3d54, 0x2cf1, 0x208d, 0x184a, 0x27ed, 0x27ed, 0x27ed, 0x27ed, 0x27ed, 0x27ed, 0x1063, 0x0000 } },
    { "Act4 Dog",   { 0, 0x7f9c, 0x7b13, 0x0013, 0x001f, 0x0013, 0x0007, 0x2def, 0x298c, 0x6e70, 0x59ec, 0x4149, 0x3506, 0x28a3, 0x1441, 0x0000 } },
    { "Cat",        { 0, 0x4f9f, 0x4eff, 0x3639, 0x1238, 0x09f6, 0x0193, 0x0151, 0x012d, 0x0154, 0x0176, 0x0178, 0x10df, 0x166e, 0x10a3, 0x0000 } },
    { "Intro NPC",  { 0, 0x7ffc, 0x4ebd, 0x2198, 0x0932, 0x00aa, 0x4665, 0x2523, 0x00d0, 0x00cd, 0x6815, 0x6850, 0x542e, 0x400b, 0x18c7, 0x0000 } },
    { "1965 NPCs",  { 0, 0x5bdd, 0x579b, 0x4f59, 0x4b17, 0x42d5, 0x3e93, 0x3651, 0x320e, 0x29cc, 0x258a, 0x1d48, 0x1906, 0x10c4, 0x0c82, 0x0000 } },
    { "Name Cursor",{ 0, 0x2fff, 0x16d9, 0x056f, 0x0067, 0x63ff, 0x3f39, 0x1e73, 0x0dad, 0x00c7, 0x16df, 0x0a38, 0x0937, 0x502e, 0x4841, 0x0000 } },
    { "Ring Menu",  { 0, 0x7fde, 0x66f6, 0x3d8b, 0x02bf, 0x015f, 0x0010, 0x25fa, 0x1111, 0x7c1f, 0x3c0f, 0x1f47, 0x01a0, 0x7f2c, 0x7c80, 0x0420 } },
    { "Ring Cursor",{ 0, 0x7fbc, 0x4edf, 0x2dba, 0x1913, 0x1489, 0x728b, 0x6586, 0x44c3, 0x025e, 0x0537, 0x0491, 0x52d8, 0x2df0, 0x1c85, 0x0000 } },
    { "Copyright",  { 0, 0x39ce, 0x77bd, 0x3dfa, 0x0014, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 } },
};

constexpr PredefinedBackground backgrounds[] = {
    { "None",    0x00000000 },
    { "White",   0xffffffff },
    { "Black",   0xff000000 },
    { "Magenta", 0xffff00ff },
    { "Cyan",    0xff00ffff },
};


static bool appendSuffixAskToOverwrite(QWidget* parent, QString& f, const QString& suffix,
                                       const QString& title="Already exists")
{
    if (!QFileInfo(f).suffix().isEmpty()) return true; // user gave suffix

    f.append(suffix);
    QFileInfo fi(f);
    if (fi.exists()) // file with suffix already exists. ask to overwrite
        return QMessageBox::question(parent, title,
                                     QStringLiteral("%1 already exists.\nDo you want to replace it?")
                                     .arg(fi.fileName())
                                     ) == QMessageBox::Yes;
    return true; // file with suffix does not yet exist
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->lstBlocks->setFont(fixedFont);
    ui->lstSprites->setFont(fixedFont);
    ui->lstSpriteChunks->setFont(fixedFont);

    // add shortcut text and filter for tab switching
    for (int i=0; i<ui->tabWidget->count(); i++) {
        const QString s = (i==0) ? " [Ctrl+1]" : (" ["+QString::number(i+1)+"]");
        ui->tabWidget->setTabText(i, ui->tabWidget->tabText(i) + s);
    }
    installEventFilter(this);

    for (auto& map: colorMaps) {
        ui->cbxDefaultColorMap->addItem(map.name);
        ui->cbxDefaultColorMap2->addItem(map.name);
    }
    for (auto& bg: backgrounds) {
        ui->cbxBackground->addItem(bg.name);
        ui->cbxBackground2->addItem(bg.name);
    }

    _baseTitle = windowTitle();

    QSettings settings;
    _file = settings.value("lastfile", "").toString();
    _lastopen = settings.value("lastopen", "").toString();
    if (_lastopen.isEmpty()) _lastopen = settings.value("lastdir", "").toString();
    _exportdir = settings.value("exportdir", "").toString();
    _workingCopies = settings.value("workingcopies").toStringList();
    _warnedCopies  = settings.value("warnedcopies").toStringList();
    if (!_file.isEmpty()) {
        if (! loadRom()) {
            _file = "";
            settings.setValue("lastfile", _file);
        }
    }
}

MainWindow::~MainWindow()
{
    QSettings settings;
    if (settings.value("exportdir") != _exportdir)
        settings.setValue("exportdir", _exportdir);
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type()==QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if (!(key->modifiers() & Qt::ControlModifier))
            return QObject::eventFilter(obj, event);
        switch (key->key())
        {
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            ui->tabWidget->setCurrentIndex(key->key()-Qt::Key_1);
                break;
        case Qt::Key_0:
            ui->tabWidget->setCurrentIndex(9);
                break;
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            if ((key->key()==Qt::Key_Backtab || (key->modifiers() & Qt::ShiftModifier)) && ui->tabWidget->currentIndex()==0)
                ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
            else if (key->key()==Qt::Key_Backtab || (key->modifiers() & Qt::ShiftModifier))
                ui->tabWidget->setCurrentIndex((ui->tabWidget->currentIndex()-1)%ui->tabWidget->count());
            else
                ui->tabWidget->setCurrentIndex((ui->tabWidget->currentIndex()+1)%ui->tabWidget->count());
            break;
        default:
            return QObject::eventFilter(obj, event);
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}


bool MainWindow::loadRom()
{
    setWindowTitle(_baseTitle);
    _willBecomeWorkingCopy = false; // opening a new file will ask the question again

    if (_rom) delete _rom;
    _rom = new Rom(_file);

    ui->lstSprites->clear();
    ui->lstBlocks->clear();
    ui->tiles->clear();
    ui->lblStats->setText("");

    if (!_rom->isOpen()) {
        QMessageBox::warning(this, "Error", "Could not open ROM");
        return false;
    }

    if (!_workingCopies.contains(_file) && !_warnedCopies.contains(_file)) {
        auto res = QMessageBox::question(this, "Create working copy?", "Changing tiles will be saved to the ROM file immediately.\nCreate a working copy?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Yes);
        if (res == QMessageBox::Yes) {
            QFileInfo srcFi(_file);
            QString f = QFileInfo(srcFi.dir(), srcFi.baseName()).filePath() + "-modified.sfc";
            f = QFileDialog::getSaveFileName(this, "Save copy", f, SFC_FILTER);
            if (f.isEmpty()) return false;
            if (!appendSuffixAskToOverwrite(this, f, ".sfc", "Savec copy")) return false;
            if (f == _file) { QMessageBox::warning(this, "Error", "Filename has to be different..."); return false; }
            if (!_rom->saveAs(f)) {
                QMessageBox::warning(this, "Error", "Could not save copy!");
                return false;
            }
            _file = f;
            _lastopen = f;
            if (!_workingCopies.contains(_file))
                _workingCopies.append(_file);
            QSettings settings;
            settings.setValue("workingcopies", _workingCopies);
        } else if (res == QMessageBox::No) {
            _warnedCopies.append(_file);
            QSettings settings;
            settings.setValue("warnedcopies", _warnedCopies);
        } else {
            return false;
        }
    }

    ui->tiles->addColorMap(ColorMap::FromSnes(colorMaps[ui->cbxDefaultColorMap->currentIndex()].snescolors));
    // 16x16 blocks
    for (int i=0; i<4854; i++) { // TODO: how many are there actually?
        SpriteBlock block(i, _rom);
        if (_rom->mapaddr(block.dataaddr) >= 0x300000-8 /*3MB*/) continue;
        _spriteBlocks.append(block);
        ui->lstBlocks->addItem(block.toString());
        ui->tiles->add(block);
    }
    _largeBlocksCount = _spriteBlocks.size();
    // 8x8 blocks
    for (int i=0; i<6265; i++) { // TODO: how many are there actually?
        SpriteBlock block(i, _rom, 8);
        if (_rom->mapaddr(block.dataaddr) >= 0x300000-2 /*3MB*/) continue;
        _spriteBlocks.append(block);
        ui->lstBlocks->addItem(block.toString());
        ui->tiles->add(block);
    }
    //ui->lstBlocks->adjustSize();
    ui->tiles->repaint();

    {
        _spriteInfos.clear();
        ui->lstSprites->clear();
        size_t start = 0xca0003; // for some reason this is not ca0001. maybe some info/count in first two bytes?
        size_t addr = start;
        bool invalidBlock = false;
        unsigned tooLongChunk = 0;

        int i=0;
        for (i=0; i<20000; i++) {
            if (_rom->mapaddr(addr) >= 0x300000-7 /*3MB*/) break; // end of rom
            if (addr==0xcb17fb) qDebug("%d boy sprites", (int)_spriteInfos.size()); // end of boy sprites

            SpriteInfo sprite(addr, _rom);
            // we have a chunk with count=0 when we need to skip to the next block unless there is no room for such a block
            if (sprite.chunks.size() == 0 || (addr&0xffff)>0x7ffd) addr = (addr&0xff0000) + 0x10001;
            if (sprite.addr != addr) sprite = {addr,_rom}; // load from new address

            if (sprite.chunks.size()>64) {
                tooLongChunk = sprite.chunks.size();
                break; // invalid -> done.
            }

            invalidBlock = false;
            for (const auto& chunk: sprite.chunks) {
                int blockIndex = chunk.flags&1 ? chunk.block : _largeBlocksCount + (int)chunk.block;
                if (blockIndex > _spriteBlocks.count()) {
                    invalidBlock = true;
                    break;
                }
            }
            if (invalidBlock) break;

            _spriteInfos.append(sprite);
            ui->lstSprites->addItem(sprite.toString());
            addr += sprite.size();
        }
        qDebug("%d items, first addr=0x%06x, last addr=0x%06x\n", i, (unsigned)start, (unsigned)addr);
        if (invalidBlock) qDebug("encountered invalid block\n");
        if (tooLongChunk) qDebug("chunk too long: %u\n", tooLongChunk);
    }

    setWindowTitle(_baseTitle + " - " + QFileInfo(_file).fileName());
    ui->lblStats->setText(QStringLiteral("%1 sprites, %2+%3 sprite blocks")
                          .arg(_spriteInfos.size())
                          .arg(_largeBlocksCount)
                          .arg(_spriteBlocks.size()-_largeBlocksCount));
    return true; // OK
}

void MainWindow::on_btnLoad_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, "Load ROM", _lastopen, SFC_FILTER);
    if (f.isEmpty()) return;
    _file = f;
    _lastopen = _file;
    if (!loadRom()) _file = "";
    QSettings settings;
    settings.setValue("lastfile", _file);
    settings.setValue("lastopen", _lastopen);
}


void MainWindow::on_tiles_selectionChanged(int index)
{
    ui->lstBlocks->setCurrentRow(index);
}

void MainWindow::on_sprite_selectionChanged(int index)
{
    ui->lstSpriteChunks->setCurrentRow(index);
}

QString toString(const QRgb color)
{
    return QStringLiteral("#%1").arg(color, 8, 16, QLatin1Char('0'));
}
QString toString(const QVector<QRgb> colors)
{
    QString res = "";
    for (auto c : colors)
        res += toString(c) + " ";
    return res;
}

QString __verifyImage(const QImage& img, int expectedSize)
{
    if (img.isNull()) return "Invalid image";
    if (img.width()!=expectedSize || img.height()!=expectedSize) return "Invalid size";
#if 0 // since I can't find a drawing program that keeps palette in takt, we try to match colors later on instead
    if (img.pixelFormat() != QImage::toPixelFormat(QImage::Format_Indexed8)) return "Image has to be indexed/use palette";
    if (img.colorCount() != -1/*16*/) return "Invalid palette/color count: " + QString::number(img.colorCount()) + "\n" + toString(img.colorTable());
#endif
    return "";
}
bool verifyImage(const QImage& img, int expectedSize, QString* err=NULL)
{
    auto msg = __verifyImage(img, expectedSize);
    if (err) *err = msg;
    return msg.isEmpty();
}

void MainWindow::on_tiles_customContextMenuRequested(const QPoint &pos)
{
    int index = ui->tiles->selected();
    if (index<0) return;

    if (_exportdir.isEmpty()) _exportdir = QFileInfo(_lastopen).dir().path();
    QMenu contextMenu("Context menu", this);

    QAction aExport("Export", this);
    connect(&aExport, &QAction::triggered, this, [this,index](){
        QString ft = (index<_largeBlocksCount) ? QString::number(index) : ("s_" + QString::number(index-_largeBlocksCount));
        QString f = QFileInfo(_exportdir, ft + ".png").filePath();
        f = QFileDialog::getSaveFileName(this, "Export sprite block", f, PNG_FILTER);
        if (f.isEmpty()) return;
        if (!appendSuffixAskToOverwrite(this, f, ".png", "Export sprite block")) return;
        _exportdir = QFileInfo(f).dir().path();
        auto pixels = _spriteBlocks[index].getPixels();
        int size = _spriteBlocks[index].size;
        QImage img = QImage(QSize(size,size), QImage::Format_Indexed8);
        img.setColorTable(ui->tiles->itemColorMap(index).toQVector());
        for (int y=0; y<size; y++)
            for (int x=0; x<size; x++)
                img.setPixel(x, y, pixels[x+y*size]);
        if (!img.save(f))
            QMessageBox::warning(this, "Error", QStringLiteral("Could not save sprite data to %1").arg(f));
    });

    QAction aReplace("Replace", this);
    connect(&aReplace, &QAction::triggered, this, [this,index](){
        bool notYetWorkingCopy = !_workingCopies.contains(_file);
        if (notYetWorkingCopy && !_willBecomeWorkingCopy) {
            auto res = QMessageBox::warning(this, "WARNING", "Importing a sprite will modify the ROM!\nContinue?", QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
            if (res != QMessageBox::Yes) return;
            _willBecomeWorkingCopy = true; // mark as working copy as soon as the file gets modified
        }
        QString ft = (index<_largeBlocksCount) ? QString::number(index) : ("s_" + QString::number(index-_largeBlocksCount));
        QString f = QFileInfo(_exportdir, ft + ".png").filePath();
        f = QFileDialog::getOpenFileName(this, "Import sprite block", f, PNG_FILTER);
        if (!f.isEmpty()) {
            _exportdir = QFileInfo(f).dir().path();
            QImage img(f);
            QString err;
            if (!verifyImage(img, _spriteBlocks[index].size, &err)) {
                QMessageBox::critical(this, "Error", err);
                return;
            }
            ColorMap colormap = ui->tiles->itemColorMap(index);
            QByteArray pixels;
            int size = img.width();
            for (int y=0; y<size; y++) {
                for (int x=0; x<size; x++) {
                    QRgb c = img.pixel(x,y);
                    uint8_t colorindex = colormap.find(c);
                    if (colorindex>15) {
                        QMessageBox::warning(this, "Error", QStringLiteral("Could not find color %1 in palette! Make sure correct palette is selected!").arg(toString(c)));
                        return;
                    }
                    pixels.append(colorindex);
                }
            }
            if (!_spriteBlocks[index].setPixels(pixels)) {
                if (_spriteBlocks[index].compressed)
                    QMessageBox::warning(this, "Error", QString("Could not compress pixels to %1 bytes!").arg(_spriteBlocks[index].romsize));
                else
                    QMessageBox::warning(this, "Error", "Could not set pixels!");
                return;
            }
            if (!_spriteBlocks[index].save(_rom)) {
                QMessageBox::critical(this, "Error", "Could not save to ROM!");
                //return;
            }
            ui->tiles->set(index, _spriteBlocks[index]);
            int x = ui->tiles->itemX(index);
            int y = ui->tiles->itemY(index);
            ui->tiles->repaint(x, y, TileView::TILE_OUTER_SIZE, TileView::TILE_OUTER_SIZE);
            qDebug("imported %s -> %d\n", f.toUtf8().data(), index);
            if (notYetWorkingCopy && _willBecomeWorkingCopy) {
                _workingCopies.append(_file);
                QSettings settings;
                settings.setValue("workingcopies", _workingCopies);
                qDebug("set file to be a working copy. no future warnings.\n");
            }
        }
    });
    contextMenu.addAction(&aExport);
    contextMenu.addAction(&aReplace);

    contextMenu.exec(ui->tiles->mapToGlobal(pos));
}

void MainWindow::on_lstBlocks_currentRowChanged(int currentRow)
{
    ui->tiles->setSelected(currentRow);
    if (currentRow<0) return;
    int tileH = TileView::TILE_OUTER_SIZE;
    int tileY1 = ui->tiles->itemY(currentRow);
    int tileY2 = tileY1+tileH; // both borders visible
    int viewY1 = ui->scrollArea->verticalScrollBar()->value();
    int viewY2 = viewY1+ui->scrollArea->viewport()->height();
    if (tileY1<viewY1) {
        // scroll up until tile is visible
        ui->scrollArea->ensureVisible(0, tileY2, 0, tileH);
    } else if (tileY2>viewY2) {
        // scroll down until tile is visible
        ui->scrollArea->ensureVisible(0, tileY1, 0, tileH);
    }
}

void MainWindow::on_cbxDefaultColorMap_activated(int index)
{
    ui->cbxDefaultColorMap2->setCurrentIndex(index);
    ui->tiles->setColorMap(0, ColorMap::FromSnes(colorMaps[index].snescolors));
    ui->sprite->setColorMap(0, ColorMap::FromSnes(colorMaps[index].snescolors));
}

void MainWindow::on_cbxDefaultColorMap2_activated(int index)
{
    ui->cbxDefaultColorMap->setCurrentIndex(index);
    on_cbxDefaultColorMap_activated(index);
}

void MainWindow::on_cbxBackground_activated(int index)
{
    ui->cbxBackground2->setCurrentIndex(index);
    if (index<0) index = 0;
    ui->tiles->setBackground(backgrounds[index].color);
    ui->sprite->setBackground(backgrounds[index].color);
}

void MainWindow::on_cbxBackground2_activated(int index)
{
    ui->cbxBackground->setCurrentIndex(index);
    on_cbxBackground_activated(index);
}



void MainWindow::on_lstSprites_currentRowChanged(int currentRow)
{
    ui->lstSpriteChunks->clear();
    ui->sprite->clear();
    ui->sprite->addColorMap(ColorMap::FromSnes(colorMaps[ui->cbxDefaultColorMap->currentIndex()].snescolors));
    for (const auto& chunk : _spriteInfos[currentRow].chunks) {
        ui->lstSpriteChunks->addItem(chunk.toString());
        if (chunk.flags&1) // 16x16 sprite block
            ui->sprite->add(chunk, _spriteBlocks[chunk.block]);
        else
            ui->sprite->add(chunk, _spriteBlocks[chunk.block+_largeBlocksCount]);
    }
    ui->sprite->repaint();
}

void MainWindow::on_lstSpriteChunks_currentRowChanged(int currentRow)
{
    ui->sprite->setSelected(currentRow);
    if (currentRow>=0 && ui->lstSprites->currentRow()) {
        // sync with other tab
        const auto& chunk = _spriteInfos[ui->lstSprites->currentRow()].chunks[currentRow];
        int index = (int)chunk.block + ((chunk.flags&1) ? 0 : _largeBlocksCount);
        ui->lstBlocks->setCurrentRow(index);
    }
}
