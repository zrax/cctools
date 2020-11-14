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
#include <QSettings>

#include "CommonWidgets/CCTools.h"

enum CheckMode {
    CheckMsccStrict, CheckMscc, CheckTWorldLynx, CheckLynxPedantic,
};

ErrorCheckDialog::ErrorCheckDialog(QWidget* parent)
                : QDialog(parent), m_levelset(0), m_dacFile(0)
{
    setWindowTitle(tr("Check for Errors"));
    QSettings settings;

    m_checkMode = new QComboBox(this);
    m_checkMode->addItems(QStringList() << tr("MSCC (Unmodified)")
                                        << tr("MSCC (CCPlay / Tile World)")
                                        << tr("Lynx (Tile World)")
                                        << tr("Lynx (Pedantic)"));
    m_checkMode->setCurrentIndex(settings.value(QStringLiteral("LevelsetCheckMode"),
                                                (int)CheckMscc).toInt());
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

    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCheck, &QPushButton::clicked, this, &ErrorCheckDialog::onCheck);
}

ErrorCheckDialog::~ErrorCheckDialog()
{
    QSettings settings;
    settings.setValue(QStringLiteral("LevelsetCheckMode"), m_checkMode->currentIndex());
}

void ErrorCheckDialog::setLevelsetInfo(ccl::Levelset* levelset, ccl::DacFile* dac)
{
    m_levelset = levelset;
    m_dacFile = dac;

    m_checkTarget->clear();
    m_checkTarget->addItem(tr("(Entire levelset)"));
    for (int i=0; i<m_levelset->levelCount(); ++i) {
        m_checkTarget->addItem(QStringLiteral("%1 - %2").arg(i + 1)
                               .arg(ccl::fromLatin1(m_levelset->level(i)->name())));
    }
}

void ErrorCheckDialog::reportError(int level, const QString& text)
{
    QString section = (level < 0) ? tr("Levelset")
                    : QStringLiteral("%1 - %2").arg(level + 1)
                      .arg(ccl::fromLatin1(m_levelset->level(level)->name()));

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
            reportError(-1, tr("[MSCC Compatibility]\n"
                               "Levelset does not contain exactly 149 levels"));
        if (m_checkMode->currentIndex() == CheckLynxPedantic && m_levelset->levelCount() != 149)
            reportError(-1, tr("[Lynx Compatibility]\n"
                               "Levelset does not contain exactly 149 levels"));
        if ((m_checkMode->currentIndex() == CheckMsccStrict || m_checkMode->currentIndex() == CheckMscc)
            && m_levelset->type() != ccl::Levelset::TypeMS && m_levelset->type() != ccl::Levelset::TypePG)
            reportError(-1, tr("[MSCC Compatibility]\n"
                               "Levelset is configured for Lynx compatibility"));

        for (int i=0; i<m_levelset->levelCount(); ++i)
            checkLevel(i);
    } else {
        checkLevel(m_checkTarget->currentIndex() - 1);
    }

    if (m_errors->topLevelItemCount() == 0) {
        auto item = new QTreeWidgetItem(m_errors);
        item->setText(0, tr("No errors found!"));
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
                reportError(level, tr("[Invalid Tile Combo]\n"
                                      "Buried player tile at (%1, %2)")
                                   .arg(x).arg(y));
            if (m_levelset->type() != ccl::Levelset::TypePG &&
                m_levelset->type() != ccl::Levelset::TypeLynxPG &&
                (fg == ccl::TileIceBlock || bg == ccl::TileIceBlock))
                reportError(level, tr("[Invalid Tile]\n"
                                      "Use of ice block at (%1, %2) in non-PGChips levelset")
                                   .arg(x).arg(y));
            if (fg == ccl::Tile_UNUSED_20 || (fg >= ccl::TilePlayerSplash &&
                fg <= ccl::TilePlayerSwim_E && fg != ccl::TileIceBlock) ||
                bg == ccl::Tile_UNUSED_20 || (bg >= ccl::TilePlayerSplash &&
                bg <= ccl::TilePlayerSwim_E && bg != ccl::TileIceBlock))
                reportError(level, tr("[Invalid Tile]\n"
                                      "Use of reserved tile at (%1, %2)")
                                   .arg(x).arg(y));
        }
    }

    if (!haveExit)
        reportError(level, tr("[Unsolvable]\n"
                              "No exit tile is present in the level"));
    if (chips < levelData->chips())
        reportError(level, tr("[Possibly Unsolvable]\n"
                              "Not enough chips to meet goal (need %1 more)")
                           .arg(levelData->chips() - chips));
    if (players == 0)
        reportError(level, tr("[Design Warning]\n"
                              "No player start tile is present in the level"));
    if (players > 1)
        reportError(level, tr("[Design Warning]\n"
                              "Multiple player start tiles are present in the level"));

    std::list<ccl::Trap>::iterator trap_iter;
    for (trap_iter = levelData->traps().begin(); trap_iter != levelData->traps().end(); ++trap_iter) {
        if (trap_iter->button.X < 0 || trap_iter->button.X > 31 ||
            trap_iter->button.Y < 0 || trap_iter->button.Y > 31)
            reportError(level, tr("[Invalid Trap]\n"
                                  "Trap button is outside of level region (%1, %2)")
                               .arg(trap_iter->button.X).arg(trap_iter->button.Y));
        if (trap_iter->trap.X < 0 || trap_iter->trap.X > 31 ||
            trap_iter->trap.Y < 0 || trap_iter->trap.Y > 31)
            reportError(level, tr("[Invalid Trap]\n"
                                  "Trap target is outside of level region (%1, %2)")
                               .arg(trap_iter->trap.X).arg(trap_iter->trap.Y));
        if (levelData->map().getFG(trap_iter->button.X, trap_iter->button.Y) != ccl::TileTrapButton &&
            levelData->map().getBG(trap_iter->button.X, trap_iter->button.Y) != ccl::TileTrapButton)
            reportError(level, tr("[Invalid Trap]\n"
                                  "Trap button points to invalid tile at (%1, %2)")
                               .arg(trap_iter->button.X).arg(trap_iter->button.Y));
        if (levelData->map().getFG(trap_iter->trap.X, trap_iter->trap.Y) != ccl::TileTrap &&
            levelData->map().getBG(trap_iter->trap.X, trap_iter->trap.Y) != ccl::TileTrap)
            reportError(level, tr("[Invalid Trap]\n"
                                  "Trap target points to invalid tile at (%1, %2)")
                               .arg(trap_iter->trap.X).arg(trap_iter->trap.Y));
    }

    std::list<ccl::Clone>::iterator clone_iter;
    for (clone_iter = levelData->clones().begin(); clone_iter != levelData->clones().end(); ++clone_iter) {
        if (clone_iter->button.X < 0 || clone_iter->button.X > 31 ||
            clone_iter->button.Y < 0 || clone_iter->button.Y > 31)
            reportError(level, tr("[Invalid Cloner]\n"
                                  "Clone button is outside of level region (%1, %2)")
                               .arg(clone_iter->button.X).arg(clone_iter->button.Y));
        if (clone_iter->clone.X < 0 || clone_iter->clone.X > 31 ||
            clone_iter->clone.Y < 0 || clone_iter->clone.Y > 31)
            reportError(level, tr("[Invalid Cloner]\n"
                                  "Cloner target is outside of level region (%1, %2)")
                               .arg(clone_iter->clone.X).arg(clone_iter->clone.Y));
        if (levelData->map().getFG(clone_iter->button.X, clone_iter->button.Y) != ccl::TileCloneButton &&
            levelData->map().getBG(clone_iter->button.X, clone_iter->button.Y) != ccl::TileCloneButton)
            reportError(level, tr("[Invalid Cloner]\n"
                                  "Clone button points to invalid tile at (%1, %2)")
                               .arg(clone_iter->button.X).arg(clone_iter->button.Y));
        if (levelData->map().getFG(clone_iter->clone.X, clone_iter->clone.Y) != ccl::TileCloner &&
            levelData->map().getBG(clone_iter->clone.X, clone_iter->clone.Y) != ccl::TileCloner)
            reportError(level, tr("[Invalid Cloner]\n"
                                  "Cloner target points to invalid tile at (%1, %2)")
                               .arg(clone_iter->clone.X).arg(clone_iter->clone.Y));
    }

    std::list<ccl::Point>::iterator move_iter;
    for (move_iter = levelData->moveList().begin(); move_iter != levelData->moveList().end(); ++move_iter) {
        if (move_iter->X < 0 || move_iter->X > 31 ||
            move_iter->Y < 0 || move_iter->Y > 31)
            reportError(level, tr("[Invalid Mover]\n"
                                  "Monster position is outside of level region (%1, %2)")
                               .arg(move_iter->X).arg(move_iter->Y));
        if ((levelData->map().getFG(move_iter->X, move_iter->Y) < ccl::MONSTER_FIRST ||
            levelData->map().getFG(move_iter->X, move_iter->Y) > ccl::MONSTER_LAST) &&
            (levelData->map().getBG(move_iter->X, move_iter->Y) < ccl::MONSTER_FIRST ||
            levelData->map().getBG(move_iter->X, move_iter->Y) > ccl::MONSTER_LAST))
            reportError(level, tr("[Invalid Mover]\n"
                                  "Invalid monster tile at mover position (%1, %2)")
                               .arg(move_iter->X).arg(move_iter->Y));
    }

    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            if (m_checkMode->currentIndex() == CheckLynxPedantic) {
                if (levelData->map().getFG(x, y) == ccl::TileTrapButton
                    || levelData->map().getBG(x, y) == ccl::TileTrapButton) {
                    std::list<ccl::Point> targets = levelData->linkedTraps(x, y);
                    if (targets.size() == 0) {
                        reportError(level, tr("[Invalid Trap]\n"
                                    "Trap button at (%1, %2) has no connections")
                                    .arg(x).arg(y));
                    } else  if (targets.size() > 1) {
                        reportError(level, tr("[Invalid Trap]\n"
                                    "Trap buttons at (%1, %2) has multiple connections")
                                    .arg(x).arg(y));
                    } else if (targets.front() != levelData->map().findNext(x, y, ccl::TileTrap)) {
                        reportError(level, tr("[Invalid Trap]\n"
                                    "Trap connection for (%1, %2) violates the reading-order rule")
                                    .arg(x).arg(y));
                    }
                }
                if (levelData->map().getFG(x, y) == ccl::TileCloneButton
                    || levelData->map().getBG(x, y) == ccl::TileCloneButton) {
                    std::list<ccl::Point> targets = levelData->linkedCloners(x, y);
                    if (targets.size() == 0) {
                        reportError(level, tr("[Invalid Cloner]\n"
                                    "Clone button at (%1, %2) has no connections")
                                    .arg(x).arg(y));
                    } else  if (targets.size() > 1) {
                        reportError(level, tr("[Invalid Cloner]\n"
                                    "Clone button at (%1, %2) has multiple connections")
                                    .arg(x).arg(y));
                    } else  if (targets.front() != levelData->map().findNext(x, y, ccl::TileCloner)) {
                        reportError(level, tr("[Invalid Cloner]\n"
                                    "Cloner connections for (%1, %2) violates the reading-order rule")
                                    .arg(x).arg(y));
                    }
                }

                if (MONSTER_TILE(levelData->map().getFG(x, y))
                    && levelData->map().getBG(x, y) != ccl::TileCloner
                    && levelData->map().getBG(x, y) != ccl::TileTrap
                    && !levelData->checkMove(x, y)) {
                    reportError(level, tr("[Non-Moving Monster]\n"
                                "Monster at (%1, %2) does not move").arg(x).arg(y));
                }
            } /* CheckLynxPedantic */
        } /* y */
    } /* x */
}
