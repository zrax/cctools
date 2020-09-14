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
#include "libcc1/Tileset.h"

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
    : QDialog(parent), m_tileset()
{
    setWindowTitle(tr("Tile Inspector"));
    setWindowIcon(QIcon(":/res/draw-inspect.png"));

    m_upperImage = new QLabel(this);
    m_lowerImage = new QLabel(this);

    m_upperType = new QComboBox(this);
    for (int i = 0; i < ccl::NUM_TILE_TYPES; ++i)
        m_upperType->addItem(CCETileset::TileName((tile_t)i), i);
    m_upperType->insertSeparator(ccl::NUM_TILE_TYPES);
    m_upperType->addItem(tr("Other:"));
    auto upperTypeLabel = new QLabel(tr("&Upper Tile:"), this);
    upperTypeLabel->setBuddy(m_upperType);
    m_upperTypeId = new QSpinBox(this);
    m_upperTypeId->setRange(0, 254);     // Don't use 255 -- it's the RLE marker!
    m_upperTypeId->setEnabled(false);

    connect(m_upperType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        QVariant data = m_upperType->itemData(index);
        if (data.isNull()) {
            m_upperTypeId->setEnabled(true);
        } else {
            m_upperTypeId->setValue(data.toInt());
            m_upperTypeId->setEnabled(false);
        }
    });
    connect(m_upperTypeId, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &TileInspector::setUpperType);

    m_lowerType = new QComboBox(this);
    for (int i = 0; i < ccl::NUM_TILE_TYPES; ++i)
        m_lowerType->addItem(CCETileset::TileName((tile_t)i), i);
    m_lowerType->insertSeparator(ccl::NUM_TILE_TYPES);
    m_lowerType->addItem(tr("Other:"));
    auto lowerTypeLabel = new QLabel(tr("&Lower Tile:"), this);
    lowerTypeLabel->setBuddy(m_lowerType);
    m_lowerTypeId = new QSpinBox(this);
    m_lowerTypeId->setRange(0, 254);     // Don't use 255 -- it's the RLE marker!
    m_lowerTypeId->setEnabled(false);

    connect(m_lowerType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        QVariant data = m_lowerType->itemData(index);
        if (data.isNull()) {
            m_lowerTypeId->setEnabled(true);
        } else {
            m_lowerTypeId->setValue(data.toInt());
            m_lowerTypeId->setEnabled(false);
        }
    });
    connect(m_lowerTypeId, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &TileInspector::setLowerType);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel,
                                        Qt::Horizontal, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &TileInspector::tryAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(8);
    layout->setHorizontalSpacing(8);
    int row = 0;
    layout->addWidget(m_upperImage, row, 0);
    layout->addWidget(upperTypeLabel, row, 1);
    layout->addWidget(m_upperType, row, 2);
    layout->addWidget(m_upperTypeId, row, 3);
    layout->addWidget(m_lowerImage, ++row, 0);
    layout->addWidget(lowerTypeLabel, row, 1);
    layout->addWidget(m_lowerType, row, 2);
    layout->addWidget(m_lowerTypeId, row, 3);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding),
                    ++row, 1, 1, 3);
    layout->addWidget(buttons, ++row, 0, 1, 4);
}

void TileInspector::setTileset(CCETileset* tileset)
{
    m_tileset = tileset;
    m_upperImage->setPixmap(m_tileset->getPixmap(m_upperTypeId->value()));
    m_lowerImage->setPixmap(m_tileset->getPixmap(m_lowerTypeId->value()));
}

void TileInspector::setTile(tile_t upper, tile_t lower)
{
    Q_ASSERT(m_tileset);
    //setUpperType(upper);
    //setLowerType(lower);

    if (upper < ccl::NUM_TILE_TYPES)
        m_upperType->setCurrentIndex((int)upper);
    else
        m_upperType->setCurrentIndex(m_upperType->count() - 1);
    m_upperTypeId->setValue((int)upper);

    if (lower < ccl::NUM_TILE_TYPES)
        m_lowerType->setCurrentIndex((int)lower);
    else
        m_lowerType->setCurrentIndex(m_lowerType->count() - 1);
    m_lowerTypeId->setValue((int)lower);
}

tile_t TileInspector::upper() const
{
    return static_cast<tile_t>(m_upperTypeId->value());
}

tile_t TileInspector::lower() const
{
    return static_cast<tile_t>(m_lowerTypeId->value());
}

void TileInspector::tryAccept()
{
    const int rangeTile = qMax(m_upperTypeId->value(), m_lowerTypeId->value());
    if (rangeTile >= ccl::NUM_TILE_TYPES) {
        auto response = QMessageBox::question(this, tr("Tile out of range"),
                            tr("Tile type (%1) is outside the range of known tile types. "
                               "This may cause random and unexpected behavior, including "
                               "crashes and possible data corruption, when loading the "
                               "level in Chip's Challenge.").arg(rangeTile),
                            QMessageBox::Ok | QMessageBox::Cancel);
        if (response == QMessageBox::Cancel)
            return;
    }

    accept();
}

void TileInspector::setUpperType(int type)
{
    if (m_tileset)
        m_upperImage->setPixmap(m_tileset->getPixmap((tile_t)type));
    m_upperImage->setToolTip(CCETileset::TileName((tile_t)type));
}

void TileInspector::setLowerType(int type)
{
    if (m_tileset)
        m_lowerImage->setPixmap(m_tileset->getPixmap((tile_t)type));
    m_lowerImage->setToolTip(CCETileset::TileName((tile_t)type));
}
