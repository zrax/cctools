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

#include "TileWidgets.h"

#include <QMouseEvent>
#include <QPainter>

Q_DECLARE_METATYPE(const cc2::Tile*)

LayerWidget::LayerWidget(QWidget* parent)
    : QFrame(parent), m_tileset()
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void LayerWidget::setTileset(CC2ETileset* tileset)
{
    m_tileset = tileset;
    resize(sizeHint());
    updateGeometry();
    update();
}

void LayerWidget::setUpper(const cc2::Tile& tile)
{
    m_upper = tile;
    update();
}

void LayerWidget::setLower(const cc2::Tile& tile)
{
    m_lower = tile;
    update();
}

void LayerWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    if (!m_tileset)
        return;

    QPainter painter(this);
    painter.scale(m_tileset->uiScale(), m_tileset->uiScale());
    const int halfway = m_tileset->size() / 2;
    m_tileset->drawAt(painter, halfway, halfway, &m_lower, false);
    m_tileset->drawAt(painter, 0, 0, &m_upper, false);
}


void TileListWidget::setTiles(std::vector<cc2::Tile> tiles)
{
    m_tiles = std::move(tiles);
    for (const cc2::Tile& tile : m_tiles) {
        auto item = new QListWidgetItem(CC2ETileset::getName(&tile), this);
        item->setData(Qt::UserRole, QVariant::fromValue(&tile));
    }
}

const cc2::Tile* TileListWidget::tile(int index)
{
    auto tileItem = item(index);
    if (tileItem)
        return tileItem->data(Qt::UserRole).value<const cc2::Tile*>();
    return nullptr;
}

void TileListWidget::setTileImages(CC2ETileset* tileset)
{
    setIconSize(tileset->iconSize());
    for (int i = 0; i < count(); ++i)
        item(i)->setIcon(tileset->getIcon(tile(i)));
}

void TileListWidget::mousePressEvent(QMouseEvent* event)
{
    QAbstractItemView::mousePressEvent(event);
    if (currentItem() == nullptr)
        return;

    if (event->button() == Qt::LeftButton)
        emit tileSelectedLeft(*currentItem()->data(Qt::UserRole).value<const cc2::Tile*>());
    else if (event->button() == Qt::RightButton)
        emit tileSelectedRight(*currentItem()->data(Qt::UserRole).value<const cc2::Tile*>());
    setCurrentItem(nullptr);
}


BigTileWidget::BigTileWidget(QWidget* parent)
    : QWidget(parent), m_tileset(), m_view(ViewTiles)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);

    // These are (mostly) in the same order as CC2.  We face south by default.
    m_tiles = std::vector<cc2::Tile>{
        cc2::Tile(cc2::Tile::Floor),
        cc2::Tile(cc2::Tile::Player, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Player2, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Transformer),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth | cc2::Tile::ArrowEast),
        cc2::Tile::panelTile(cc2::Tile::Canopy),
        cc2::Tile(cc2::Tile::Gravel),
        cc2::Tile(cc2::Tile::SpeedShoes),
        cc2::Tile(cc2::Tile::Exit),
        cc2::Tile(cc2::Tile::MirrorPlayer, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::MirrorPlayer2, cc2::Tile::South, 0),
        cc2::Tile::panelTile(cc2::Tile::PanelSouth),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth | cc2::Tile::ArrowSouth),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth | cc2::Tile::ArrowSouth | cc2::Tile::ArrowEast),
        cc2::Tile(cc2::Tile::Disallow),
        cc2::Tile(cc2::Tile::Dirt),
        cc2::Tile(cc2::Tile::HikingBoots),
        cc2::Tile(cc2::Tile::Socket),
        cc2::Tile(cc2::Tile::MaleOnly),
        cc2::Tile(cc2::Tile::FemaleOnly),
        cc2::Tile(cc2::Tile::Wall),
        cc2::Tile::dirBlockTile(cc2::Tile::AllArrows),
        cc2::Tile(cc2::Tile::IceBlock, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::ToolThief),
        cc2::Tile(cc2::Tile::Bribe),
        cc2::Tile(cc2::Tile::TimeBonus),
        cc2::Tile(cc2::Tile::Chip),
        cc2::Tile(cc2::Tile::StayUpGWall),
        cc2::Tile(cc2::Tile::PopDownGWall),
        cc2::Tile(cc2::Tile::StyledWall, cc2::TileModifier::CamoTheme),
        cc2::Tile(cc2::Tile::StyledFloor, cc2::TileModifier::CamoTheme),
        cc2::Tile(cc2::Tile::DirtBlock, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::KeyThief),
        cc2::Tile(cc2::Tile::TimeBomb),
        cc2::Tile(cc2::Tile::TimePenalty),
        cc2::Tile(cc2::Tile::ExtraChip),
        cc2::Tile(cc2::Tile::BlueWall),
        cc2::Tile(cc2::Tile::BlueFloor),
        cc2::Tile(cc2::Tile::InvisWall),
        cc2::Tile(cc2::Tile::AppearingWall),
        cc2::Tile(cc2::Tile::SteelWall),
        cc2::Tile(cc2::Tile::SteelFoil),
        cc2::Tile(cc2::Tile::Hook),
        cc2::Tile(cc2::Tile::ToggleClock),
        cc2::Tile(cc2::Tile::YellowTankCtrl),
        cc2::Tile(cc2::Tile::YellowTank, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Ant, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Ship, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Ball, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::AngryTeeth, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Blob, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Ghost, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Flag10),
        cc2::Tile(cc2::Tile::TankButton),
        cc2::Tile(cc2::Tile::BlueTank, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Centipede, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::FireBox, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Walker, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::TimidTeeth, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Rover, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::FloorMimic, cc2::Tile::South, 0),
        cc2::Tile(cc2::Tile::Flag100),
        cc2::Tile(cc2::Tile::AreaCtlButton),
        cc2::Tile(cc2::Tile::PopUpWall),
        cc2::Tile(cc2::Tile::Ice_SE),
        cc2::Tile(cc2::Tile::Ice),
        cc2::Tile(cc2::Tile::IceCleats),
        cc2::Tile(cc2::Tile::Teleport_Blue),
        cc2::Tile(cc2::Tile::Key_Blue),
        cc2::Tile(cc2::Tile::Door_Blue),
        cc2::Tile(cc2::Tile::Flag1000),
        cc2::Tile(cc2::Tile::TrapButton),
        cc2::Tile(cc2::Tile::Trap),
        cc2::Tile(cc2::Tile::Force_Rand),
        cc2::Tile(cc2::Tile::Force_S),
        cc2::Tile(cc2::Tile::MagnoShoes),
        cc2::Tile(cc2::Tile::Teleport_Green),
        cc2::Tile(cc2::Tile::Key_Green),
        cc2::Tile(cc2::Tile::Door_Green),
        cc2::Tile(cc2::Tile::Flag2x),
        cc2::Tile(cc2::Tile::CloneButton),
        cc2::Tile(cc2::Tile::Cloner, cc2::TileModifier::CloneSouth),
        cc2::Tile(cc2::Tile::Turtle),
        cc2::Tile(cc2::Tile::Water),
        cc2::Tile(cc2::Tile::Flippers),
        cc2::Tile(cc2::Tile::Teleport_Yellow),
        cc2::Tile(cc2::Tile::Key_Yellow),
        cc2::Tile(cc2::Tile::Door_Yellow),
        cc2::Tile(cc2::Tile::BowlingBall),
        cc2::Tile(cc2::Tile::FlameJetButton),
        cc2::Tile(cc2::Tile::FlameJet_Off),
        cc2::Tile(cc2::Tile::FlameJet_On),
        cc2::Tile(cc2::Tile::Fire),
        cc2::Tile(cc2::Tile::FireShoes),
        cc2::Tile(cc2::Tile::Teleport_Red),
        cc2::Tile(cc2::Tile::Key_Red),
        cc2::Tile(cc2::Tile::Door_Red),
        cc2::Tile(cc2::Tile::Helmet),
        cc2::Tile(cc2::Tile::ToggleButton),
        cc2::Tile(cc2::Tile::ToggleFloor),
        cc2::Tile(cc2::Tile::ToggleWall),
        cc2::Tile(cc2::Tile::GreenBomb),
        cc2::Tile(cc2::Tile::GreenChip),
        cc2::Tile(cc2::Tile::TrainTracks, cc2::TileModifier::Track_NS),
        cc2::Tile(cc2::Tile::TrainTracks, cc2::TileModifier::Track_SE),
        cc2::Tile(cc2::Tile::TrainTracks, cc2::TileModifier::TrackSwitch),
        cc2::Tile(cc2::Tile::RRSign),
        cc2::Tile(cc2::Tile::LogicButton),
        cc2::Tile(cc2::Tile::LSwitchFloor),
        cc2::Tile(cc2::Tile::LSwitchWall),
        cc2::Tile(cc2::Tile::RedBomb),
        cc2::Tile(cc2::Tile::Slime),
        cc2::Tile(cc2::Tile::RevolvDoor_NW),
        cc2::Tile(cc2::Tile::Switch_Off),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::CounterGate_0),
        cc2::Tile(cc2::Tile::Eye),
        cc2::Tile(cc2::Tile::RevLogicButton),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::AndGate_N),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::OrGate_N),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::NandGate_N),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::XorGate_N),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::LatchGateCW_N),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::LatchGateCCW_N),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::Inverter_N),
        cc2::Tile(cc2::Tile::Lightning),
        cc2::Tile(cc2::Tile::Clue),
        cc2::Tile(cc2::Tile::Floor, cc2::TileModifier::WireSouth | cc2::TileModifier::WireTunnelSouth),
        cc2::Tile(cc2::Tile::Floor, cc2::TileModifier::WireSouth | cc2::TileModifier::WireTunnelSouth
                                    | cc2::TileModifier::WireEast | cc2::TileModifier::WireTunnelEast),
        cc2::Tile(cc2::Tile::Floor, cc2::TileModifier::WireSouth | cc2::TileModifier::WireTunnelSouth
                                    | cc2::TileModifier::WireEast | cc2::TileModifier::WireTunnelEast
                                    | cc2::TileModifier::WireWest | cc2::TileModifier::WireTunnelWest),
        cc2::Tile(cc2::Tile::Floor, cc2::TileModifier::WireMask | cc2::TileModifier::WireTunnelMask),
        cc2::Tile(cc2::Tile::Invalid),
        cc2::Tile(cc2::Tile::Invalid),
        cc2::Tile::dirBlockTile(0),
        cc2::Tile(cc2::Tile::Trap_Open),
    };

    m_glyphs.reserve(cc2::TileModifier::GlyphMAX - cc2::TileModifier::GlyphMIN);
    for (int i = cc2::TileModifier::GlyphMIN; i < cc2::TileModifier::GlyphMAX; ++i)
        m_glyphs.emplace_back(cc2::Tile::AsciiGlyph, i);
}

void BigTileWidget::setTileset(CC2ETileset* tileset)
{
    m_tileset = tileset;
    resize(sizeHint());
    update();
}

void BigTileWidget::setView(ViewType type)
{
    m_view = type;
    update();
}

void BigTileWidget::rotateLeft()
{
    for (cc2::Tile& tile : tileList())
        tile.rotateLeft();
    update();
}

void BigTileWidget::rotateRight()
{
    for (cc2::Tile& tile : tileList())
        tile.rotateRight();
    update();
}

void BigTileWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    if (!m_tileset)
        return;

    QPainter painter(this);
    painter.scale(m_tileset->uiScale(), m_tileset->uiScale());
    int x = 0, y = 0;
    for (const cc2::Tile& tile : tileList()) {
        if (tile.type() != cc2::Tile::Invalid)
            m_tileset->draw(painter, x, y, &tile, false);
        if (++x >= 9) {
            ++y;
            x = 0;
        }
    }
}

void BigTileWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_tileset || event->x() >= (m_tileset->uiSize() * 9))
        return;

    const auto& tiles = tileList();
    const int tileSize = m_tileset->uiSize();
    size_t which = ((event->y() / tileSize) * 9) + (event->x() / tileSize);
    if (which >= tiles.size() || tiles[which].type() == cc2::Tile::Invalid)
        return;
    if (event->button() == Qt::LeftButton)
        emit tileSelectedLeft(tiles[which]);
    else if (event->button() == Qt::RightButton)
        emit tileSelectedRight(tiles[which]);
}

void BigTileWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    if (!m_tileset || event->x() >= (m_tileset->uiSize() * 9)) {
        setToolTip(QString());
        return;
    }

    const auto& tiles = tileList();
    const int tileSize = m_tileset->uiSize();
    size_t which = ((event->y() / tileSize) * 9) + (event->x() / tileSize);
    if (which >= tiles.size() || tiles[which].type() == cc2::Tile::Invalid) {
        setToolTip(QString());
        return;
    }
    setToolTip(CC2ETileset::getName(&tiles[which]));
}
