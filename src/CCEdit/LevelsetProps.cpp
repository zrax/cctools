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

#include "LevelsetProps.h"

#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>

static int mapLevelsetType(int type)
{
    switch (type) {
    case ccl::Levelset::TypeLynx:
        return 1;
    default:
        return 0;
    }
}

static int unmapLevelsetType(int idx)
{
    switch (idx) {
    case 1:
        return ccl::Levelset::TypeLynx;
    default:
        return ccl::Levelset::TypeMS;
    }
}

LevelsetProps::LevelsetProps(QWidget* parent)
             : QDialog(parent), m_levelset(0), m_dacFile(0)
{
    setWindowTitle(tr("Levelset Properties"));

    m_levelsetType = new QComboBox(this);
    m_levelsetType->addItems(QStringList() << "MSCC (recommended)" << "TWorld Lynx");
    m_dacGroup = new QGroupBox(tr("Use DAC file"), this);
    m_dacGroup->setCheckable(true);
    m_dacGroup->setChecked(false);
    m_dacFilename = new QLineEdit(m_dacGroup);
    m_dacRuleset = new QComboBox(m_dacGroup);
    m_dacRuleset->addItems(QStringList() << "MSCC rules" << "Lynx rules");
    m_lastLevel = new QSpinBox(m_dacGroup);
    m_lastLevel->setValue(0);
    m_usePasswords = new QCheckBox(tr("Use &Passwords"), m_dacGroup);
    m_usePasswords->setChecked(false);
    QDialogButtonBox* buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

    QLabel* lblLevelsetType = new QLabel(tr("Internal &Format:"), m_dacGroup);
    lblLevelsetType->setBuddy(m_levelsetType);
    QLabel* lblDacFilename = new QLabel(tr("&Levelset Filename:"), m_dacGroup);
    lblDacFilename->setBuddy(m_dacFilename);
    QLabel* lblDacRuleset = new QLabel(tr("TWorld &Ruleset:"), m_dacGroup);
    lblDacRuleset->setBuddy(m_dacRuleset);
    QLabel* lblLastLevel = new QLabel(tr("Last Normal Le&vel:"), m_dacGroup);
    lblLastLevel->setBuddy(m_lastLevel);

    QGridLayout* dacLayout = new QGridLayout(m_dacGroup);
    dacLayout->setContentsMargins(4, 4, 4, 4);
    dacLayout->setVerticalSpacing(4);
    dacLayout->addWidget(lblDacFilename, 0, 0);
    dacLayout->addWidget(m_dacFilename, 0, 1);
    dacLayout->addWidget(lblDacRuleset, 1, 0);
    dacLayout->addWidget(m_dacRuleset, 1, 1);
    dacLayout->addWidget(lblLastLevel, 2, 0);
    dacLayout->addWidget(m_lastLevel, 2, 1);
    dacLayout->addWidget(m_usePasswords, 3, 1);

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(8);
    layout->addWidget(lblLevelsetType, 0, 0);
    layout->addWidget(m_levelsetType, 0, 1);
    layout->addWidget(m_dacGroup, 1, 0, 1, 2);
    layout->addWidget(buttons, 2, 0, 1, 2);

    connect(buttons, SIGNAL(accepted()), SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), SLOT(reject()));
}

void LevelsetProps::setLevelset(ccl::Levelset* levelset)
{
    m_levelset = levelset;
    m_levelsetType->setCurrentIndex(mapLevelsetType(levelset->type()));
    m_lastLevel->setRange(0, levelset->levelCount());
}

void LevelsetProps::setDacFile(ccl::DacFile* dac)
{
    m_dacGroup->setChecked(dac != 0);
    if (dac != 0) {
        m_dacFilename->setText(dac->m_filename.c_str());
        m_dacRuleset->setCurrentIndex(mapLevelsetType(dac->m_ruleset));
        m_lastLevel->setValue(dac->m_lastLevel);
        m_usePasswords->setChecked(dac->m_usePasswords);
    }
}

int LevelsetProps::levelsetType() const
{
    return unmapLevelsetType(m_levelsetType->currentIndex());
}

int LevelsetProps::dacRuleset() const
{
    return unmapLevelsetType(m_dacRuleset->currentIndex());
}
