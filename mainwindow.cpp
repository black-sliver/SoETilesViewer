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
#include "colormap.h"


struct PredefinedColorMap {
    const char* name;
    uint16_t snescolors[16];
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
    ui->lstBlocks->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    for (auto& map: colorMaps)
        ui->cbxDefaultColorMap->addItem(map.name);

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


bool MainWindow::loadRom()
{
    setWindowTitle(_baseTitle);
    _willBecomeWorkingCopy = false; // opening a new file will ask the question again

    if (_rom) delete _rom;
    _rom = new Rom(_file);

    ui->lstBlocks->clear();
    ui->tiles->clear();

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
    for (int i=0; i<4854; i++) { // TODO: how many are there actually?
        SpriteBlock block(i, _rom);
        if (_rom->mapaddr(block.dataaddr) >= 0x300000-8 /*3MB*/) continue;
        _spriteBlocks.append(block);
        ui->lstBlocks->addItem(block.toString());
        ui->tiles->add(block);
    }
    //ui->lstBlocks->adjustSize();
    ui->tiles->repaint();

    setWindowTitle(_baseTitle + " - " + QFileInfo(_file).fileName());

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

QString __verifyImage(const QImage& img)
{
    if (img.isNull()) return "Invalid image";
    if (img.width()!=16 || img.height()!=16) return "Invalid size";
#if 0 // since I can't find a drawing program that keeps palette in takt, we try to match colors later on instead
    if (img.pixelFormat() != QImage::toPixelFormat(QImage::Format_Indexed8)) return "Image has to be indexed/use palette";
    if (img.colorCount() != -1/*16*/) return "Invalid palette/color count: " + QString::number(img.colorCount()) + "\n" + toString(img.colorTable());
#endif
    return "";
}
bool verifyImage(const QImage& img, QString* err=NULL)
{
    auto msg = __verifyImage(img);
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
        QString f = QFileInfo(_exportdir, QString::number(index) + ".png").filePath();
        f = QFileDialog::getSaveFileName(this, "Export sprite block", f, PNG_FILTER);
        if (f.isEmpty()) return;
        if (!appendSuffixAskToOverwrite(this, f, ".png", "Export sprite block")) return;
        _exportdir = QFileInfo(f).dir().path();
        auto pixels = _spriteBlocks[index].getPixels();
        QImage img = QImage(QSize(16,16), QImage::Format_Indexed8);
        img.setColorTable(ui->tiles->itemColorMap(index).toQVector());
        for (int y=0; y<16; y++)
            for (int x=0; x<16; x++)
                img.setPixel(x, y, pixels[x+y*16]);
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
        QString f = QFileInfo(_exportdir, QString::number(index) + ".png").filePath();
        f = QFileDialog::getOpenFileName(this, "Import sprite block", f, PNG_FILTER);
        if (!f.isEmpty()) {
            _exportdir = QFileInfo(f).dir().path();
            QImage img(f);
            QString err;
            if (!verifyImage(img, &err)) {
                QMessageBox::critical(this, "Error", err);
                return;
            }
            ColorMap colormap = ui->tiles->itemColorMap(index);
            QByteArray pixels;
            for (int y=0; y<16; y++) {
                for (int x=0; x<16; x++) {
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
                settings.setValue("workingcopies", _file);
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
    ui->tiles->setColorMap(0, ColorMap::FromSnes(colorMaps[index].snescolors));
}
