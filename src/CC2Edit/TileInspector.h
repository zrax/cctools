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
#include "libcc2/Map.h"

class QListWidget;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QGroupBox;
class QAction;
class QToolBar;
class CC2ETileset;
class TileInspector : public QDialog {
    Q_OBJECT

public:
    explicit TileInspector(QWidget* parent = nullptr);

    void setTileset(CC2ETileset* tileset);
    void loadTile(const cc2::Tile& tile);

    const cc2::Tile& tile() const { return m_tile; }

private Q_SLOTS:
    void tryAccept();
    void onChangeLayer(int layer);
    void setTileType(int type);
    void setTileModifier(int modifier);
    void setTileDirection(int dir);
    void onChangeFlag(uint8_t flag, bool on);

private:
    QListWidget* m_layers;
    CC2ETileset* m_tileset;
    cc2::Tile m_tile;
    bool m_paging;

    QComboBox* m_tileType;
    QSpinBox* m_tileTypeId;
    QSpinBox* m_tileModifier;
    QComboBox* m_tileDir;
    QSpinBox* m_tileDirValue;
    QGroupBox* m_flagsGroup;
    QCheckBox* m_tileFlags[8];
    QAction* m_addLayer;
    QAction* m_removeLayer;
    QAction* m_moveLayerUp;
    QAction* m_moveLayerDown;
    QToolBar* m_layerToolbox;
    void addLayers(const cc2::Tile* tile);
    void createLayerAbove();
    void removeLayer();
    void swapLayers(int layer);
    void moveLayerUp();
    void moveLayerDown();
    cc2::Tile* tileLayer(int index);
};

#endif
