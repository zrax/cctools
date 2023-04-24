/******************************************************************************
 * This file is part of CCTools.                                              *
 *                                                                            *
 * CCTools is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * CCTools is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with CCTools.  If not, see <http://www.gnu.org/licenses/>.           *
 ******************************************************************************/

#ifndef _CC2EDIT_H
#define _CC2EDIT_H

#include <QMainWindow>
#include <QProcess>
#include "libcc2/Map.h"
#include "libcc2/Tileset.h"
#include "EditorWidget.h"
#include "ScriptEditor.h"

class QLabel;
class QListWidget;
class QListWidgetItem;
class QActionGroup;

class EditorTabWidget;
class MapProperties;

class CC2EditMain : public QMainWindow {
    Q_OBJECT

public:
    explicit CC2EditMain(QWidget* parent = nullptr);

    void createNewMap();
    void createNewScript();
    void loadFile(const QString& filename);
    bool loadMap(const QString& filename, bool floatTab);
    bool loadScript(const QString& filename);
    void editScript(const QString& filename);
    void closeScript();
    bool saveTab(int index);
    bool saveTabAs(int index);
    bool saveMap(cc2::Map* map, const QString& filename);
    bool saveScript(const QString& script, const QString& filename);

    void populateTilesets();
    void loadTileset(CC2ETileset* tileset);

    CC2EditorWidget* getEditorAt(int idx);
    CC2EditorWidget* currentEditor();
    CC2EditorWidget* addEditor(cc2::Map* map, const QString& filename, bool floatTab);
    CC2ScriptEditor* getScriptEditorAt(int idx);
    CC2ScriptEditor* currentScriptEditor();
    CC2ScriptEditor* addScriptEditor(const QString& filename);
    void closeAllTabs();

signals:
    void tilesetChanged(CC2ETileset*);
    void leftTileChanged(const cc2::Tile&);
    void rightTileChanged(const cc2::Tile&);

protected:
    void closeEvent(QCloseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    bool eventFilter(QObject*, QEvent*) override;

private slots:
    void onOpenAction();
    void onImportCC1Action();
    void onSaveAction();
    void onSaveAsAction();
    void onReportAction();
    void onSelectToggled(bool);
    void onClipboardDataChanged();
    void onCutAction();
    void onCopyAction();
    void onPasteAction();
    void onClearAction();
    void onUndoAction();
    void onRedoAction();
    void onDrawPencilAction(bool);
    void onDrawLineAction(bool);
    void onDrawRectAction(bool);
    void onDrawFillAction(bool);
    void onDrawFloodAction(bool);
    void onPathMakerAction(bool);
    void onDrawWireAction(bool);
    void onInspectHints(bool);
    void onInspectTiles(bool);
    void onToggleGreensAction();

    void onViewViewportToggled(bool);
    void onViewMonsterPathsToggled(bool);

    void onTilePicked(int x, int y);

    void setZoomFactor(double);
    void onZoomCust();
    void onZoomFit();
    void onTilesetMenu(QAction*);
    void onTestChips2();
    void onTestLexy();

    void onTabClosed(int);
    void onTabChanged(int);

    void setLeftTile(const cc2::Tile&);
    void setRightTile(const cc2::Tile&);

    void onTitleChanged(const std::string&);
    void onAuthorChanged(const std::string&);
    void onLockChanged(const std::string&);
    void onEditorVersionChanged(const std::string&);
    void onTimeLimitChanged(int);
    void onViewportChanged(cc2::MapOption::Viewport);
    void onBlobPatternChanged(cc2::MapOption::BlobPattern);
    void onHideLogicChanged(bool);
    void onCC1BootsChanged(bool);
    void onReadOnlyChanged(bool);
    void onClueChanged(const std::string&);
    void onNoteChanged(const std::string&);
    void onResizeMap();

    void onProcessError(QProcess::ProcessError err);
    void onProcessFinished(int result, QProcess::ExitStatus status);

    void setGameName(const QString& name, const QString& filename = QString());

private:
    enum ActionType {
        ActionNewMap, ActionNewScript, ActionOpen, ActionImportCC1, ActionSave,
        ActionSaveAs, ActionCloseTab, ActionCloseGame, ActionGenReport, ActionPreferences, ActionExit,
        ActionSelect, ActionCut, ActionCopy, ActionPaste, ActionClear,
        ActionUndo, ActionRedo, ActionDrawPencil, ActionDrawLine, ActionDrawRect,
        ActionDrawFill, ActionDrawFlood, ActionPathMaker, ActionDrawWire,
        ActionInspectHints, ActionInspectTiles, ActionToggleGreens,
        ActionViewViewport, ActionViewMonsterPaths, ActionZoom200, ActionZoom150,
        ActionZoom100, ActionZoom75, ActionZoom50, ActionZoom25, ActionZoom125,
        ActionZoomCust, ActionZoomFit, ActionTestCC2, ActionTestLexy,
        ActionTestSetup, ActionAbout,
        ActionScriptHelp, ActionReloadScript, ActionEditScript,
        NUM_ACTIONS
    };

    QAction* m_actions[NUM_ACTIONS];
    QActionGroup* m_drawModeGroup;
    QActionGroup* m_modalToolGroup;
    QMenu* m_recentFiles;
    QMenu* m_tilesetMenu;
    QActionGroup* m_tilesetGroup;
    QActionGroup* m_scaleGroup;
    CC2ETileset* m_currentTileset;
    ActionType m_savedDrawMode;
    CC2EditorWidget::DrawMode m_currentDrawMode;
    double m_zoomFactor;

    QDockWidget* m_gamePropsDock;
    QDockWidget* m_mapPropsDock;

    // Game (c2g) properties
    QWidget* m_gameProperties;
    QLabel* m_gameName;
    QListWidget* m_gameMapList;
    QString m_currentGameScript;

    // Map properties
    MapProperties *m_mapProperties;

    cc2::Tile m_leftTile, m_rightTile;

    EditorTabWidget* m_editorTabs;
    QLabel* m_positionLabel;

    QProcess* m_subProc;
    QString m_testGameDir;

    void registerTileset(const QString& filename);
    void loadEditorForItem(QListWidgetItem* item);
    void populateRecentFiles();
};

#endif // _CC2EDIT_H
