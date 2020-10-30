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

#include "TileInspector.h"
#include "libcc2/Tileset.h"
#include "CommonWidgets/CCTools.h"

#include <QListWidget>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QMessageBox>

TileInspector::TileInspector(QWidget* parent)
    : QDialog(parent), m_tileset(), m_paging(false)
{
    setWindowTitle(tr("Tile Inspector"));
    setWindowIcon(ICON("draw-inspect"));

    m_layers = new QListWidget(this);

    m_tileType = new QComboBox(this);
    for (int i = 0; i < cc2::Tile::NUM_TILE_TYPES; ++i) {
        if (i == cc2::Tile::Modifier8 || i == cc2::Tile::Modifier16
                || i == cc2::Tile::Modifier32)
            continue;

        m_tileType->addItem(CC2ETileset::baseName((cc2::Tile::Type)i), i);
    }
    m_tileType->insertSeparator(cc2::Tile::NUM_TILE_TYPES);
    m_tileType->addItem(tr("Other:"));
    auto tileTypeLabel = new QLabel(tr("Base &Type:"), this);
    tileTypeLabel->setBuddy(m_tileType);
    m_tileTypeId = new QSpinBox(this);
    m_tileTypeId->setRange(0, 255);
    m_tileTypeId->setEnabled(false);

    connect(m_tileType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        QVariant data = m_tileType->itemData(index);
        if (data.isNull()) {
            m_tileTypeId->setEnabled(true);
        } else {
            m_tileTypeId->setValue(data.toInt());
            m_tileTypeId->setEnabled(false);
        }
    });
    connect(m_tileTypeId, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &TileInspector::setTileType);

    m_tileModifier = new QSpinBox(this);
    m_tileModifier->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    m_tileModifier->setDisplayIntegerBase(16);
    auto tileModifierLabel = new QLabel(tr("&Modifier:"), this);
    tileModifierLabel->setBuddy(m_tileModifier);

    connect(m_tileModifier, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &TileInspector::setTileModifier);

    m_tileDir = new QComboBox(this);
    m_tileDir->addItem(tr("North"), (int)cc2::Tile::North);
    m_tileDir->addItem(tr("East"), (int)cc2::Tile::East);
    m_tileDir->addItem(tr("South"), (int)cc2::Tile::South);
    m_tileDir->addItem(tr("West"), (int)cc2::Tile::West);
    m_tileDir->insertSeparator(4);
    m_tileDir->addItem(tr("Other:"));
    auto tileDirLabel = new QLabel(tr("&Direction:"), this);
    tileDirLabel->setBuddy(m_tileDir);
    m_tileDirValue = new QSpinBox(this);
    m_tileDirValue->setRange(0, 255);
    m_tileDirValue->setEnabled(false);

    connect(m_tileDir, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        QVariant data = m_tileDir->itemData(index);
        if (data.isNull()) {
            m_tileDirValue->setEnabled(true);
        } else {
            m_tileDirValue->setValue(data.toInt());
            m_tileDirValue->setEnabled(false);
        }
    });
    connect(m_tileDirValue, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &TileInspector::setTileDirection);

    m_flagsGroup = new QGroupBox(tr("Tile Flags"), this);
    auto tileFlagsLayout = new QGridLayout(m_flagsGroup);
    tileFlagsLayout->setContentsMargins(4, 4, 4, 4);
    tileFlagsLayout->setSpacing(4);
    for (int i = 0; i < 8; ++i) {
        m_tileFlags[i] = new QCheckBox(this);
        tileFlagsLayout->addWidget(m_tileFlags[i], i % 4, i / 4);
        connect(m_tileFlags[i], &QCheckBox::toggled, this, [this, i](bool on) {
            onChangeFlag(1u << i, on);
        });
    }

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel,
                                        Qt::Horizontal, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &TileInspector::tryAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(8);
    layout->setHorizontalSpacing(8);
    int row = 0;
    layout->addWidget(m_layers, row, 0, 5, 1);
    layout->addWidget(tileTypeLabel, row, 1);
    layout->addWidget(m_tileType, row, 2);
    layout->addWidget(m_tileTypeId, row, 3);
    layout->addWidget(tileModifierLabel, ++row, 1);
    layout->addWidget(m_tileModifier, row, 2, 1, 2);
    layout->addWidget(tileDirLabel, ++row, 1);
    layout->addWidget(m_tileDir, row, 2);
    layout->addWidget(m_tileDirValue, row, 3);
    layout->addWidget(m_flagsGroup, ++row, 1, 1, 3);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding),
                    ++row, 1, 1, 3);
    layout->addWidget(buttons, ++row, 0, 1, 4);

    connect(m_layers, &QListWidget::currentRowChanged, this, &TileInspector::onChangeLayer);
}

void TileInspector::setTileset(CC2ETileset* tileset)
{
    m_tileset = tileset;
    m_layers->setIconSize(m_tileset->qsize());
}

void TileInspector::loadTile(const cc2::Tile& tile)
{
    Q_ASSERT(m_tileset);
    m_tile = tile;

    m_layers->clear();
    addLayers(&tile);
    m_layers->setCurrentRow(0);
}

void TileInspector::tryAccept()
{
    int rangeTile = 0;
    for (cc2::Tile* tp = &m_tile; tp; tp = tp->lower()) {
        if (tp->type() == cc2::Tile::Modifier8 || tp->type() == cc2::Tile::Modifier16
                || tp->type() == cc2::Tile::Modifier32) {
            QMessageBox::critical(this, tr("Invalid Tile"),
                    tr("Invalid tile type (%1).  Tile type must not be a modifier tile.")
                    .arg(tp->type()));
            return;
        } else if (tp->type() >= cc2::Tile::NUM_TILE_TYPES) {
            rangeTile = tp->type();
        }
    }

    if (rangeTile) {
        auto response = QMessageBox::question(this, tr("Tile out of range"),
                            tr("Tile type (%1) is outside the range of known tile types. "
                               "This may cause random and unexpected behavior, including "
                               "crashes and possible data corruption, when loading the "
                               "map in Chip's Challenge 2.").arg(rangeTile),
                            QMessageBox::Ok | QMessageBox::Cancel);
        if (response == QMessageBox::Cancel)
            return;
    }

    accept();
}

void TileInspector::onChangeLayer(int layer)
{
    if (layer < 0)
        return;

    m_paging = true;

    cc2::Tile* tile = tileLayer(layer);
    if (tile->type() < cc2::Tile::NUM_TILE_TYPES)
        m_tileType->setCurrentIndex((int)tile->type());
    else
        m_tileType->setCurrentIndex(m_tileType->count() - 1);
    m_tileTypeId->setValue((int)tile->type());
    m_tileModifier->setValue(tile->modifier());

    if (tile->direction() <= cc2::Tile::West)
        m_tileDir->setCurrentIndex((int)tile->direction());
    else
        m_tileDir->setCurrentIndex(m_tileDir->count() - 1);
    m_tileDirValue->setValue((int)tile->direction());

    for (int i = 0; i < 8; ++i)
        m_tileFlags[i]->setChecked((tile->tileFlags() & (1u << i)) != 0);

    m_paging = false;
}

void TileInspector::setTileType(int type)
{
    m_tileDir->setEnabled(cc2::Tile::haveDirection(type));
    if (!m_tileDirValue->isEnabled())
        m_tileDirValue->setEnabled(false);
    else
        m_tileDirValue->setEnabled(m_tileDir->itemData(m_tileDir->currentIndex()).isNull());

    int flagStart = 0;
    if (type == cc2::Tile::PanelCanopy) {
        m_flagsGroup->setEnabled(true);
        m_tileFlags[0]->setText(tr("Panel &North"));
        m_tileFlags[1]->setText(tr("Panel &East"));
        m_tileFlags[2]->setText(tr("Panel &South"));
        m_tileFlags[3]->setText(tr("Panel &West"));
        m_tileFlags[4]->setText(tr("&Canopy"));
        flagStart = 5;
    } else if (type == cc2::Tile::DirBlock) {
        m_flagsGroup->setEnabled(true);
        m_tileFlags[0]->setText(tr("Arrow &North"));
        m_tileFlags[1]->setText(tr("Arrow &East"));
        m_tileFlags[2]->setText(tr("Arrow &South"));
        m_tileFlags[3]->setText(tr("Arrow &West"));
        flagStart = 4;
    } else {
        m_flagsGroup->setEnabled(false);
    }
    for (int i = flagStart; i < 8; ++i)
        m_tileFlags[i]->setText(tr("Flag 0x%1").arg(1u << i, 0, 16));

    // The below applies only when changing the value from the UI
    if (m_paging)
        return;

    int layer = m_layers->currentRow();
    cc2::Tile* tile = tileLayer(layer);
    tile->setType(type);
    m_layers->item(layer)->setIcon(m_tileset->getIcon(tile));
    m_layers->item(layer)->setText(CC2ETileset::getName(tile));

    // Re-populate lower layers in the layer list
    while (m_layers->count() > layer + 1)
        delete m_layers->takeItem(layer + 1);
    cc2::Tile* lower = tile->lower();
    if (lower)
        addLayers(lower);
}

void TileInspector::setTileModifier(int modifier)
{
    // The below applies only when changing the value from the UI
    if (m_paging)
        return;

    int layer = m_layers->currentRow();
    cc2::Tile* tile = tileLayer(layer);
    tile->setModifier((uint32_t)modifier);
    m_layers->item(layer)->setIcon(m_tileset->getIcon(tile));
    m_layers->item(layer)->setText(CC2ETileset::getName(tile));
}

void TileInspector::setTileDirection(int dir)
{
    // The below applies only when changing the value from the UI
    if (m_paging)
        return;

    int layer = m_layers->currentRow();
    cc2::Tile* tile = tileLayer(layer);
    tile->setDirection((cc2::Tile::Direction)dir);
    m_layers->item(layer)->setIcon(m_tileset->getIcon(tile));
    m_layers->item(layer)->setText(CC2ETileset::getName(tile));
}

void TileInspector::onChangeFlag(uint8_t flag, bool on)
{
    // The below applies only when changing the value from the UI
    if (m_paging)
        return;

    int layer = m_layers->currentRow();
    cc2::Tile* tile = tileLayer(layer);
    if (on)
        tile->setTileFlags(tile->tileFlags() | flag);
    else
        tile->setTileFlags(tile->tileFlags() & ~flag);
    m_layers->item(layer)->setIcon(m_tileset->getIcon(tile));
    m_layers->item(layer)->setText(CC2ETileset::getName(tile));
}

void TileInspector::addLayers(const cc2::Tile* tile)
{
    while (tile) {
        auto item = new QListWidgetItem(m_layers);
        item->setIcon(m_tileset->getIcon(tile));
        item->setText(CC2ETileset::getName(tile));
        tile = tile->lower();
    }
}

cc2::Tile* TileInspector::tileLayer(int index)
{
    cc2::Tile* tile = &m_tile;
    for (int i = 0; i < index; ++i)
        tile = tile->lower();
    return tile;
}
