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
    : HackPage(parent)
{
    auto layout = new QGridLayout(this);

    m_cbTitle = new QCheckBox(tr("Game Title:"), this);
    m_title = new QLineEdit(this);
    m_title->setEnabled(false);
    m_title->setMaxLength(16);
    m_defTitle = new QLineEdit(this);
    m_defTitle->setEnabled(false);

    m_cbIniFile = new QCheckBox(tr("INI Filename:"), this);
    m_iniFile = new QLineEdit(this);
    m_iniFile->setEnabled(false);
    m_iniFile->setMaxLength(11);
    m_defIniFile = new QLineEdit(this);
    m_defIniFile->setEnabled(false);

    m_cbIniEntry = new QCheckBox(tr("INI Entry:"), this);
    m_iniEntry = new QLineEdit(this);
    m_iniEntry->setEnabled(false);
    m_iniEntry->setMaxLength(17);
    m_defIniEntry = new QLineEdit(this);
    m_defIniEntry->setEnabled(false);

    m_cbDatFile = new QCheckBox(tr("Data File:"), this);
    m_datFile = new QLineEdit(this);
    m_datFile->setEnabled(false);
    m_datFile->setMaxLength(9);
    m_defDatFile = new QLineEdit(this);
    m_defDatFile->setEnabled(false);

    m_alwaysFirstTry = new QCheckBox(tr("Always grant \"First Try\" bonus"), this);
    m_ccPatch = new QCheckBox(tr("CCPatch (fixes crash while walking over squares with two masked tiles)"), this);
    m_pgChips = new QCheckBox(tr("PGChips (adds Ice Block support to the game)"), this);

    m_cbFakeLastLevel = new QCheckBox(tr("\"Fake\" Last Level:"), this);
    m_fakeLastLevel = new QSpinBox(this);
    m_fakeLastLevel->setEnabled(false);
    m_fakeLastLevel->setRange(0, 65535);
    m_defFakeLastLevel = new QLineEdit(this);
    m_defFakeLastLevel->setEnabled(false);

    m_cbRealLastLevel = new QCheckBox(tr("Actual Last Level:"), this);
    m_realLastLevel = new QSpinBox(this);
    m_realLastLevel->setEnabled(false);
    m_realLastLevel->setRange(0, 65535);
    m_defRealLastLevel = new QLineEdit(this);
    m_defRealLastLevel->setEnabled(false);

    layout->addWidget(new QLabel(tr("Override"), this), 0, 1);
    layout->addWidget(new QLabel(tr("Default"), this), 0, 2);
    layout->addWidget(m_cbTitle, 1, 0);
    layout->addWidget(m_title, 1, 1);
    layout->addWidget(m_defTitle, 1, 2);
    layout->addWidget(m_cbIniFile, 2, 0);
    layout->addWidget(m_iniFile, 2, 1);
    layout->addWidget(m_defIniFile, 2, 2);
    layout->addWidget(m_cbIniEntry, 3, 0);
    layout->addWidget(m_iniEntry, 3, 1);
    layout->addWidget(m_defIniEntry, 3, 2);
    layout->addWidget(m_cbDatFile, 4, 0);
    layout->addWidget(m_datFile, 4, 1);
    layout->addWidget(m_defDatFile, 4, 2);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), 5, 0, 1, 3);
    layout->addWidget(new QLabel(tr("Code Patches:"), this), 6, 0, 1, 3);
    layout->addWidget(m_alwaysFirstTry, 7, 0, 1, 3);
    layout->addWidget(m_ccPatch, 8, 0, 1, 3);
    layout->addWidget(m_pgChips, 9, 0, 1, 3);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), 10, 0, 1, 3);
    layout->addWidget(m_cbFakeLastLevel, 11, 0);
    layout->addWidget(m_fakeLastLevel, 11, 1);
    layout->addWidget(m_defFakeLastLevel, 11, 2);
    layout->addWidget(m_cbRealLastLevel, 12, 0);
    layout->addWidget(m_realLastLevel, 12, 1);
    layout->addWidget(m_defRealLastLevel, 12, 2);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 13, 0, 1, 3);

    connect(m_cbTitle, &QCheckBox::toggled, m_title, &QWidget::setEnabled);
    connect(m_cbIniFile, &QCheckBox::toggled, m_iniFile, &QWidget::setEnabled);
    connect(m_cbIniEntry, &QCheckBox::toggled, m_iniEntry, &QWidget::setEnabled);
    connect(m_cbDatFile, &QCheckBox::toggled, m_datFile, &QWidget::setEnabled);
    connect(m_cbFakeLastLevel, &QCheckBox::toggled, m_fakeLastLevel, &QWidget::setEnabled);
    connect(m_cbRealLastLevel, &QCheckBox::toggled, m_realLastLevel, &QWidget::setEnabled);
}

void CCHack::PageGeneral::setValues(HackSettings* settings)
{
    m_cbTitle->setChecked(settings->have_title());
    m_title->setText(QString::fromLatin1(settings->get_title().c_str()));
    m_cbIniFile->setChecked(settings->have_iniFile());
    m_iniFile->setText(QString::fromLatin1(settings->get_iniFile().c_str()));
    m_cbIniEntry->setChecked(settings->have_iniEntry());
    m_iniEntry->setText(QString::fromLatin1(settings->get_iniEntry().c_str()));
    m_cbDatFile->setChecked(settings->have_datFile());
    m_datFile->setText(QString::fromLatin1(settings->get_datFile().c_str()));
    m_alwaysFirstTry->setChecked(settings->get_alwaysFirstTry());
    m_ccPatch->setChecked(settings->get_ccPatch());
    m_pgChips->setChecked(settings->get_pgChips());
    m_cbFakeLastLevel->setChecked(settings->have_fakeLastLevel());
    m_fakeLastLevel->setValue(settings->get_fakeLastLevel());
    m_cbRealLastLevel->setChecked(settings->have_realLastLevel());
    m_realLastLevel->setValue(settings->get_realLastLevel());
}

void CCHack::PageGeneral::setDefaults(HackSettings* settings)
{
    m_defTitle->setText(QString::fromLatin1(settings->get_title().c_str()));
    m_defIniFile->setText(QString::fromLatin1(settings->get_iniFile().c_str()));
    m_defIniEntry->setText(QString::fromLatin1(settings->get_iniEntry().c_str()));
    m_defDatFile->setText(QString::fromLatin1(settings->get_datFile().c_str()));
    m_alwaysFirstTry->setChecked(settings->get_alwaysFirstTry());
    m_ccPatch->setChecked(settings->get_ccPatch());
    m_pgChips->setChecked(settings->get_pgChips());
    m_defFakeLastLevel->setText(QString("%1").arg(settings->get_fakeLastLevel()));
    m_defRealLastLevel->setText(QString("%1").arg(settings->get_realLastLevel()));
}
