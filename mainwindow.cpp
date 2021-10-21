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
#include <QTextStream>
#include <QTextBlock>
#include <QTimer>
#include <QDateTime>
#include "colormap.h"
#include "finddialog.h"
#include "scriptparser.h"
#include "tile.h"
#include "text.h"


struct PredefinedColorMap {
    const char* name;
    uint16_t snescolors[16];
};
struct PredefinedBackground {
    const char* name;
    QRgb color;
};


constexpr auto SFC_FILTER = "SNES ROM (*.sfc *.smc);;All Files (*)";
constexpr auto PNG_FILTER = "PNG Image (*.png);;All Files (*)";
constexpr auto HTML_FILTER = "HTML (*.html *.htm);;All Files (*)";
constexpr auto LIST_DARK_THEME = "QListWidget::item{color:#eee;background-color:#070707;} QListWidget::item:selected{color:#fff;background-color:#28282d;}";
static const QColor TEXT_HIGHLIGHT_DEFAULT = QColor(0,0,64,20);
static const QColor TEXT_HIGHLIGHT_DARK = QColor(40,40,45,255);

constexpr PredefinedColorMap spriteColorMaps[] = {
    { "Boy",          { 0, 0x7fbc, 0x4edf, 0x2dba, 0x1913, 0x1489, 0x7eaa, 0x7923, 0x5465, 0x02bf, 0x059c, 0x08f6, 0x07ea, 0x0287, 0x1885, 0x0000 } },
    { "Act0 Dog",     { 0, 0x7fde, 0x52d8, 0x4253, 0x35f0, 0x298d, 0x62f7, 0x4e73, 0x4210, 0x35ac, 0x4a10, 0x39ac, 0x2d29, 0x1cc6, 0x14a5, 0x0000 } },
    { "Act1 Dog",     { 0, 0x7fbc, 0x7eaa, 0x7923, 0x4eb6, 0x1d4d, 0x3e11, 0x212a, 0x0c86, 0x0c63, 0x4534, 0x2cf4, 0x24cd, 0x10aa, 0x1063, 0x0000 } },
    { "Act2 Dog",     { 0, 0x7ffe, 0x7b99, 0x6715, 0x5690, 0x39aa, 0x2927, 0x18a3, 0x5259, 0x41b6, 0x27ed, 0x27ed, 0x27ed, 0x27ed, 0x1884, 0x0000 } },
    { "Act3 Dog",     { 0, 0x6eff, 0x5a5b, 0x49d8, 0x3d54, 0x2cf1, 0x208d, 0x184a, 0x27ed, 0x27ed, 0x27ed, 0x27ed, 0x27ed, 0x27ed, 0x1063, 0x0000 } },
    { "Act4 Dog",     { 0, 0x7f9c, 0x7b13, 0x0013, 0x001f, 0x0013, 0x0007, 0x2def, 0x298c, 0x6e70, 0x59ec, 0x4149, 0x3506, 0x28a3, 0x1441, 0x0000 } },
    { "Cat",          { 0, 0x4f9f, 0x4eff, 0x3639, 0x1238, 0x09f6, 0x0193, 0x0151, 0x012d, 0x0154, 0x0176, 0x0178, 0x10df, 0x166e, 0x10a3, 0x0000 } },
    { "Intro NPC",    { 0, 0x7ffc, 0x4ebd, 0x2198, 0x0932, 0x00aa, 0x4665, 0x2523, 0x00d0, 0x00cd, 0x6815, 0x6850, 0x542e, 0x400b, 0x18c7, 0x0000 } },
    { "1965 NPCs",    { 0, 0x5bdd, 0x579b, 0x4f59, 0x4b17, 0x42d5, 0x3e93, 0x3651, 0x320e, 0x29cc, 0x258a, 0x1d48, 0x1906, 0x10c4, 0x0c82, 0x0000 } },
    { "Name Cursor",  { 0, 0x2fff, 0x16d9, 0x056f, 0x0067, 0x63ff, 0x3f39, 0x1e73, 0x0dad, 0x00c7, 0x16df, 0x0a38, 0x0937, 0x502e, 0x4841, 0x0000 } },
    { "Ring Menu",    { 0, 0x7fde, 0x66f6, 0x3d8b, 0x02bf, 0x015f, 0x0010, 0x25fa, 0x1111, 0x7c1f, 0x3c0f, 0x1f47, 0x01a0, 0x7f2c, 0x7c80, 0x0420 } },
    { "Ring Cursor",  { 0, 0x7fbc, 0x4edf, 0x2dba, 0x1913, 0x1489, 0x728b, 0x6586, 0x44c3, 0x025e, 0x0537, 0x0491, 0x52d8, 0x2df0, 0x1c85, 0x0000 } },
    { "Copyright",    { 0, 0x39ce, 0x77bd, 0x3dfa, 0x0014, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 } },
    { "Spell Icons 2",{ 0, 0x7fff, 0x03bf, 0x02bf, 0x01bf, 0x001d, 0x0010, 0x5b36, 0x4270, 0x2dcb, 0x1926, 0x0882, 0x1954, 0x0cce, 0x0448, 0x0000 } },
    { "Hard Ball",    { 0, 0x4ed3, 0x4270, 0x2dcb, 0x1926, 0x0882, 0x4ed3, 0x4ed3, 0x4270, 0x2dcb, 0x1926, 0x0882, 0x4ed3, 0x4270, 0x1505, 0x0441 } },
    { "Turd Ball",    { 0, 0x4eb6, 0x4253, 0x2dae, 0x1909, 0x0464, 0x4eb6, 0x4eb6, 0x4253, 0x2dae, 0x1909, 0x0464, 0x4eb6, 0x4253, 0x14e8, 0x0422 } },
};
constexpr PredefinedColorMap mapColorMaps[] = {
    { "Podunk 1",     { 0, 0x1464, 0x1485, 0x18a7, 0x24e8, 0x312a, 0x3d6c, 0x2d6d, 0x31af, 0x3a12, 0x3e55, 0x4297, 0x4b3b, 0x57ff, 0x000a, 0x0005 } }, // Buildings
    { "1965 Tiles 1", { 0, 0x0c82, 0x10a3, 0x14e5, 0x1907, 0x1d48, 0x258a, 0x29ac, 0x2ded, 0x320f, 0x3a51, 0x3e92, 0x42b4, 0x5bdd, 0x0000, 0x0000 } }, // Buildings
    { "Omnitopia 1",  { 0, 0x6358, 0x5ef5, 0x4ed4, 0x4251, 0x39ed, 0x31ab, 0x298b, 0x2949, 0x2108, 0x2106, 0x18e6, 0x14a5, 0x0c83, 0x0442, 0x0000 } }, // Grill
    { "Omnit. Tree",  { 0, 0x0843, 0x0c65, 0x1087, 0x14a9, 0x01ed, 0x0967, 0x04c3, 0x0040, 0x14a5, 0x18c7, 0x1ce8, 0x1d09, 0x254a, 0x2d8c, 0x31ce } },
    { "Jungle 1",     { 0, 0x19ef, 0x15ac, 0x116a, 0x1127, 0x0ce5, 0x08a3, 0x0481, 0x0040, 0x08a8, 0x0485, 0x0064, 0x0022, 0x10a6, 0x0864, 0x0443 } }, // Most bushes with no color accent
    { "Hut Int. 1",   { 0, 0x0c43, 0x10a5, 0x1507, 0x1d6a, 0x0020, 0x0c61, 0x18c4, 0x3168, 0x1024, 0x1066, 0x14c8, 0x190a, 0x214c, 0x25ae, 0x0461 } },
    { "Hut Ext. 1",   { 0, 0x0843, 0x0c66, 0x14c6, 0x1908, 0x0c63, 0x14e8, 0x1d8a, 0x262d, 0x0866, 0x112b, 0x1a11, 0x22d6, 0x4926, 0x3189, 0x0423 } },
    { "Thraxx Body",  { 0, 0x0090, 0x006e, 0x044b, 0x0829, 0x0c27, 0x0405, 0x0803, 0x0801, 0x0d5c, 0x04f6, 0x639d, 0x367d, 0x03e0, 0x03e0, 0x0000 } },
    { "Thraxx Eyes",  { 0, 0x7bff, 0x73bd, 0x677b, 0x5b19, 0x4ed7, 0x4295, 0x3a33, 0x2df1, 0x35ae, 0x2d6c, 0x252a, 0x20c8, 0x1886, 0x1044, 0x0402 } },
    { "Title Text",   { 0, 0x0400, 0x0442, 0x0464, 0x04a7, 0x04c9, 0x050b, 0x052d, 0x056f, 0x0591, 0x05d4, 0x05f6, 0x0638, 0x065a, 0x0a9c, 0x0abe } },
    { "Title Sky",    { 0, 0x4587, 0x4166, 0x3d46, 0x3925, 0x3125, 0x2d04, 0x28e4, 0x24c3, 0x20a3, 0x1c82, 0x1482, 0x1061, 0x0c41, 0x0820, 0x0400 } },
    { "Title Machine",{ 0, 0x0885, 0x0463, 0x0442, 0x0021, 0x0000, 0x0000, 0x0443, 0x0022, 0x0001, 0x0000, 0x18ee, 0x10aa, 0x0c67, 0x0423, 0x0000 } },
    { "Title Pipes",  { 0, 0x0863, 0x0442, 0x0442, 0x0421, 0x0421, 0x0021, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 } },
    { "Title Grill",  { 0, 0x0044, 0x0023, 0x0023, 0x0022, 0x0022, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0067 } },

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
    ui->btnLoad->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    auto fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->lstBlocks->setFont(fixedFont);
    ui->lstSprites->setFont(fixedFont);
    ui->lstSpriteChunks->setFont(fixedFont);
    ui->lstCharacters->setFont(fixedFont);
    ui->lstMapTiles->setFont(fixedFont);
    ui->btnCharacterNameRelocate->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    ui->btnCharacterNameRelocate->setToolTip("Relocate...");
    ui->btnCharacterNameRelocate->setText("");
    ui->lstScripts->setFont(fixedFont);
    ui->txtScripts->setFont(fixedFont);
    ui->hexScripts->setFont(fixedFont);

    // add shortcut text and filter for tab switching
    for (int i=0; i<ui->tabWidget->count(); i++) {
        const QString s = (i==0) ? " [Ctrl+1]" : (" ["+QString::number(i+1)+"]");
        ui->tabWidget->setTabText(i, ui->tabWidget->tabText(i) + s);
    }
    installEventFilter(this);
    ui->txtScripts->installEventFilter(this);
    ui->hexScripts->installEventFilter(this);
    for (const auto& cbx : { ui->cbxBackground,
                             ui->cbxBackground2,
                             ui->cbxBackground3,
                             ui->cbxDefaultColorMap,
                             ui->cbxDefaultColorMap2,
                             ui->cbxMapTilesColorMap })
        cbx->installEventFilter(this);

    // primary color maps (sprite for sprites, map for map tiles)
    for (auto& map: spriteColorMaps) {
        ui->cbxDefaultColorMap->addItem(map.name);
        ui->cbxDefaultColorMap2->addItem(map.name);
    }
    for (auto& map: mapColorMaps) {
        ui->cbxMapTilesColorMap->addItem(map.name);
    }
#if 0
    // secondary color maps (map for sprites, sprite for map tiles)
    for (auto& map: spriteColorMaps) {
        ui->cbxMapTilesColorMap->addItem(map.name);
    }
    for (auto& map: mapColorMaps) {
        ui->cbxDefaultColorMap->addItem(map.name);
        ui->cbxDefaultColorMap2->addItem(map.name);
    }
#endif

    // background color
    for (auto& bg: backgrounds) {
        ui->cbxBackground->addItem(bg.name);
        ui->cbxBackground2->addItem(bg.name);
        ui->cbxBackground3->addItem(bg.name);
    }
    for (auto& scriptcolor: {"System","Dark"}) {
        ui->cbxScriptColor->addItem(scriptcolor);
    }

    _baseTitle = windowTitle();

    QSettings settings;
    ui->cbxScriptColor->setCurrentIndex(settings.value("darkmode", false).toBool()?1:0);
    if (ui->cbxScriptColor->currentIndex())
        ui->lstScripts->setStyleSheet(LIST_DARK_THEME);
    _file = settings.value("lastfile", "").toString();
    _lastopen = settings.value("lastopen", "").toString();
    if (_lastopen.isEmpty()) _lastopen = settings.value("lastdir", "").toString();
    _lastsave_html = settings.value("lastsave_html", "soe.html").toString();
    _exportdir = settings.value("exportdir", "").toString();
    _workingCopies = settings.value("workingcopies").toStringList();
    _warnedCopies  = settings.value("warnedcopies").toStringList();
    if (!_file.isEmpty()) {
        if (! loadRom()) {
            _file = "";
            settings.setValue("lastfile", _file);
        }
    }
    _findFlags = (QTextDocument::FindFlags) settings.value("findflags", 0).toUInt();
    _findRegex = settings.value("findregex", false).toBool();

    // synchronize scripts zoom and scroll position between text and hex
    QScrollBar* sbar = ui->txtScripts->verticalScrollBar();
    connect(sbar, &QScrollBar::valueChanged, this, [this]() {
        ui->hexScripts->verticalScrollBar()->setValue(ui->txtScripts->verticalScrollBar()->value());
    } );
    connect(sbar, &QScrollBar::rangeChanged, this, [this]() {
        ui->hexScripts->setFont(ui->txtScripts->font());
        ui->hexScripts->verticalScrollBar()->setValue(ui->txtScripts->verticalScrollBar()->value());
    } );
    sbar = ui->hexScripts->verticalScrollBar();
    connect(sbar, &QScrollBar::valueChanged, this, [this]() {
        ui->txtScripts->verticalScrollBar()->setValue(ui->hexScripts->verticalScrollBar()->value());
    } );
    connect(sbar, &QScrollBar::rangeChanged, this, [this]() {
        ui->txtScripts->setFont(ui->hexScripts->font());
        ui->txtScripts->verticalScrollBar()->setValue(ui->hexScripts->verticalScrollBar()->value());
    } );

    QTimer::singleShot(100, this, [this]() {
        QApplication::processEvents();
        on_tabWidget_currentChanged(ui->tabWidget->currentIndex());
    });

#ifdef WIN32
    ui->txtScripts->setPlaceholderText(ui->txtScripts->placeholderText() + "\nThe app may be unresponsive for an additional minute!");
#endif
}

MainWindow::~MainWindow()
{
    QSettings settings;
    if (settings.value("exportdir") != _exportdir)
        settings.setValue("exportdir", _exportdir);
    if (_findFlags != (QTextDocument::FindFlags) settings.value("findflags", 0).toUInt())
        settings.setValue("findflags", (unsigned)_findFlags);
    if (_findRegex != settings.value("findregex", false).toBool())
        settings.setValue("findregex", _findRegex);
    if ((ui->cbxScriptColor->currentIndex()?true:false) != settings.value("darkmode",false).toBool())
        settings.setValue("darkmode", ui->cbxScriptColor->currentIndex()?true:false);
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type()==QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if ((key->modifiers() & Qt::ControlModifier) || key->key() == Qt::Key_F3||
                key->key()==Qt::Key_Home || key->key()==Qt::Key_End)
        {
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
            case Qt::Key_F: // ctrl+f = find, shift=reverse search direction
                {
                auto dlg = new FindDialog(this, _lastSearch, _findFlags, _findRegex);
                if (dlg->exec() == QDialog::Accepted) {
                    _lastSearch = dlg->text();
                    _findFlags = dlg->flags();
                    _findRegex = dlg->regex();
                    if (!_lastSearch.isEmpty()) findNext(key->modifiers() & Qt::ShiftModifier);
                }
                delete dlg;
                break;
                }
            case Qt::Key_F3:
            case Qt::Key_G: // F3 = ctrl+g = find next, shift=reverse search direction
                if (!_lastSearch.isEmpty()) findNext(key->modifiers() & Qt::ShiftModifier);
                break;
            case Qt::Key_Home: // Home in scripts goes to first line
                if (ui->tabWidget->currentWidget()==ui->tabScripts) {
                    ui->txtScripts->moveCursor(QTextCursor::Start);
                } else return QObject::eventFilter(obj, event);
                break;
            case Qt::Key_End: // End in scripts goes to last line
                if (ui->tabWidget->currentWidget()==ui->tabScripts) {
                    ui->txtScripts->moveCursor(QTextCursor::End);
                } else return QObject::eventFilter(obj, event);
                break;
            default:
                return QObject::eventFilter(obj, event);
            }
        } else {
            switch (key->key())
            {
            default:
                return QObject::eventFilter(obj, event);
            }
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

    int oldCharacterIndex = ui->lstCharacters->currentRow();

    ui->lstSprites->clear();
    ui->lstBlocks->clear();
    ui->tilesSprites->clear();
    ui->lstCharacters->clear();
    ui->lblStats->setText("");
    ui->lstScripts->clear();
    ui->hexScripts->clear();
    ui->txtScripts->clear();
    ui->lstTexts->clear();

    _spriteBlocks.clear();
    _characterData.clear();
    _texts.clear();

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
            if (!appendSuffixAskToOverwrite(this, f, ".sfc", "Save copy")) return false;
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

    ui->tilesSprites->addColorMap(ColorMap::FromSnes(spriteColorMaps[ui->cbxDefaultColorMap->currentIndex()].snescolors));
    // 16x16 blocks
    for (int i=0; i<4854; i++) { // TODO: how many are there actually?
        SpriteBlock block(i, _rom);
        if (_rom->mapaddr(block.dataaddr) >= 0x300000-8 /*3MB*/) continue;
        _spriteBlocks.append(block);
        ui->lstBlocks->addItem(block.toString());
        ui->tilesSprites->add(block);
    }
    _largeBlocksCount = _spriteBlocks.size();
    // 8x8 blocks
    for (int i=0; i<6265; i++) { // TODO: how many are there actually?
        SpriteBlock block(i, _rom, 8);
        if (_rom->mapaddr(block.dataaddr) >= 0x300000-2 /*3MB*/) continue;
        _spriteBlocks.append(block);
        ui->lstBlocks->addItem(block.toString());
        ui->tilesSprites->add(block);
    }
    //ui->lstBlocks->adjustSize();
    ui->tilesSprites->repaint();

    ui->tilesMapTiles->addColorMap(ColorMap::FromSnes(mapColorMaps[ui->cbxMapTilesColorMap->currentIndex()].snescolors));
    for (int i=0; i<6687; i++) {
        Tile tile(i, _rom);
        _mapTiles.append(tile);
        ui->lstMapTiles->addItem(tile.toString());
        ui->tilesMapTiles->add(tile);
    }
    ui->tilesMapTiles->repaint();


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

    for (int i=0; i<142; i++) { // TODO: any indication on how many there are in rom?
        CharacterData character(i, _rom);
        _characterData.append(character);
        ui->lstCharacters->addItem(character.toString());
    }
    if (oldCharacterIndex<0 || oldCharacterIndex>=_characterData.length())
        oldCharacterIndex = 0; // select first
    ui->lstCharacters->setCurrentRow(oldCharacterIndex);

    // load texts
    for (int i=0; i<5000; i++) {
        auto text = Text(i, _rom);
        if (text.text.length()==0) {
            printf("%d %04x @ 0x%06x: <empty>\n", i, (unsigned)i*3, (unsigned)text.ptraddr);
            break;
        }
        else if (!text.valid) {
            printf("%d %04x @ 0x%06x: <invalid>\n", i, (unsigned)i*3, (unsigned)text.ptraddr);
            break;
        }
        _texts.append(text);
        ui->lstTexts->addItem(QString::fromStdString(text.text));
    }

    setWindowTitle(_baseTitle + " - " + QFileInfo(_file).fileName());
    ui->lblStats->setText(QStringLiteral("%1 sprites, %2+%3 sprite tiles, %4 map tiles, %5 texts")
                          .arg(_spriteInfos.size())
                          .arg(_largeBlocksCount)
                          .arg(_spriteBlocks.size()-_largeBlocksCount)
                          .arg(_mapTiles.size())
                          .arg(_texts.size()));

    on_tabWidget_currentChanged(ui->tabWidget->currentIndex()); // update scripts if on scripts tab

    return true; // OK
}

bool MainWindow::saveHTML(QString f) {
    QFile html(f);
    if (!html.open(QIODevice::WriteOnly)) {
        return false;
    }
    QTextStream out(&html);

    //out << ui->txtScripts->toHtml();
    out << _generatedScriptsHTML;

    html.close();
    return true;
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


void MainWindow::on_tilesSprites_selectionChanged(int index)
{
    ui->lstBlocks->setCurrentRow(index);
}

void MainWindow::on_sprite_selectionChanged(int index)
{
    ui->lstSpriteChunks->setCurrentRow(index);
}

void MainWindow::on_tilesMapTiles_selectionChanged(int index)
{
    ui->lstMapTiles->setCurrentRow(index);
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

void MainWindow::on_tilesSprites_customContextMenuRequested(const QPoint &pos)
{
    int index = ui->tilesSprites->selected();
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
        img.setColorTable(ui->tilesSprites->itemColorMap(index).toQVector());
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
            ColorMap colormap = ui->tilesSprites->itemColorMap(index);
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
            ui->tilesSprites->set(index, _spriteBlocks[index]);
            int x = ui->tilesSprites->itemX(index);
            int y = ui->tilesSprites->itemY(index);
            ui->tilesSprites->repaint(x, y, TileView::TILE_OUTER_SIZE, TileView::TILE_OUTER_SIZE);
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

    contextMenu.exec(ui->tilesSprites->mapToGlobal(pos));
}

void MainWindow::on_lstBlocks_currentRowChanged(int currentRow)
{
    ui->tilesSprites->setSelected(currentRow);
    if (currentRow<0) return;
    int tileH = TileView::TILE_OUTER_SIZE;
    int tileY1 = ui->tilesSprites->itemY(currentRow);
    int tileY2 = tileY1+tileH; // both borders visible
    int viewY1 = ui->scrollBlockView->verticalScrollBar()->value();
    int viewY2 = viewY1+ui->scrollBlockView->viewport()->height();
    if (tileY1<viewY1) {
        // scroll up until tile is visible
        ui->scrollBlockView->ensureVisible(0, tileY2, 0, tileH);
    } else if (tileY2>viewY2) {
        // scroll down until tile is visible
        ui->scrollBlockView->ensureVisible(0, tileY1, 0, tileH);
    }
}

void MainWindow::on_lstMapTiles_currentRowChanged(int currentRow)
{
    ui->tilesMapTiles->setSelected(currentRow);
    if (currentRow<0) return;
    int tileH = TileView::TILE_OUTER_SIZE;
    int tileY1 = ui->tilesMapTiles->itemY(currentRow);
    int tileY2 = tileY1+tileH; // both borders visible
    int viewY1 = ui->scrollMapTiles->verticalScrollBar()->value();
    int viewY2 = viewY1+ui->scrollMapTiles->viewport()->height();
    if (tileY1<viewY1) {
        // scroll up until tile is visible
        ui->scrollMapTiles->ensureVisible(0, tileY2, 0, tileH);
    } else if (tileY2>viewY2) {
        // scroll down until tile is visible
        ui->scrollMapTiles->ensureVisible(0, tileY1, 0, tileH);
    }
}

void MainWindow::on_cbxDefaultColorMap_activated(int index)
{
    ui->cbxDefaultColorMap2->setCurrentIndex(index);
    ui->tilesSprites->setColorMap(0, ColorMap::FromSnes(spriteColorMaps[index].snescolors));
    ui->sprite->setColorMap(0, ColorMap::FromSnes(spriteColorMaps[index].snescolors));
}

void MainWindow::on_cbxDefaultColorMap2_activated(int index)
{
    ui->cbxDefaultColorMap->setCurrentIndex(index);
    on_cbxDefaultColorMap_activated(index);
}

void MainWindow::on_cbxMapTilesColorMap_activated(int index)
{
    ui->tilesMapTiles->setColorMap(0, ColorMap::FromSnes(mapColorMaps[index].snescolors));
}

void MainWindow::on_cbxBackground_activated(int index)
{
    ui->cbxBackground2->setCurrentIndex(index);
    ui->cbxBackground3->setCurrentIndex(index);
    if (index<0) index = 0;
    ui->tilesSprites->setBackground(backgrounds[index].color);
    ui->sprite->setBackground(backgrounds[index].color);
    ui->tilesMapTiles->setBackground(backgrounds[index].color);
}

void MainWindow::on_cbxBackground2_activated(int index)
{
    ui->cbxBackground->setCurrentIndex(index);
    on_cbxBackground_activated(index);
}


void MainWindow::on_cbxBackground3_activated(int index)
{
    ui->cbxBackground->setCurrentIndex(index);
    on_cbxBackground_activated(index);
}


void MainWindow::on_lstSprites_currentRowChanged(int currentRow)
{
    ui->lstSpriteChunks->clear();
    ui->sprite->clear();
    ui->sprite->addColorMap(ColorMap::FromSnes(spriteColorMaps[ui->cbxDefaultColorMap->currentIndex()].snescolors));
    if (currentRow<0) return;
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
    if (currentRow>=0 && ui->lstSprites->currentRow()>=0) {
        // sync with other tab
        const auto& chunk = _spriteInfos[ui->lstSprites->currentRow()].chunks[currentRow];
        int index = (int)chunk.block + ((chunk.flags&1) ? 0 : _largeBlocksCount);
        ui->lstBlocks->setCurrentRow(index);
    }
}

static int script_select(QTextBrowser* src, QTextBrowser* dst, QColor color)
{
    QTextCursor cursor = src->textCursor();

    // select current line in src
    QTextBrowser::ExtraSelection selection;
    selection.format.setBackground(color);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = cursor;
    selection.cursor.clearSelection();
    QList<QTextBrowser::ExtraSelection> extraSelections;
    extraSelections.append(selection);
    src->setExtraSelections(extraSelections);

    // count line number ...
    cursor.movePosition(QTextCursor::StartOfLine);
    int lineNo = 0;
    while(cursor.positionInBlock()>0) {
        cursor.movePosition(QTextCursor::Up);
        lineNo++;
    }
    QTextBlock block = cursor.block().previous();
    while(block.isValid()) {
        lineNo += block.lineCount();
        block = block.previous();
    }

    // ... to synchronize with dst
    cursor = QTextCursor(dst->document()->findBlockByLineNumber(lineNo));
    selection.cursor = cursor;
    selection.cursor.clearSelection();
    extraSelections.clear();
    extraSelections.append(selection);
    dst->setExtraSelections(extraSelections);

    // return lineNo to synchronize with list
    return lineNo;
}

void MainWindow::on_txtScripts_cursorPositionChanged()
{
    if (_scriptLoading) return;
    int lineNo = script_select(ui->txtScripts, ui->hexScripts,
                               ui->cbxScriptColor->currentIndex() ?
                                   TEXT_HIGHLIGHT_DARK : TEXT_HIGHLIGHT_DEFAULT);
    // synchronize with list unless it has signals blocked
    // (which indicates the list set the cursor)
    if (ui->lstScripts->signalsBlocked()) return;
    int row = -1;
    for (auto& shortcut: _scriptShortcuts) {
        if (shortcut>lineNo) break;
        row++;
    }
    if (row>=0) {
        ui->lstScripts->blockSignals(true);
        ui->lstScripts->setCurrentRow(row);
        ui->lstScripts->blockSignals(false);
    }
}

void MainWindow::on_hexScripts_cursorPositionChanged()
{
    if (_scriptLoading) return;
    int lineNo = script_select(ui->hexScripts, ui->txtScripts,
                               ui->cbxScriptColor->currentIndex() ?
                                   TEXT_HIGHLIGHT_DARK : TEXT_HIGHLIGHT_DEFAULT);
    // synchronize with list unless it has signals blocked
    // (which indicates the list set the cursor)
    if (ui->lstScripts->signalsBlocked()) return;
    int row = -1;
    for (auto& shortcut: _scriptShortcuts) {
        if (shortcut>lineNo) break;
        row++;
    }
    if (row>=0) {
        ui->lstScripts->blockSignals(true);
        ui->lstScripts->setCurrentRow(row);
        ui->lstScripts->blockSignals(false);
    }
}

void MainWindow::on_lstScripts_currentRowChanged(int currentRow)
{
    if (currentRow<0) return;
    ui->lstScripts->blockSignals(true);
    auto cursor = QTextCursor(ui->txtScripts->document()->findBlockByLineNumber(_scriptShortcuts[currentRow]));
    ui->txtScripts->moveCursor(QTextCursor::End); // scroll from bottom
    ui->txtScripts->setTextCursor(cursor); // to have line of interest on top
    ui->lstScripts->blockSignals(false);
}

void MainWindow::on_lstScripts_itemClicked(QListWidgetItem *item)
{
    // scroll if clicked while already selected
    if (!item) return;
    int currentRow = item->listWidget()->row(item);
    on_lstScripts_currentRowChanged(currentRow);
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    (void)index; // ignore unused warning
    if (ui->tabWidget->currentWidget() == ui->tabScripts && ui->txtScripts->toPlainText().length()==0)
    {
        auto t0 = QDateTime::currentMSecsSinceEpoch();
        // TODO: since the regex stuff and string cat on 8MB may take some time,
        //       wrap non-ui part to run it in a different thread when loading ROM, then
        //       update GUI here once that is done
        _scriptLoading = true;
        _scriptShortcuts.clear();
        ui->btnScriptsSaveHTML->setEnabled(false);
        ui->lstScripts->clear();
        ui->hexScripts->clear();
        ui->txtScripts->clear();
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QApplication::processEvents();
        ui->txtScripts->setUpdatesEnabled(false);
        ui->hexScripts->setUpdatesEnabled(false);
        ui->lstScripts->setUpdatesEnabled(false);

        QByteArray fn = QFile::encodeName(_file);
        std::string err;
        QString s = QString::fromStdString(ScriptParser(fn).parse(&err,ui->cbxScriptColor->currentIndex()));
        if (!err.empty()) {
            QString qsErr = QString::fromStdString(err);
            if (!qsErr.startsWith("WARN:")) QMessageBox::warning(this, "Parse Error", QString::fromStdString(err));
            else qWarning("-- ScriptParser --\n%s--\n", qUtf8Printable(qsErr));
        }
        qDebug("generated %dKB of HTML\n", s.length()/1024);
        auto lines = s.split("\n");
        QRegularExpression reHexSpan("\\s*<span[^>]*class=\"hex\"[^>]*>([^>]*)</span>");
        QRegularExpression reHexI("\\s*<i>([^>]*)</i>");
        // store generated HTML for later use
        _generatedScriptsHTML = s.replace(reHexSpan, "").replace(reHexI, "");
        ui->txtScripts->setHtml(_generatedScriptsHTML);

        QRegularExpression reSpan("<span[^>]*>");
        QRegularExpression reFont("<font[^>]*>");
        QRegularExpression reMapId("^\\[0x..\\] ");
        QRegularExpression reMapAt(" at 0x[0-9a-fA-F]+$");
        QRegularExpression reAbsScript("^\\s+\"[^\"]*\" = ");
        auto undecorate = [&reSpan,&reFont,&reHexSpan,&reHexI](QString s) -> QString {
            return s.replace("</span>","")
                    .replace("</font>","")
                    .replace("</b>","")
                    .replace("<b>","")
                    .replace(reSpan,"")
                    .replace(reFont,"")
                    .replace(reHexSpan,"")
                    .replace(reHexI,"");
        };
        auto reduce = [&reMapId,&reMapAt](QString s) -> QString {
            QString t = s.replace(reMapAt, ""); // makes a copy after in-line replace
            if (t.replace(reMapId, "") != "Unknown") return t;
            return s;
        };
#if QT_VERSION >= 0x050900 // actually I don't know what the first version is that auto-casts QStringRef when doing +
        auto extract = [](const QString& s) -> QStringRef {
            auto p1 = s.indexOf((QChar)'\"')+1;
            auto p2 = s.indexOf((QChar)'\"', p1);
            return s.midRef(p1, p2-p1);
        };
#else
        auto extract = [](const QString& s) -> QString {
            auto p1 = s.indexOf((QChar)'\"')+1;
            auto p2 = s.indexOf((QChar)'\"', p1);
            return s.mid(p1, p2-p1);
        };
#endif

        QString hexData;
        int n=-1;
        for (const auto& line: lines) {
            n++;
            auto tmp = (line.startsWith("</span>") || line.startsWith("</font>")) ?
                        line.mid(7) : line; // TODO: QStringRef
            bool isTitle = (tmp.startsWith("<span class=\"h\">") ||
                            tmp.startsWith("<span class=\"t\">") ||
                            tmp.startsWith("<font color=\"#770\">"));
            if (isTitle) {
                ui->lstScripts->addItem(reduce(undecorate(tmp)));
                _scriptShortcuts.append(n);
            } else if (reAbsScript.match(tmp).hasMatch()) {
                ui->lstScripts->addItem("  "+extract(tmp));
                _scriptShortcuts.append(n);
            }
            auto hexMatch = reHexI.match(tmp);
            if (! hexMatch.hasMatch()) hexMatch = reHexSpan.match(tmp);
            if (hexMatch.hasMatch() && hexData.length()) hexData += "\n"+hexMatch.captured(1);
            else if (hexMatch.hasMatch()) hexData += hexMatch.captured(1);
            else if (hexData.length()) hexData += "\n&nbsp;";
            else hexData += "&nbsp;";
        }

        ui->hexScripts->setHtml((ui->cbxScriptColor->currentIndex()?
                                     "<head><style>*{background:#070707;color:#eee;}</style></heah><body><pre>":
                                     "<body><pre>")+hexData+"</pre></body>");

        ui->txtScripts->verticalScrollBar()->setValue(0);
        _scriptLoading = false;
        on_txtScripts_cursorPositionChanged();

        ui->lstScripts->setUpdatesEnabled(true);
        ui->btnScriptsSaveHTML->setEnabled(true);

        QTimer::singleShot(0, this, [t0,this]() {
            auto t1 = QDateTime::currentMSecsSinceEpoch();
            qDebug("UI was blocked for %ld ms\n", (long)(t1-t0));
            ui->txtScripts->setUpdatesEnabled(true);
            ui->hexScripts->setUpdatesEnabled(true);
            QApplication::restoreOverrideCursor();
        });
    }
}

bool MainWindow::findNext(bool backwards)
{
    auto flags = _findFlags;
    if (backwards) flags |= QTextDocument::FindBackward;

    if (ui->tabWidget->currentWidget() == ui->tabTexts) {
        // search texts if on that page
        int i = ui->lstTexts->currentRow();
        int n = ui->lstTexts->count();
        if (i < 0)
            i = 0;
        else if (backwards)
            i--;
        else
            i++;
        bool is_cs = flags & QTextDocument::FindCaseSensitively;
        auto cs = is_cs ? Qt::CaseSensitive : Qt::CaseInsensitive;
        QRegularExpression term(_lastSearch, is_cs ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
        while (i>=0 && i<n) {
            if (!_findRegex) {
                if (ui->lstTexts->item(i)->text().contains(_lastSearch, cs)) {
                    ui->lstTexts->setCurrentRow(i);
                    return true;
                }
            } else {
                auto match = term.match(ui->lstTexts->item(i)->text());
                if (match.hasMatch()) {
                    ui->lstTexts->setCurrentRow(i);
                    return true;
                }
            }
            if (backwards)
                i--;
            else
                i++;
        }
        return false;
    } else {
        // desfault to search scripts
        ui->tabWidget->setCurrentWidget(ui->tabScripts);
        ui->txtScripts->setFocus(); // make sure selection is visible

        auto document = ui->txtScripts->document();
        const QTextCursor cur = ui->txtScripts->textCursor();
        QTextCursor match;
        if (_findRegex) {
            QRegularExpression term(_lastSearch);
            match = document->find(term, cur, flags);
        } else {
            auto term = _lastSearch;
            match = document->find(term, cur, flags);
        }
        if (match.isNull()) return false;

        ui->txtScripts->setTextCursor(match);
    }
    return true;
}

void MainWindow::on_cbxScriptColor_activated(int index)
{
    // set list style
    ui->lstScripts->setStyleSheet(index ? LIST_DARK_THEME : "");
    // generate HTML with new style
    ui->txtScripts->clear();
    on_tabWidget_currentChanged(ui->tabWidget->currentIndex());
}

void MainWindow::on_btnScriptsSaveHTML_clicked()
{
    QString f = QFileDialog::getSaveFileName(this, "Save scripts HTML", _lastsave_html, HTML_FILTER);
    if (f.isEmpty()) return;
    if (!saveHTML(f)) {
        QMessageBox::warning(this, "Error", "Could not save HTML!");
        return;
    }
    _lastsave_html = f;
    QSettings settings;
    settings.setValue("lastsave_html", _lastsave_html);
}

static void setTextAndTooltip(QLineEdit* w, QString text, QString tooltip)
{
    w->setText(text);
    w->setToolTip(tooltip);
}
static void setTextAndAddrTooltip(QLineEdit* w, QString text, unsigned addr)
{
    setTextAndTooltip(w, text, QStringLiteral("$%1").arg(addr,0,16));
}
static void setHexTextAndAddrTooltip(QLineEdit* w, unsigned value, unsigned addr)
{
    setTextAndAddrTooltip(w, QStringLiteral("0x%1").arg(value,0,16), addr);
}
static void setDecTextAndAddrTooltip(QLineEdit* w, unsigned value, unsigned addr)
{
    setTextAndAddrTooltip(w, QString::number(value), addr);
}

void MainWindow::on_lstCharacters_currentRowChanged(int currentRow)
{
    if (currentRow<0 || currentRow>_characterData.length()) return;
    auto& c = _characterData[currentRow];
    setDecTextAndAddrTooltip(ui->txtCharacterId, c.i, c.getAddr());
    ui->txtCharacterName->setMaxLength(c.namelength);
    ui->txtCharacterName->setPlaceholderText(c.namehint);
    setHexTextAndAddrTooltip(ui->txtCharacterUnknown03,  c.unknown03, c.getUnknown03Addr());
    setHexTextAndAddrTooltip(ui->txtCharacterUnknown05,  c.unknown05, c.getUnknown05Addr());
    setHexTextAndAddrTooltip(ui->txtCharacterFlags,      c.flags, c.getFlagsAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterPalette,    c.palette, c.getPaletteAddr());
    setTextAndTooltip(ui->txtCharacterName,              c.name, QStringLiteral("$%1: $%2").arg(c.getNamePtrAddr(),0,16).arg(c.nameptr,0,16));
    setHexTextAndAddrTooltip(ui->txtCharacterUnknown0b,  c.unknown0b, c.getUnknown0bAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterUnknown0d,  c.unknown0d, c.getUnknown0dAddr());
    setDecTextAndAddrTooltip(ui->txtCharacterHP,         c.hp, c.getHPAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterUnknown11,  c.unknown11, c.getUnknown11Addr());
    setDecTextAndAddrTooltip(ui->txtCharacterAggroRange, c.aggro_range, c.getAggroRangeAddr());
    setDecTextAndAddrTooltip(ui->txtCharacterAggroChance,c.aggro_chance, c.getAggroChanceAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterUnknown17,  c.unknown17, c.getUnknown17Addr());
    setDecTextAndAddrTooltip(ui->txtCharacterAttack,     c.attack, c.getAttackAddr());
    setDecTextAndAddrTooltip(ui->txtCharacterDefense,    c.defense, c.getDefenseAddr());
    setDecTextAndAddrTooltip(ui->txtCharacterMDefense,   c.magic_defense, c.getMagicDefenseAddr());
    setDecTextAndAddrTooltip(ui->txtCharacterEvade,      c.evade, c.getEvadeAddr());
    setDecTextAndAddrTooltip(ui->txtCharacterHitRate,    c.hit_rate, c.getHitRateAddr());
    setDecTextAndAddrTooltip(ui->txtCharacterExp,        c.exp, c.getExpAddr());
    setDecTextAndAddrTooltip(ui->txtCharacterMoney,      c.money, c.getMoneyAddr());
    setDecTextAndAddrTooltip(ui->txtCharacterPrizeChance,c.prize_chance, c.getPrizeChanceAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterUnknown2a,  c.unknown2a, c.getUnknown2aAddr());
    setDecTextAndAddrTooltip(ui->txtCharacterChargeLimit,c.charge_limit, c.getChargeLimitAddr());
    setDecTextAndAddrTooltip(ui->txtCharacterChargeRate, c.charge_rate, c.getChargeRateAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterAttackProc, c.attack_proc, c.getAttackProcAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterAnimStand,  c.anim_stand, c.getAnimStandAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterAnimWalk,   c.anim_walk, c.getAnimWalkAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterAnimRun,    c.anim_run, c.getAnimRunAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterAnimAtk0,   c.anim_atk0, c.getAnimAtk0Addr());
    setHexTextAndAddrTooltip(ui->txtCharacterAnimAtk1,   c.anim_atk1, c.getAnimAtk1Addr());
    setHexTextAndAddrTooltip(ui->txtCharacterAnimAtk2,   c.anim_atk2, c.getAnimAtk2Addr());
    setHexTextAndAddrTooltip(ui->txtCharacterAnimAtk3,   c.anim_atk3, c.getAnimAtk3Addr());
    setHexTextAndAddrTooltip(ui->txtCharacterAnimDamage, c.anim_damage, c.getAnimDamageAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterAnimDeath,  c.anim_death, c.getAnimDeathAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterAnimSpoils, c.anim_spoils, c.getAnimSpoilsAddr());
    setHexTextAndAddrTooltip(ui->txtCharacterAnimBlock,  c.anim_block, c.getAnimBlockAddr());
}

void MainWindow::on_btnCharacterNameRelocate_clicked()
{
    QMessageBox::information(this, "Not implemented", "Relocation not implemented!");
}

void MainWindow::on_lstTexts_currentRowChanged(int currentRow)
{
    if (currentRow < 0 || currentRow >= _texts.size()) {
        ui->txtTextNumber->clear();
        ui->txtTextOffset->clear();
        ui->txtTextPtrAddr->clear();
        ui->txtTextDataAddr->clear();
        ui->txtTextDataLen->clear();
        ui->chkTextCompressed->setChecked(false);
        return;
    }
    Text& text = _texts[currentRow];
    ui->txtTextNumber->setText(QString::number(text.i));
    ui->txtTextOffset->setText("$" + QString::number(3 * text.i, 16));
    ui->txtTextPtrAddr->setText("$" + QString::number(text.ptraddr, 16));
    ui->txtTextDataAddr->setText("$" + QString::number(text.dataaddr, 16));
    ui->txtTextDataLen->setText(QString::number(text.datalen));
    ui->chkTextCompressed->setChecked(text.compressed);
}
