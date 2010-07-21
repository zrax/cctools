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
#include <QToolButton>
#include <QPlainTextEdit>
#include <QTabWidget>
#include "../Levelset.h"
#include "../Tileset.h"
#include "EditorWidget.h"

class CCEditMain : public QMainWindow {
    Q_OBJECT

public:
    CCEditMain(QWidget* parent = 0);

    void loadLevelset(QString filename);
    bool closeLevelset();
    void loadTileset(QString filename);
    void findTilesets();

private:
    enum ActionType {
        ActionNew, ActionOpen, ActionSave, ActionSaveAs, ActionExit,
        ActionSelect, ActionCut, ActionCopy, ActionPaste, ActionClear,
        ActionFill, ActionUndo, ActionRedo,
        NUM_ACTIONS
    };

    enum TileListId {
        ListStandard, ListObstacles, ListDoors, ListItems, ListMonsters,
        ListMisc, ListSpecial, NUM_TILE_LISTS
    };

    QAction* m_actions[NUM_ACTIONS];
    EditorWidget* m_editor;
    QListWidget* m_levelList;
    QLineEdit* m_nameEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_chipEdit;
    QLineEdit* m_timeEdit;
    QToolButton* m_passwordButton;
    QToolButton* m_chipsButton;
    QPlainTextEdit* m_hintEdit;
    QListWidget* m_tileLists[NUM_TILE_LISTS];
    QMenu* m_tilesetMenu;
    CCETileset m_tileset;

    ccl::Levelset* m_levelset;
    QString m_levelsetFilename;

private slots:
    void onOpenAction();
    void onSelectToggled(bool);
    void onSelectLevel(int);
    void onPasswordGenAction();
    void onChipCountAction();
    void onNameChanged(QString);
    void onPasswordChanged(QString);
    void onChipsChanged(QString);
    void onTimerChanged(QString);
};

#endif
