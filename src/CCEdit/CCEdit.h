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
#include <QAction>
#include <QMenu>
#include <QLineEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QLabel>
#include <QProcess>
#include "EditorWidget.h"
#include "LayerWidget.h"
#include "TileWidgets.h"
#include "libcc1/Levelset.h"
#include "libcc1/DacFile.h"
#include "libcc1/CCMetaData.h"

class CCEditMain : public QMainWindow {
    Q_OBJECT

public:
    CCEditMain(QWidget* parent = 0);

    void loadLevelset(QString filename);
    void saveLevelset(QString filename);
    bool closeLevelset();
    void loadTileset(CCETileset* tileset);
    void findTilesets();
    void selectLevel(int level);

    EditorWidget* getEditorAt(int idx);
    EditorWidget* addEditor(ccl::LevelData* level);
    void closeAllTabs();

private:
    enum ActionType {
        ActionNew, ActionOpen, ActionSave, ActionSaveAs, ActionClose,
        ActionGenReport, ActionExit, ActionSelect, ActionCut, ActionCopy,
        ActionPaste, ActionClear, ActionUndo, ActionRedo, ActionDrawPencil,
        ActionDrawLine, ActionDrawFill, ActionPathMaker, ActionConnect,
        ActionAdvancedMech, ActionToggleWalls, ActionCheckErrors,
        ActionViewButtons, ActionViewMovers, ActionViewActivePlayer,
        ActionViewViewport, ActionViewMonsterPaths, ActionViewErrors,
        ActionZoom100, ActionZoom75, ActionZoom50, ActionZoom25, ActionZoom125,
        ActionZoomCust, ActionZoomFit, ActionTestChips, ActionTestTWorldCC,
        ActionTestTWorldLynx, ActionTestSetup, ActionAbout,
        ActionAddLevel, ActionDelLevel, ActionMoveUp, ActionMoveDown,
        ActionProperties, ActionOrganize,
        NUM_ACTIONS
    };

    enum TileListId {
        ListStandard, ListObstacles, ListDoors, ListItems, ListMonsters,
        ListMisc, ListSpecial, NUM_TILE_LISTS
    };

    QAction* m_actions[NUM_ACTIONS];
    QMenu* m_tilesetMenu;
    QActionGroup* m_tilesetGroup;
    CCETileset* m_currentTileset;
    ActionType m_savedDrawMode;
    EditorWidget::DrawMode m_currentDrawMode;
    double m_zoomFactor;

    QTabWidget* m_editorTabs;
    QTabWidget* m_toolTabs;
    QListWidget* m_levelList;
    QLineEdit* m_nameEdit;
    QLineEdit* m_passwordEdit;
    QSpinBox* m_chipEdit;
    QSpinBox* m_timeEdit;
    QLineEdit* m_hintEdit;
    TileListWidget* m_tileLists[NUM_TILE_LISTS];
    BigTileWiget* m_allTiles;
    LayerWidget* m_layer[2];
    QLabel* m_foreLabel[2];
    QLabel* m_backLabel[2];

    ccl::Levelset* m_levelset;
    ccl::DacFile m_dacInfo;
    CCX::Levelset m_ccxFile;
    QString m_levelsetFilename;
    QString m_dialogDir;
    bool m_useDac, m_haveCcx;
    bool m_checkSave;

    enum SubprocType { SubprocMSCC, SubprocTWorld };
    QProcess* m_subProc;
    SubprocType m_subProcType;
    QString m_tempExe, m_tempDat, m_tempIni;

protected:
    void registerTileset(QString filename);
    void doLevelsetLoad();
    void setLevelsetFilename(QString filename);
    virtual void closeEvent(QCloseEvent*);
    virtual void resizeEvent(QResizeEvent*);

private slots:
    void onNewAction();
    void onOpenAction();
    void onSaveAction();
    void onSaveAsAction();
    void onCloseAction() { closeLevelset(); }
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
    void onDrawFillAction(bool);
    void onPathMakerToggled(bool);
    void onConnectToggled(bool);
    void onAdvancedMechAction();
    void onToggleWallsAction();
    void onCheckErrorsAction();
    void onViewButtonsToggled(bool);
    void onViewMoversToggled(bool);
    void onViewActivePlayerToggled(bool);
    void onViewViewportToggled(bool);
    void onViewMonsterPathsToggled(bool);
    void onViewErrorsToggled(bool);
    void onZoom100();
    void onZoom75();
    void onZoom50();
    void onZoom25();
    void onZoom125();
    void onZoomCust();
    void onZoomFit();
    void onTilesetMenu(QAction*);
    void onTestChips();
    void onTestTWorld(unsigned int levelsetType);
    void onTestTWorldCC() { onTestTWorld(ccl::Levelset::TypeMS); }
    void onTestTWorldLynx() { onTestTWorld(ccl::Levelset::TypeLynx); }
    void onTestSetup();
    void onAboutAction();

    void onAddLevelAction();
    void onDelLevelAction();
    void onMoveUpAction();
    void onMoveDownAction();
    void onPropertiesAction();
    void onOrganizeAction();

    void onSelectLevel(int);
    void onPasswordGenAction();
    void onChipCountAction();
    void onNameChanged(QString);
    void onPasswordChanged(QString);
    void onChipsChanged(int);
    void onTimerChanged(int);
    void onHintChanged(QString);
    void onClipboardDataChanged();

    void onNewTab();
    void onCloseTab(int);
    void onCloseCurrentTab();
    void onTabChanged(int);
    void onDockChanged(Qt::DockWidgetArea);
    void onMakeDirty() { m_levelset->makeDirty(); }

    void setForeground(tile_t);
    void setBackground(tile_t);

    void onProcessFinished(int);
    void onProcessError(QProcess::ProcessError);
};

#endif
