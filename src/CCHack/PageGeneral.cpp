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

#include "PageGeneral.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>

CCHack::PageGeneral::PageGeneral(QWidget* parent)
    : QObject(parent)
{
    QGridLayout* layout = (QGridLayout*)parent->layout();

    QCheckBox* cbTitle = new QCheckBox(tr("Game Title:"), parent);
    m_title = new QLineEdit(parent);
    m_title->setEnabled(false);
    m_defTitle = new QLineEdit("Chip's Challenge", parent);
    m_defTitle->setEnabled(false);

    QCheckBox* cbIniFile = new QCheckBox(tr("INI Filename:"), parent);
    m_iniFile = new QLineEdit(parent);
    m_iniFile->setEnabled(false);
    m_defIniFile = new QLineEdit("entpack.ini", parent);
    m_defIniFile->setEnabled(false);

    QCheckBox* cbIniEntry = new QCheckBox(tr("INI Entry:"), parent);
    m_iniEntry = new QLineEdit(parent);
    m_iniEntry->setEnabled(false);
    m_defIniEntry = new QLineEdit("Chip's Challenge", parent);
    m_defIniEntry->setEnabled(false);

    QCheckBox* cbDatFile = new QCheckBox(tr("Data File:"), parent);
    m_datFile = new QLineEdit(parent);
    m_datFile->setEnabled(false);
    m_defDatFile = new QLineEdit("CHIPS.DAT", parent);
    m_defDatFile->setEnabled(false);

    m_alwaysFirstTry = new QCheckBox(tr("Always grant \"First Try\" bonus"), parent);
    m_ccPatch = new QCheckBox(tr("CCPatch (fixes crash while walking over squares with two masked tiles)"), parent);
    m_pgChips = new QCheckBox(tr("PGChips (adds Ice Tile support to the game)"), parent);

    QCheckBox* cbFakeLastLevel = new QCheckBox(tr("\"Fake\" Last Level:"), parent);
    m_fakeLastLevel = new QLineEdit(parent);
    m_fakeLastLevel->setEnabled(false);
    m_defFakeLastLevel = new QLineEdit("144", parent);
    m_defFakeLastLevel->setEnabled(false);

    QCheckBox* cbRealLastLevel = new QCheckBox(tr("Actual Last Level:"), parent);
    m_realLastLevel = new QLineEdit(parent);
    m_realLastLevel->setEnabled(false);
    m_defRealLastLevel = new QLineEdit("149", parent);
    m_defRealLastLevel->setEnabled(false);

    layout->addWidget(new QLabel(tr("Override"), parent), 0, 1);
    layout->addWidget(new QLabel(tr("Default"), parent), 0, 2);
    layout->addWidget(cbTitle, 1, 0);
    layout->addWidget(m_title, 1, 1);
    layout->addWidget(m_defTitle, 1, 2);
    layout->addWidget(cbIniFile, 2, 0);
    layout->addWidget(m_iniFile, 2, 1);
    layout->addWidget(m_defIniFile, 2, 2);
    layout->addWidget(cbIniEntry, 3, 0);
    layout->addWidget(m_iniEntry, 3, 1);
    layout->addWidget(m_defIniEntry, 3, 2);
    layout->addWidget(cbDatFile, 4, 0);
    layout->addWidget(m_datFile, 4, 1);
    layout->addWidget(m_defDatFile, 4, 2);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), 5, 0, 1, 3);
    layout->addWidget(new QLabel(tr("Code Patches:"), parent), 6, 0, 1, 3);
    layout->addWidget(m_alwaysFirstTry, 7, 0, 1, 3);
    layout->addWidget(m_ccPatch, 8, 0, 1, 3);
    layout->addWidget(m_pgChips, 9, 0, 1, 3);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), 10, 0, 1, 3);
    layout->addWidget(cbFakeLastLevel, 11, 0);
    layout->addWidget(m_fakeLastLevel, 11, 1);
    layout->addWidget(m_defFakeLastLevel, 11, 2);
    layout->addWidget(cbRealLastLevel, 12, 0);
    layout->addWidget(m_realLastLevel, 12, 1);
    layout->addWidget(m_defRealLastLevel, 12, 2);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 13, 0, 1, 3);

    connect(cbTitle, SIGNAL(toggled(bool)), m_title, SLOT(setEnabled(bool)));
    connect(cbIniFile, SIGNAL(toggled(bool)), m_iniFile, SLOT(setEnabled(bool)));
    connect(cbIniEntry, SIGNAL(toggled(bool)), m_iniEntry, SLOT(setEnabled(bool)));
    connect(cbDatFile, SIGNAL(toggled(bool)), m_datFile, SLOT(setEnabled(bool)));
    connect(cbFakeLastLevel, SIGNAL(toggled(bool)), m_fakeLastLevel, SLOT(setEnabled(bool)));
    connect(cbRealLastLevel, SIGNAL(toggled(bool)), m_realLastLevel, SLOT(setEnabled(bool)));
}
