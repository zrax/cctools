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

#include "PageMisc.h"
#include "CommonWidgets/CCTools.h"

#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>

CCHack::PageMisc::PageMisc(QWidget* parent)
    : HackPage(parent)
{
    auto scroll = new QScrollArea(this);
    auto content = new QWidget(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    m_cbFireDeath = new QCheckBox(tr("Fire Death:"), this);
    m_fireDeath = new QLineEdit(this);
    m_fireDeath->setEnabled(false);
    m_fireDeath->setMaxLength(49);
    m_defFireDeath = new QLineEdit(this);
    m_defFireDeath->setEnabled(false);

    m_cbWaterDeath = new QCheckBox(tr("Water Death:"), this);
    m_waterDeath = new QLineEdit(this);
    m_waterDeath->setEnabled(false);
    m_waterDeath->setMaxLength(40);
    m_defWaterDeath = new QLineEdit(this);
    m_defWaterDeath->setEnabled(false);

    m_cbBombDeath = new QCheckBox(tr("Bomb Death:"), this);
    m_bombDeath = new QLineEdit(this);
    m_bombDeath->setEnabled(false);
    m_bombDeath->setMaxLength(29);
    m_defBombDeath = new QLineEdit(this);
    m_defBombDeath->setEnabled(false);

    m_cbBlockDeath = new QCheckBox(tr("Block Death:"), this);
    m_blockDeath = new QLineEdit(this);
    m_blockDeath->setEnabled(false);
    m_blockDeath->setMaxLength(35);
    m_defBlockDeath = new QLineEdit(this);
    m_defBlockDeath->setEnabled(false);

    m_cbCreatureDeath = new QCheckBox(tr("Creature Death:"), this);
    m_creatureDeath = new QLineEdit(this);
    m_creatureDeath->setEnabled(false);
    m_creatureDeath->setMaxLength(30);
    m_defCreatureDeath = new QLineEdit(this);
    m_defCreatureDeath->setEnabled(false);

    m_cbTimeLimit = new QCheckBox(tr("Out of Time:"), this);
    m_timeLimit = new QLineEdit(this);
    m_timeLimit->setEnabled(false);
    m_timeLimit->setMaxLength(19);
    m_defTimeLimit = new QLineEdit(this);
    m_defTimeLimit->setEnabled(false);

    m_cbNewGameConfirm = new QCheckBox(tr("New Game:"), this);
    m_newGameConfirm = new LLTextEdit(this);
    m_newGameConfirm->setEnabled(false);
    m_newGameConfirm->setMaxLength(157);
    m_defNewGameConfirm = new QPlainTextEdit(this);
    m_defNewGameConfirm->setEnabled(false);
    m_cbSkipLevel = new QCheckBox(tr("Skip Level:"), this);
    m_skipLevel = new LLTextEdit(this);
    m_skipLevel->setEnabled(false);
    m_skipLevel->setMaxLength(88);
    m_defSkipLevel = new QPlainTextEdit(this);
    m_defSkipLevel->setEnabled(false);
    m_cbNotEnoughTimers = new QCheckBox(tr("Out of Timers:"), this);
    m_notEnoughTimers = new QLineEdit(this);
    m_notEnoughTimers->setEnabled(false);
    m_notEnoughTimers->setMaxLength(39);
    m_defNotEnoughTimers = new QLineEdit(this);
    m_defNotEnoughTimers->setEnabled(false);
    m_cbNotEnoughMemory = new QCheckBox(tr("Out of Memory:"), this);
    m_notEnoughMemory = new QLineEdit(this);
    m_notEnoughMemory->setEnabled(false);
    m_notEnoughMemory->setMaxLength(52);
    m_defNotEnoughMemory = new QLineEdit(this);
    m_defNotEnoughMemory->setEnabled(false);
    m_cbCorruptDataFile = new QCheckBox(tr("Corrupt DAT File:"), this);
    m_corruptDataFile = new QLineEdit(this);
    m_corruptDataFile->setEnabled(false);
    m_corruptDataFile->setMaxLength(39);
    m_defCorruptDataFile = new QLineEdit(this);
    m_defCorruptDataFile->setEnabled(false);

    auto layout = new QGridLayout(content);
    int row = 0;
    layout->addWidget(new QLabel(tr("Override"), this), row, 1);
    layout->addWidget(new QLabel(tr("Default"), this), row, 2);
    layout->addWidget(m_cbFireDeath, ++row, 0);
    layout->addWidget(m_fireDeath, row, 1);
    layout->addWidget(m_defFireDeath, row, 2);
    layout->addWidget(m_cbWaterDeath, ++row, 0);
    layout->addWidget(m_waterDeath, row, 1);
    layout->addWidget(m_defWaterDeath, row, 2);
    layout->addWidget(m_cbBombDeath, ++row, 0);
    layout->addWidget(m_bombDeath, row, 1);
    layout->addWidget(m_defBombDeath, row, 2);
    layout->addWidget(m_cbBlockDeath, ++row, 0);
    layout->addWidget(m_blockDeath, row, 1);
    layout->addWidget(m_defBlockDeath, row, 2);
    layout->addWidget(m_cbCreatureDeath, ++row, 0);
    layout->addWidget(m_creatureDeath, row, 1);
    layout->addWidget(m_defCreatureDeath, row, 2);
    layout->addWidget(m_cbTimeLimit, ++row, 0);
    layout->addWidget(m_timeLimit, row, 1);
    layout->addWidget(m_defTimeLimit, row, 2);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), ++row, 0, 1, 3);
    layout->addWidget(m_cbNewGameConfirm, ++row, 0);
    layout->addWidget(m_newGameConfirm, row, 1);
    layout->addWidget(m_defNewGameConfirm, row, 2);
    layout->addWidget(m_cbSkipLevel, ++row, 0);
    layout->addWidget(m_skipLevel, row, 1);
    layout->addWidget(m_defSkipLevel, row, 2);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), ++row, 0, 1, 3);
    layout->addWidget(m_cbNotEnoughTimers, ++row, 0);
    layout->addWidget(m_notEnoughTimers, row, 1);
    layout->addWidget(m_defNotEnoughTimers, row, 2);
    layout->addWidget(m_cbNotEnoughMemory, ++row, 0);
    layout->addWidget(m_notEnoughMemory, row, 1);
    layout->addWidget(m_defNotEnoughMemory, row, 2);
    layout->addWidget(m_cbCorruptDataFile, ++row, 0);
    layout->addWidget(m_corruptDataFile, row, 1);
    layout->addWidget(m_defCorruptDataFile, row, 2);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), ++row, 0, 1, 3);

    auto topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(scroll);

    connect(m_cbFireDeath, &QCheckBox::toggled, m_fireDeath, &QWidget::setEnabled);
    connect(m_cbWaterDeath, &QCheckBox::toggled, m_waterDeath, &QWidget::setEnabled);
    connect(m_cbBombDeath, &QCheckBox::toggled, m_bombDeath, &QWidget::setEnabled);
    connect(m_cbBlockDeath, &QCheckBox::toggled, m_blockDeath, &QWidget::setEnabled);
    connect(m_cbCreatureDeath, &QCheckBox::toggled, m_creatureDeath, &QWidget::setEnabled);
    connect(m_cbTimeLimit, &QCheckBox::toggled, m_timeLimit, &QWidget::setEnabled);
    connect(m_cbNewGameConfirm, &QCheckBox::toggled, m_newGameConfirm, &QWidget::setEnabled);
    connect(m_cbSkipLevel, &QCheckBox::toggled, m_skipLevel, &QWidget::setEnabled);
    connect(m_cbNotEnoughTimers, &QCheckBox::toggled, m_notEnoughTimers, &QWidget::setEnabled);
    connect(m_cbNotEnoughMemory, &QCheckBox::toggled, m_notEnoughMemory, &QWidget::setEnabled);
    connect(m_cbCorruptDataFile, &QCheckBox::toggled, m_corruptDataFile, &QWidget::setEnabled);
}

void CCHack::PageMisc::setValues(HackSettings* settings)
{
    m_cbFireDeath->setChecked(settings->have_fireDeathMsg());
    m_fireDeath->setText(ccl::fromLatin1(settings->get_fireDeathMsg()));
    m_cbWaterDeath->setChecked(settings->have_waterDeathMsg());
    m_waterDeath->setText(ccl::fromLatin1(settings->get_waterDeathMsg()));
    m_cbBombDeath->setChecked(settings->have_bombDeathMsg());
    m_bombDeath->setText(ccl::fromLatin1(settings->get_bombDeathMsg()));
    m_cbBlockDeath->setChecked(settings->have_blockDeathMsg());
    m_blockDeath->setText(ccl::fromLatin1(settings->get_blockDeathMsg()));
    m_cbCreatureDeath->setChecked(settings->have_creatureDeathMsg());
    m_creatureDeath->setText(ccl::fromLatin1(settings->get_creatureDeathMsg()));
    m_cbTimeLimit->setChecked(settings->have_timeLimitMsg());
    m_timeLimit->setText(ccl::fromLatin1(settings->get_timeLimitMsg()));
    m_cbNewGameConfirm->setChecked(settings->have_newGameConfirmMsg());
    m_newGameConfirm->setPlainText(ccl::fromLatin1(settings->get_newGameConfirmMsg()));
    m_cbSkipLevel->setChecked(settings->have_skipLevelMsg());
    m_skipLevel->setPlainText(ccl::fromLatin1(settings->get_skipLevelMsg()));
    m_cbNotEnoughTimers->setChecked(settings->have_notEnoughTimersMsg());
    m_notEnoughTimers->setText(ccl::fromLatin1(settings->get_notEnoughTimersMsg()));
    m_cbNotEnoughMemory->setChecked(settings->have_notEnoughMemoryMsg());
    m_notEnoughMemory->setText(ccl::fromLatin1(settings->get_notEnoughMemoryMsg()));
    m_cbCorruptDataFile->setChecked(settings->have_corruptDataFileMsg());
    m_corruptDataFile->setText(ccl::fromLatin1(settings->get_corruptDataFileMsg()));
}

void CCHack::PageMisc::setDefaults(HackSettings* settings)
{
    m_defFireDeath->setText(ccl::fromLatin1(settings->get_fireDeathMsg()));
    m_defFireDeath->setCursorPosition(0);
    m_defWaterDeath->setText(ccl::fromLatin1(settings->get_waterDeathMsg()));
    m_defWaterDeath->setCursorPosition(0);
    m_defBombDeath->setText(ccl::fromLatin1(settings->get_bombDeathMsg()));
    m_defBombDeath->setCursorPosition(0);
    m_defBlockDeath->setText(ccl::fromLatin1(settings->get_blockDeathMsg()));
    m_defBlockDeath->setCursorPosition(0);
    m_defCreatureDeath->setText(ccl::fromLatin1(settings->get_creatureDeathMsg()));
    m_defCreatureDeath->setCursorPosition(0);
    m_defTimeLimit->setText(ccl::fromLatin1(settings->get_timeLimitMsg()));
    m_defTimeLimit->setCursorPosition(0);
    m_defNewGameConfirm->setPlainText(ccl::fromLatin1(settings->get_newGameConfirmMsg()));
    m_defSkipLevel->setPlainText(ccl::fromLatin1(settings->get_skipLevelMsg()));
    m_defNotEnoughTimers->setText(ccl::fromLatin1(settings->get_notEnoughTimersMsg()));
    m_defNotEnoughTimers->setCursorPosition(0);
    m_defNotEnoughMemory->setText(ccl::fromLatin1(settings->get_notEnoughMemoryMsg()));
    m_defNotEnoughMemory->setCursorPosition(0);
    m_defCorruptDataFile->setText(ccl::fromLatin1(settings->get_corruptDataFileMsg()));
    m_defCorruptDataFile->setCursorPosition(0);
}

void CCHack::PageMisc::saveTo(HackSettings* settings)
{
    if (m_cbFireDeath->isChecked())
        settings->set_fireDeathMsg(ccl::toLatin1(m_fireDeath->text()));
    if (m_cbWaterDeath->isChecked())
        settings->set_waterDeathMsg(ccl::toLatin1(m_waterDeath->text()));
    if (m_cbBombDeath->isChecked())
        settings->set_bombDeathMsg(ccl::toLatin1(m_bombDeath->text()));
    if (m_cbBlockDeath->isChecked())
        settings->set_blockDeathMsg(ccl::toLatin1(m_blockDeath->text()));
    if (m_cbCreatureDeath->isChecked())
        settings->set_creatureDeathMsg(ccl::toLatin1(m_creatureDeath->text()));
    if (m_cbTimeLimit->isChecked())
        settings->set_timeLimitMsg(ccl::toLatin1(m_timeLimit->text()));
    if (m_cbNewGameConfirm->isChecked())
        settings->set_newGameConfirmMsg(ccl::toLatin1(m_newGameConfirm->toPlainText()));
    if (m_cbSkipLevel->isChecked())
        settings->set_skipLevelMsg(ccl::toLatin1(m_skipLevel->toPlainText()));
    if (m_cbNotEnoughTimers->isChecked())
        settings->set_notEnoughTimersMsg(ccl::toLatin1(m_notEnoughTimers->text()));
    if (m_cbNotEnoughMemory->isChecked())
        settings->set_notEnoughMemoryMsg(ccl::toLatin1(m_notEnoughMemory->text()));
    if (m_cbCorruptDataFile->isChecked())
        settings->set_corruptDataFileMsg(ccl::toLatin1(m_corruptDataFile->text()));
}
