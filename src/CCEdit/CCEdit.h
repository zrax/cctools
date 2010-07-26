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
#include <QToolButton>
#include <QTabWidget>
#include <QLabel>
#include "../Levelset.h"
#include "../Tileset.h"
#include "EditorWidget.h"
#include "LayerWidget.h"

class TileListWidget : public QListWidget {
    Q_OBJECT

public:
    TileListWidget(QWidget* parent = 0);
    void addTiles(QList<tile_t> tiles);

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

private:
    enum ActionType {
        ActionNew, ActionOpen, ActionSave, ActionSaveAs, ActionClose, ActionExit,
        ActionSelect, ActionCut, ActionCopy, ActionPaste, ActionClear,
        ActionFill, ActionUndo, ActionRedo, ActionDrawPencil, ActionDrawLine,
        ActionDrawFill, ActionPathMaker, ActionTrapConnect, ActionCloneConnect,
        ActionViewButtons, ActionViewTeleports, ActionViewActivePlayer,
        ActionViewMovers,
        ActionAddLevel, ActionDelLevel, ActionMoveUp, ActionMoveDown,
        ActionProperties,
        NUM_ACTIONS
    };

    enum TileListId {
        ListStandard, ListObstacles, ListDoors, ListItems, ListMonsters,
        ListMisc, ListSpecial, ListAllTiles, NUM_TILE_LISTS
    };

    QAction* m_actions[NUM_ACTIONS];
    QMenu* m_tilesetMenu;
    QActionGroup* m_tilesetGroup;

    EditorWidget* m_editor;
    QTabWidget* m_toolTabs;
    QListWidget* m_levelList;
    QLineEdit* m_nameEdit;
    QLineEdit* m_passwordEdit;
    QSpinBox* m_chipEdit;
    QSpinBox* m_timeEdit;
    QToolButton* m_passwordButton;
    QToolButton* m_chipsButton;
    QLineEdit* m_hintEdit;
    TileListWidget* m_tileLists[NUM_TILE_LISTS];
    LayerWidget* m_layer;
    QLabel* m_foreLabel;
    QLabel* m_backLabel;

    ccl::Levelset* m_levelset;
    QString m_levelsetFilename;
    ActionType m_savedDrawMode;

protected:
    void registerTileset(QString filename);
    void doLevelsetLoad();
    virtual void closeEvent(QCloseEvent*);

private slots:
    void onNewAction();
    void onOpenAction();
    void onSaveAction();
    void onSaveAsAction();
    void onCloseAction() { closeLevelset(); }
    void onSelectToggled(bool);
    void onDrawPencilAction();
    void onDrawLineAction();
    void onDrawFillAction();
    void onPathMakerAction();
    void onTrapConnectAction();
    void onCloneConnectAction();
    void onViewButtonsToggled(bool);
    void onViewTeleportsToggled(bool);
    void onViewActivePlayerToggled(bool);
    void onViewMoversToggled(bool);
    void onTilesetMenu(QAction*);

    void onAddLevelAction();
    void onDelLevelAction();
    void onMoveUpAction();
    void onMoveDownAction();

    void onSelectLevel(int);
    void onPasswordGenAction();
    void onChipCountAction();
    void onNameChanged(QString);
    void onPasswordChanged(QString);
    void onChipsChanged(int);
    void onTimerChanged(int);

    void setForeground(tile_t);
    void setBackground(tile_t);
};

#endif
