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

#ifndef _ORGANIZER_H
#define _ORGANIZER_H

#include <QDialog>
#include <QListWidget>
#include <QAction>
#include "../Levelset.h"
#include "../Tileset.h"

Q_DECLARE_METATYPE(ccl::LevelData*);

class LevelListWidget : public QListWidget {
    Q_OBJECT

public:
    LevelListWidget(QWidget* parent = 0);
    virtual ~LevelListWidget();

    void setTileset(CCETileset* tileset) { m_tileset = tileset; }
    void addLevel(ccl::LevelData* level);
    void delLevel(int row);

    ccl::LevelData* level(int row)
    {
        return item(row)->data(Qt::UserRole).value<ccl::LevelData*>();
    }

protected:
    virtual void paintEvent(QPaintEvent*);

private:
    CCETileset* m_tileset;
    void loadLevelImage(int row);
};


class OrganizerDialog : public QDialog {
    Q_OBJECT

public:
    OrganizerDialog(QWidget* parent = 0);

    void loadLevelset(ccl::Levelset* levelset);
    void setTileset(CCETileset* tileset) { m_levels->setTileset(tileset); }

private slots:
    void saveChanges();
    void updateActions();
    void onDeleteLevels();

private:
    ccl::Levelset* m_levelset;
    LevelListWidget* m_levels;

    enum ActionType {
        ActionCut, ActionCopy, ActionPaste, ActionDelete, ActionCount
    };
    QAction* m_actions[ActionCount];
};

#endif
