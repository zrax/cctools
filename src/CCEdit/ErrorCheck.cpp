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

#include "ErrorCheck.h"

#include <QPushButton>
#include <QGridLayout>
#include <QLabel>

enum CheckMode {
    CheckMsccStrict, CheckMscc, CheckTWorldMS, CheckTWorldLynx,
    CheckLynxPedantic,
};

ErrorCheckDialog::ErrorCheckDialog(QWidget* parent)
                : QDialog(parent), m_levelset(0), m_dacFile(0)
{
    m_checkMode = new QComboBox(this);
    m_checkMode->addItems(QStringList() << tr("MSCC (Unmodified)")
                                        << tr("MSCC (CCPlay)")
                                        << tr("Tile World (MSCC rules)")
                                        << tr("Tile World (Lynx rules)")
                                        << tr("Tile World (Lynx pedantic)"));
    m_checkMode->setCurrentIndex(CheckMscc);
    m_checkTarget = new QComboBox(this);

    m_errors = new QTreeWidget(this);
    m_errors->setHeaderHidden(true);
    m_errors->setRootIsDecorated(false);

    QWidget* checkAlign = new QWidget(this);
    QPushButton* btnClose = new QPushButton(tr("Clo&se"), checkAlign);
    QPushButton* btnCheck = new QPushButton(tr("&Check"), checkAlign);
    QGridLayout* layCheck = new QGridLayout(checkAlign);
    layCheck->setContentsMargins(0, 0, 0, 0);
    layCheck->setHorizontalSpacing(8);
    layCheck->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum), 0, 0);
    layCheck->addWidget(btnClose, 0, 1);
    layCheck->addWidget(btnCheck, 0, 2);
    btnCheck->setDefault(true);

    QLabel* checkLabel = new QLabel(tr("Check &against:"), this);
    checkLabel->setBuddy(m_checkMode);
    QLabel* targetLabel = new QLabel(tr("&Look at:"), this);
    targetLabel->setBuddy(m_checkTarget);

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(8);
    layout->addWidget(checkLabel, 0, 0);
    layout->addWidget(m_checkMode, 0, 1);
    layout->addWidget(targetLabel, 1, 0);
    layout->addWidget(m_checkTarget, 1, 1);
    layout->addWidget(m_errors, 2, 0, 1, 2);
    layout->addWidget(checkAlign, 3, 0, 1, 2);

    connect(btnClose, SIGNAL(clicked()), SLOT(accept()));
    connect(btnCheck, SIGNAL(clicked()), SLOT(onCheck()));
}

void ErrorCheckDialog::setLevelsetInfo(ccl::Levelset* levelset, ccl::DacFile* dac)
{
    m_levelset = levelset;
    m_dacFile = dac;

    m_checkTarget->clear();
    m_checkTarget->addItem(tr("(Entire levelset)"));
    for (int i=0; i<m_levelset->levelCount(); ++i) {
        m_checkTarget->addItem(QString("%1 - %2").arg(i + 1)
                               .arg(QString::fromAscii(m_levelset->level(i)->name().c_str())));
    }
}

void ErrorCheckDialog::reportError(QString section, QString text)
{
    QTreeWidgetItem* sectionItem;
    if (m_errors->topLevelItemCount() > 0 &&
        m_errors->topLevelItem(m_errors->topLevelItemCount()-1)->text(0) == section) {
        sectionItem = m_errors->topLevelItem(m_errors->topLevelItemCount()-1);
    } else {
        sectionItem = new QTreeWidgetItem(m_errors);
        sectionItem->setText(0, section);
        QFont font = sectionItem->font(0);
        font.setBold(true);
        sectionItem->setFont(0, font);
    }

    QTreeWidgetItem* item = new QTreeWidgetItem(sectionItem);
    item->setText(0, text);
}

void ErrorCheckDialog::onCheck()
{
    m_errors->clear();

    if (m_checkTarget->currentIndex() == 0) {
        if (m_checkMode->currentIndex() == CheckMsccStrict && m_levelset->levelCount() != 149)
            reportError("Levelset",
                        "[MSCC Compatibility]\n"
                        "Levelset does not contain exactly 149 levels");
        if (m_checkMode->currentIndex() == CheckLynxPedantic && m_levelset->levelCount() != 148)
            reportError("Levelset",
                        "[Lynx Compatibility]\n"
                        "Levelset does not contain exactly 148 levels");
        if ((m_checkMode->currentIndex() == CheckMsccStrict || m_checkMode->currentIndex() == CheckMscc)
            && m_levelset->type() != ccl::Levelset::TypeMS && m_levelset->type() != ccl::Levelset::TypePG)
            reportError("Levelset",
                        "[MSCC Compatibility]\n"
                        "Levelset is configured for Lynx compatibility");

        for (int i=0; i<m_levelset->levelCount(); ++i)
            checkLevel(i);
    } else {
        checkLevel(m_checkTarget->currentIndex() - 1);
    }

    if (m_errors->topLevelItemCount() == 0) {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_errors);
        item->setText(0, "No errors found!");
        QFont font = item->font(0);
        font.setItalic(true);
        item->setFont(0, font);
    }
    m_errors->expandAll();
}

void ErrorCheckDialog::checkLevel(int level)
{
    ccl::LevelData* levelData = m_levelset->level(level);

    int chips = 0, players = 0;
    bool haveExit = false;
    for (int y=0; y<32; ++y) {
        for (int x=0; x<32; ++x) {
            tile_t fg = levelData->map().getFG(x, y);
            tile_t bg = levelData->map().getBG(x, y);
            if (fg == ccl::TileExit || bg == ccl::TileExit)
                haveExit = true;
            if (fg == ccl::TileChip)
                ++chips;
            if (bg == ccl::TileChip)
                ++chips;
            if ((fg & 0xFC) == ccl::TilePlayer_N)
                ++players;
            if ((bg & 0xFC) == ccl::TilePlayer_N)
                reportError(QString("%1 - %2").arg(level)
                            .arg(QString::fromAscii(levelData->name().c_str())),
                            QString("[Invalid Tile Combo]\n"
                                    "Buried player tile at (%1, %2)")
                            .arg(x).arg(y));
            if (m_levelset->type() != ccl::Levelset::TypePG &&
                m_levelset->type() != ccl::Levelset::TypeLynxPG &&
                (fg == ccl::TileIceBlock || bg == ccl::TileIceBlock))
                reportError(QString("%1 - %2").arg(level)
                            .arg(QString::fromAscii(levelData->name().c_str())),
                            QString("[Invalid Tile]\n"
                                    "Use of ice block at (%1, %2) in non-PGChips levelset")
                            .arg(x).arg(y));
            if (fg == ccl::Tile_UNUSED_20 || (fg >= ccl::TilePlayerSplash &&
                fg <= ccl::TilePlayerSwim_E && fg != ccl::TileIceBlock) ||
                bg == ccl::Tile_UNUSED_20 || (bg >= ccl::TilePlayerSplash &&
                bg <= ccl::TilePlayerSwim_E && bg != ccl::TileIceBlock))
                reportError(QString("%1 - %2").arg(level)
                            .arg(QString::fromAscii(levelData->name().c_str())),
                            QString("[Invalid Tile]\n"
                                    "Use of reserved tile at (%1, %2)")
                            .arg(x).arg(y));
        }
    }

    if (!haveExit)
        reportError(QString("%1 - %2").arg(level)
                    .arg(QString::fromAscii(levelData->name().c_str())),
                    "[Unsolvable]\n"
                    "No exit tile is present in the level");
    if (chips < levelData->chips())
        reportError(QString("%1 - %2").arg(level)
                    .arg(QString::fromAscii(levelData->name().c_str())),
                    QString("[Possibly Unsolvable]\n"
                            "Not enough chips to meet goal (need %1 more)")
                    .arg(levelData->chips() - chips));
    if (players == 0)
        reportError(QString("%1 - %2").arg(level)
                    .arg(QString::fromAscii(levelData->name().c_str())),
                    "[Design Error]\n"
                    "No player start tile is present in the level");
    if (players > 1)
        reportError(QString("%1 - %2").arg(level + 1)
                    .arg(QString::fromAscii(levelData->name().c_str())),
                    "[Design Error]\n"
                    "Multiple player start tiles are present in the level");
}
