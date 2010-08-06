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
#include <QListWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QLabel>
#include "EditorWidget.h"
#include "LayerWidget.h"
#include "../Levelset.h"
#include "../Tileset.h"
#include "../DacFile.h"

class TileListWidget : public QListWidget {
    Q_OBJECT

public:
    TileListWidget(QWidget* parent = 0);
    void addTiles(const QList<tile_t>& tiles);

protected:
    virtual void mousePressEvent(QMouseEvent*);
    Qt::MouseButton m_button;

signals:
    void itemSelectedLeft(tile_t);
    void itemSelectedRight(tile_t);
};


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
        ActionNew, ActionOpen, ActionSave, ActionSaveAs, ActionClose, ActionExit,
        ActionSelect, ActionCut, ActionCopy, ActionPaste, ActionClear,
        ActionUndo, ActionRedo, ActionDrawPencil, ActionDrawLine, ActionDrawFill,
        ActionPathMaker, ActionConnect, ActionAdvancedMech, ActionViewButtons,
        ActionViewMovers, ActionViewActivePlayer, ActionAddLevel, ActionDelLevel,
        ActionMoveUp, ActionMoveDown, ActionProperties,
        NUM_ACTIONS
    };

    enum TileListId {
        ListStandard, ListObstacles, ListDoors, ListItems, ListMonsters,
        ListMisc, ListSpecial, ListAllTiles, NUM_TILE_LISTS
    };

    QAction* m_actions[NUM_ACTIONS];
    QMenu* m_tilesetMenu;
    QActionGroup* m_tilesetGroup;
    CCETileset* m_currentTileset;
    ActionType m_savedDrawMode;
    EditorWidget::DrawMode m_currentDrawMode;

    QTabWidget* m_editorTabs;
    QTabWidget* m_toolTabs;
    QListWidget* m_levelList;
    QLineEdit* m_nameEdit;
    QLineEdit* m_passwordEdit;
    QSpinBox* m_chipEdit;
    QSpinBox* m_timeEdit;
    QLineEdit* m_hintEdit;
    TileListWidget* m_tileLists[NUM_TILE_LISTS];
    LayerWidget* m_layer[2];
    QLabel* m_foreLabel[2];
    QLabel* m_backLabel[2];

    ccl::Levelset* m_levelset;
    ccl::DacFile m_dacInfo;
    QString m_levelsetFilename;
    bool m_useDac;

protected:
    void registerTileset(QString filename);
    void doLevelsetLoad();
    void setLevelsetFilename(QString filename);
    virtual void closeEvent(QCloseEvent*);

private slots:
    void onNewAction();
    void onOpenAction();
    void onSaveAction();
    void onSaveAsAction();
    void onCloseAction() { closeLevelset(); }
    void onSelectToggled(bool);
    void onCutAction();
    void onCopyAction();
    void onPasteAction();
    void onClearAction();
    void onUndoAction();
    void onRedoAction();
    void onDrawPencilAction();
    void onDrawLineAction();
    void onDrawFillAction();
    void onPathMakerAction();
    void onConnectAction();
    void onAdvancedMechAction();
    void onViewButtonsToggled(bool);
    void onViewMoversToggled(bool);
    void onViewActivePlayerToggled(bool);
    void onTilesetMenu(QAction*);

    void onAddLevelAction();
    void onDelLevelAction();
    void onMoveUpAction();
    void onMoveDownAction();
    void onPropertiesAction();

    void onSelectLevel(int);
    //void onLevelDClicked(QListWidgetItem*) { onNewTab(); }
    void onPasswordGenAction();
    void onChipCountAction();
    void onNameChanged(QString);
    void onPasswordChanged(QString);
    void onChipsChanged(int);
    void onTimerChanged(int);
    void onClipboardDataChanged();

    void onNewTab();
    void onCloseTab(int);
    void onCloseCurrentTab();
    void onTabChanged(int);

    void setForeground(tile_t);
    void setBackground(tile_t);
};

#endif
