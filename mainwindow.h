#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextDocument>
#include <QListWidgetItem>
#include "rom.h"
#include "spriteblock.h"
#include "spriteinfo.h"
#include "characterdata.h"
#include "tile.h"
#include "text.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void on_btnLoad_clicked();

    void on_lstBlocks_currentRowChanged(int currentRow);
    void on_tilesSprites_selectionChanged(int index);
    void on_tilesSprites_customContextMenuRequested(const QPoint &pos);

    void on_lstSprites_currentRowChanged(int currentRow);
    void on_lstSpriteChunks_currentRowChanged(int currentRow);
    void on_sprite_selectionChanged(int index);

    void on_lstMapTiles_currentRowChanged(int currentRow);
    void on_tilesMapTiles_selectionChanged(int index);

    void on_cbxDefaultColorMap_activated(int index);
    void on_cbxDefaultColorMap2_activated(int index);
    void on_cbxMapTilesColorMap_activated(int index);

    void on_cbxBackground_activated(int index);
    void on_cbxBackground2_activated(int index);
    void on_cbxBackground3_activated(int index);

    void on_tabWidget_currentChanged(int index);

    void on_txtScripts_cursorPositionChanged();
    void on_hexScripts_cursorPositionChanged();

    void on_lstScripts_currentRowChanged(int currentRow);
    void on_lstScripts_itemClicked(QListWidgetItem *item);

    void on_cbxScriptColor_activated(int index);

    void on_btnScriptsSaveHTML_clicked();

    void on_lstCharacters_currentRowChanged(int currentRow);

    void on_btnCharacterNameRelocate_clicked();

    void on_lstTexts_currentRowChanged(int currentRow);

private:
    Ui::MainWindow *ui;
    QString _baseTitle;
    QString _file;
    QString _lastopen;
    QString _lastsave_html;
    QString _generatedScriptsHTML;
    QString _exportdir;
    QStringList _workingCopies;
    QStringList _warnedCopies;
    bool _willBecomeWorkingCopy = false;
    Rom* _rom = NULL;
    QList<SpriteBlock> _spriteBlocks;
    int _largeBlocksCount=0;
    QList<SpriteInfo> _spriteInfos;
    QList<CharacterData> _characterData;
    QList<Tile> _mapTiles;
    QList<Text> _texts;

    QString _lastSearch;
    bool _scriptLoading = false;
    QList<int> _scriptShortcuts;
    QTextDocument::FindFlags _findFlags;
    bool _findRegex;

    bool loadRom();
    bool saveHTML(QString f);
    bool findNext(bool backwards);
};
#endif // MAINWINDOW_H
