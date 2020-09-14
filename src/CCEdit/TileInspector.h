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

#ifndef _TILE_INSPECTOR_H
#define _TILE_INSPECTOR_H

#include <QDialog>
#include "libcc1/Levelset.h"

class QLabel;
class QComboBox;
class QSpinBox;
class QGroupBox;
class CCETileset;

class TileInspector : public QDialog {
    Q_OBJECT

public:
    explicit TileInspector(QWidget* parent = nullptr);

    void setTileset(CCETileset* tileset);
    void setTile(tile_t upper, tile_t lower);

    tile_t upper() const;
    tile_t lower() const;

private Q_SLOTS:
    void tryAccept();
    void setUpperType(int type);
    void setLowerType(int type);

private:
    QLabel* m_upperImage;
    QLabel* m_lowerImage;
    CCETileset* m_tileset;

    QComboBox* m_upperType;
    QSpinBox* m_upperTypeId;
    QComboBox* m_lowerType;
    QSpinBox* m_lowerTypeId;
};

#endif
