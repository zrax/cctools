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

#ifndef _CCEDIT_H
#define _CCEDIT_H

#include <QMainWindow>
#include <QProcess>
#include "EditorWidget.h"
#include "History.h"
#include "libcc1/Levelset.h"
#include "libcc1/DacFile.h"
#include "libcc1/CCMetaData.h"

class QMenu;
class QAction;
class QActionGroup;
class QListWidget;
class QUndoStack;

class EditorTabWidget;
class LevelProperties;

class CCEditMain : public QMainWindow {
    Q_OBJECT

public:
    explicit CCEditMain(QWidget* parent = nullptr);

    void loadLevelset(const QString& filename);
    void saveLevelset(const QString& filename);
    bool closeLevelset();
    void loadTileset(CCETileset* tileset);
    void populateTilesets();

    void loadLevel(int level);
    void loadLevel(ccl::LevelData* levelPtr);

    EditorWidget* getEditorAt(int idx);
    EditorWidget* currentEditor();
    EditorWidget* addEditor(ccl::LevelData* level);
    void closeAllTabs();

signals:
    void tilesetChanged(CCETileset*);
    void leftTileChanged(tile_t);
    void rightTileChanged(tile_t);

public slots:
    void createNewLevelset();

private:
    enum ActionType {
        ActionNew, ActionOpen, ActionSave, ActionSaveAs, ActionCloseTab,
        ActionCloseLevelset,
        ActionGenReport, ActionExit, ActionSelect, ActionCut, ActionCopy,
        ActionPaste, ActionClear, ActionUndo, ActionRedo, ActionDrawPencil,
        ActionDrawLine, ActionDrawRect, ActionDrawFill, ActionDrawFlood,
        ActionPathMaker, ActionConnect, ActionAdvancedMech, ActionInspectTiles,
        ActionToggleWalls, ActionCheckErrors,
        ActionViewButtons, ActionViewMovers,
        ActionViewActivePlayer,
        ActionViewViewport, ActionViewMonsterPaths, ActionViewErrors,
        ActionZoom200, ActionZoom150,
        ActionZoom100, ActionZoom75, ActionZoom50, ActionZoom25, ActionZoom125,
        ActionZoomCust, ActionZoomFit, ActionTestChips, ActionTestTWorldCC,
        ActionTestTWorldLynx, ActionTestLexy, ActionTestSetup, ActionAbout,
        ActionAddLevel, ActionDelLevel, ActionMoveUp, ActionMoveDown,
        ActionProperties, ActionOrganize,  ActionViewCloners, ActionViewTraps,
        ActionViewDRCloners, ActionViewMultiTanks, ActionViewButtonsOnMouse,
        NUM_ACTIONS
    };

    QAction* m_actions[NUM_ACTIONS];
    QActionGroup* m_drawModeGroup;
    QActionGroup* m_modalToolGroup;
    QMenu* m_recentFiles;
    QMenu* m_tilesetMenu;
    QActionGroup* m_tilesetGroup;
    QActionGroup* m_scaleGroup;
    QUndoStack* m_undoStack;
    EditorUndoCommand* m_undoCommand;
    CCETileset* m_currentTileset;
    ActionType m_savedDrawMode;
    EditorWidget::DrawMode m_currentDrawMode;
    double m_zoomFactor;

    QDockWidget* m_levelManDock;
    EditorTabWidget* m_editorTabs;
    QListWidget* m_levelList;
    LevelProperties* m_levelProperties;
    tile_t m_leftTile, m_rightTile;

    ccl::Levelset* m_levelset;
    unsigned int m_dirtyFlag;
    ccl::DacFile m_dacInfo;
    CCX::Levelset m_ccxFile;
    QString m_levelsetFilename;
    bool m_useDac, m_haveCcx;
    bool m_checkSave;

    enum SubprocType { SubprocMSCC, SubprocTWorld };
    QProcess* m_subProc;
    SubprocType m_subProcType;
    QString m_tempExe, m_tempDat, m_tempIni;

    int levelIndex(ccl::LevelData* level);
    void updateForUndoCommand(const QUndoCommand*);

protected:
    void closeEvent(QCloseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;

private:
    void registerTileset(const QString& filename);
    void doLevelsetLoad();
    void setLevelsetFilename(const QString& filename);
    void populateRecentFiles();

private slots:
    void onOpenAction();
    void onSaveAction();
    void onSaveAsAction();
    void onReportAction();
    void onSelectToggled(bool);
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
    void onPathMakerToggled(bool);
    void onConnectToggled(bool);
    void onAdvancedMechAction();
    void onInspectTilesToggled(bool);
    void onToggleWallsAction();
    void onCheckErrorsAction();
    void onViewButtonsToggled(bool);
    void onViewMoversToggled(bool);
    void onViewClonersToggled(bool);
    void onViewTrapsToggled(bool);
    void onViewDRClonersToggled(bool);
    void onViewMultiTanksToggled(bool);
    void onViewActivePlayerToggled(bool);
    void onViewViewportToggled(bool);
    void onViewMonsterPathsToggled(bool);
    void onViewErrorsToggled(bool);
    void onViewButtonsOnMouseToggled(bool);
    void setZoomFactor(double);
    void onZoomCust();
    void onZoomFit();
    void onTilesetMenu(QAction*);
    void onTestChips();
    void onTestTWorld(unsigned int levelsetType);
    void onTestLexy();

    void onTilePicked(int x, int y);

    void beginEdit(CCEditHistory::Type type);
    void endEdit();
    void cancelEdit();
    void onCleanChanged(bool);

    void onAddLevelAction();
    void onDelLevelAction();
    void onMoveUpAction();
    void onMoveDownAction();
    void onPropertiesAction();
    void onOrganizeAction();

    void onSelectLevel(int);
    void onChipCountAction();
    void onNameChanged(const std::string&);
    void onAuthorChanged(const std::string&);
    void onPasswordChanged(const std::string&);
    void onChipsChanged(int);
    void onTimerChanged(int);
    void onHintChanged(const std::string&);
    void onClipboardDataChanged();

    void onTabChanged(int);

    void setLeftTile(tile_t);
    void setRightTile(tile_t);

    void onProcessFinished(int, QProcess::ExitStatus);
    void onProcessError(QProcess::ProcessError);
};

#endif
