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

#include "PageSounds.h"
#include "CommonWidgets/CCTools.h"

#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>

CCHack::PageSounds::PageSounds(QWidget* parent)
    : HackPage(parent)
{
    auto scroll = new QScrollArea(this);
    auto content = new QWidget(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    m_cbToolSound = new QCheckBox(tr("Pickup Tool:"), this);
    m_toolSound = new QLineEdit(this);
    m_toolSound->setEnabled(false);
    m_toolSound->setMaxLength(9);
    m_defToolSound = new QLineEdit(this);
    m_defToolSound->setEnabled(false);

    m_cbDoorSound = new QCheckBox(tr("Open Door:"), this);
    m_doorSound = new QLineEdit(this);
    m_doorSound->setEnabled(false);
    m_doorSound->setMaxLength(8);
    m_defDoorSound = new QLineEdit(this);
    m_defDoorSound->setEnabled(false);

    m_cbDeathSound = new QCheckBox(tr("Death (Bummer):"), this);
    m_deathSound = new QLineEdit(this);
    m_deathSound->setEnabled(false);
    m_deathSound->setMaxLength(10);
    m_defDeathSound = new QLineEdit(this);
    m_defDeathSound->setEnabled(false);

    m_cbLevelCompleteSound = new QCheckBox(tr("Level Complete:"), this);
    m_levelCompleteSound = new QLineEdit(this);
    m_levelCompleteSound->setEnabled(false);
    m_levelCompleteSound->setMaxLength(10);
    m_defLevelCompleteSound = new QLineEdit(this);
    m_defLevelCompleteSound->setEnabled(false);

    m_cbSocketSound = new QCheckBox(tr("Open Socket:"), this);
    m_socketSound = new QLineEdit(this);
    m_socketSound->setEnabled(false);
    m_socketSound->setMaxLength(10);
    m_defSocketSound = new QLineEdit(this);
    m_defSocketSound->setEnabled(false);

    m_cbWallSound = new QCheckBox(tr("Hit Wall (Oof):"), this);
    m_wallSound = new QLineEdit(this);
    m_wallSound->setEnabled(false);
    m_wallSound->setMaxLength(8);
    m_defWallSound = new QLineEdit(this);
    m_defWallSound->setEnabled(false);

    m_cbThiefSound = new QCheckBox(tr("Thief:"), this);
    m_thiefSound = new QLineEdit(this);
    m_thiefSound->setEnabled(false);
    m_thiefSound->setMaxLength(10);
    m_defThiefSound = new QLineEdit(this);
    m_defThiefSound->setEnabled(false);

    m_cbSoundOnSound = new QCheckBox(tr("Sound Enabled:"), this);
    m_soundOnSound = new QLineEdit(this);
    m_soundOnSound->setEnabled(false);
    m_soundOnSound->setMaxLength(10);
    m_defSoundOnSound = new QLineEdit(this);
    m_defSoundOnSound->setEnabled(false);

    m_cbChipSound = new QCheckBox(tr("Pickup Chip:"), this);
    m_chipSound = new QLineEdit(this);
    m_chipSound->setEnabled(false);
    m_chipSound->setMaxLength(10);
    m_defChipSound = new QLineEdit(this);
    m_defChipSound->setEnabled(false);

    m_cbButtonSound = new QCheckBox(tr("Button:"), this);
    m_buttonSound = new QLineEdit(this);
    m_buttonSound->setEnabled(false);
    m_buttonSound->setMaxLength(8);
    m_defButtonSound = new QLineEdit(this);
    m_defButtonSound->setEnabled(false);

    m_cbWaterSound = new QCheckBox(tr("Water Splash:"), this);
    m_waterSound = new QLineEdit(this);
    m_waterSound->setEnabled(false);
    m_waterSound->setMaxLength(10);
    m_defWaterSound = new QLineEdit(this);
    m_defWaterSound->setEnabled(false);

    m_cbBombSound = new QCheckBox(tr("Bomb:"), this);
    m_bombSound = new QLineEdit(this);
    m_bombSound->setEnabled(false);
    m_bombSound->setMaxLength(8);
    m_defBombSound = new QLineEdit(this);
    m_defBombSound->setEnabled(false);

    m_cbTeleportSound = new QCheckBox(tr("Teleport:"), this);
    m_teleportSound = new QLineEdit(this);
    m_teleportSound->setEnabled(false);
    m_teleportSound->setMaxLength(12);
    m_defTeleportSound = new QLineEdit(this);
    m_defTeleportSound->setEnabled(false);

    m_cbTimerTickSound = new QCheckBox(tr("Timer Tick:"), this);
    m_timerTickSound = new QLineEdit(this);
    m_timerTickSound->setEnabled(false);
    m_timerTickSound->setMaxLength(10);
    m_defTimerTickSound = new QLineEdit(this);
    m_defTimerTickSound->setEnabled(false);

    m_cbTimesUpSound = new QCheckBox(tr("Time's Up (Bell):"), this);
    m_timesUpSound = new QLineEdit(this);
    m_timesUpSound->setEnabled(false);
    m_timesUpSound->setMaxLength(8);
    m_defTimesUpSound = new QLineEdit(this);
    m_defTimesUpSound->setEnabled(false);

    m_cbMidi_1 = new QCheckBox(tr("MIDI 1:"), this);
    m_midi_1 = new QLineEdit(this);
    m_midi_1->setEnabled(false);
    m_midi_1->setMaxLength(10);
    m_defMidi_1 = new QLineEdit(this);
    m_defMidi_1->setEnabled(false);

    m_cbMidi_2 = new QCheckBox(tr("MIDI 2:"), this);
    m_midi_2 = new QLineEdit(this);
    m_midi_2->setEnabled(false);
    m_midi_2->setMaxLength(10);
    m_defMidi_2 = new QLineEdit(this);
    m_defMidi_2->setEnabled(false);

    m_cbMidi_3 = new QCheckBox(tr("MIDI 3:"), this);
    m_midi_3 = new QLineEdit(this);
    m_midi_3->setEnabled(false);
    m_midi_3->setMaxLength(10);
    m_defMidi_3 = new QLineEdit(this);
    m_defMidi_3->setEnabled(false);

    auto layout = new QGridLayout(content);
    int row = 0;
    layout->addWidget(new QLabel(tr("Override"), this), row, 1);
    layout->addWidget(new QLabel(tr("Default"), this), row, 2);
    layout->addWidget(m_cbToolSound, ++row, 0);
    layout->addWidget(m_toolSound, row, 1);
    layout->addWidget(m_defToolSound, row, 2);
    layout->addWidget(m_cbDoorSound, ++row, 0);
    layout->addWidget(m_doorSound, row, 1);
    layout->addWidget(m_defDoorSound, row, 2);
    layout->addWidget(m_cbDeathSound, ++row, 0);
    layout->addWidget(m_deathSound, row, 1);
    layout->addWidget(m_defDeathSound, row, 2);
    layout->addWidget(m_cbLevelCompleteSound, ++row, 0);
    layout->addWidget(m_levelCompleteSound, row, 1);
    layout->addWidget(m_defLevelCompleteSound, row, 2);
    layout->addWidget(m_cbSocketSound, ++row, 0);
    layout->addWidget(m_socketSound, row, 1);
    layout->addWidget(m_defSocketSound, row, 2);
    layout->addWidget(m_cbWallSound, ++row, 0);
    layout->addWidget(m_wallSound, row, 1);
    layout->addWidget(m_defWallSound, row, 2);
    layout->addWidget(m_cbThiefSound, ++row, 0);
    layout->addWidget(m_thiefSound, row, 1);
    layout->addWidget(m_defThiefSound, row, 2);
    layout->addWidget(m_cbSoundOnSound, ++row, 0);
    layout->addWidget(m_soundOnSound, row, 1);
    layout->addWidget(m_defSoundOnSound, row, 2);
    layout->addWidget(m_cbChipSound, ++row, 0);
    layout->addWidget(m_chipSound, row, 1);
    layout->addWidget(m_defChipSound, row, 2);
    layout->addWidget(m_cbButtonSound, ++row, 0);
    layout->addWidget(m_buttonSound, row, 1);
    layout->addWidget(m_defButtonSound, row, 2);
    layout->addWidget(m_cbWaterSound, ++row, 0);
    layout->addWidget(m_waterSound, row, 1);
    layout->addWidget(m_defWaterSound, row, 2);
    layout->addWidget(m_cbBombSound, ++row, 0);
    layout->addWidget(m_bombSound, row, 1);
    layout->addWidget(m_defBombSound, row, 2);
    layout->addWidget(m_cbTeleportSound, ++row, 0);
    layout->addWidget(m_teleportSound, row, 1);
    layout->addWidget(m_defTeleportSound, row, 2);
    layout->addWidget(m_cbTimerTickSound, ++row, 0);
    layout->addWidget(m_timerTickSound, row, 1);
    layout->addWidget(m_defTimerTickSound, row, 2);
    layout->addWidget(m_cbTimesUpSound, ++row, 0);
    layout->addWidget(m_timesUpSound, row, 1);
    layout->addWidget(m_defTimesUpSound, row, 2);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), ++row, 0, 1, 3);
    layout->addWidget(m_cbMidi_1, ++row, 0);
    layout->addWidget(m_midi_1, row, 1);
    layout->addWidget(m_defMidi_1, row, 2);
    layout->addWidget(m_cbMidi_2, ++row, 0);
    layout->addWidget(m_midi_2, row, 1);
    layout->addWidget(m_defMidi_2, row, 2);
    layout->addWidget(m_cbMidi_3, ++row, 0);
    layout->addWidget(m_midi_3, row, 1);
    layout->addWidget(m_defMidi_3, row, 2);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), ++row, 0, 1, 3);
    layout->addWidget(new QLabel(tr("NOTE: The above are default values.  They can also be overridden by the .ini file."),
                                 this), ++row, 0, 1, 3);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), ++row, 0, 1, 3);

    connect(m_cbToolSound, &QCheckBox::toggled, m_toolSound, &QWidget::setEnabled);
    connect(m_cbDoorSound, &QCheckBox::toggled, m_doorSound, &QWidget::setEnabled);
    connect(m_cbDeathSound, &QCheckBox::toggled, m_deathSound, &QWidget::setEnabled);
    connect(m_cbLevelCompleteSound, &QCheckBox::toggled, m_levelCompleteSound, &QWidget::setEnabled);
    connect(m_cbSocketSound, &QCheckBox::toggled, m_socketSound, &QWidget::setEnabled);
    connect(m_cbWallSound, &QCheckBox::toggled, m_wallSound, &QWidget::setEnabled);
    connect(m_cbThiefSound, &QCheckBox::toggled, m_thiefSound, &QWidget::setEnabled);
    connect(m_cbSoundOnSound, &QCheckBox::toggled, m_soundOnSound, &QWidget::setEnabled);
    connect(m_cbChipSound, &QCheckBox::toggled, m_chipSound, &QWidget::setEnabled);
    connect(m_cbButtonSound, &QCheckBox::toggled, m_buttonSound, &QWidget::setEnabled);
    connect(m_cbWaterSound, &QCheckBox::toggled, m_waterSound, &QWidget::setEnabled);
    connect(m_cbBombSound, &QCheckBox::toggled, m_bombSound, &QWidget::setEnabled);
    connect(m_cbTeleportSound, &QCheckBox::toggled, m_teleportSound, &QWidget::setEnabled);
    connect(m_cbTimerTickSound, &QCheckBox::toggled, m_timerTickSound, &QWidget::setEnabled);
    connect(m_cbTimesUpSound, &QCheckBox::toggled, m_timesUpSound, &QWidget::setEnabled);
    connect(m_cbMidi_1, &QCheckBox::toggled, m_midi_1, &QWidget::setEnabled);
    connect(m_cbMidi_2, &QCheckBox::toggled, m_midi_2, &QWidget::setEnabled);
    connect(m_cbMidi_3, &QCheckBox::toggled, m_midi_3, &QWidget::setEnabled);

    auto topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(scroll);
}

void CCHack::PageSounds::setValues(HackSettings* settings)
{
    m_cbToolSound->setChecked(settings->have_toolSound());
    m_toolSound->setText(ccl::fromLatin1(settings->get_toolSound()));
    m_cbDoorSound->setChecked(settings->have_doorSound());
    m_doorSound->setText(ccl::fromLatin1(settings->get_doorSound()));
    m_cbDeathSound->setChecked(settings->have_deathSound());
    m_deathSound->setText(ccl::fromLatin1(settings->get_deathSound()));
    m_cbLevelCompleteSound->setChecked(settings->have_levelCompleteSound());
    m_levelCompleteSound->setText(ccl::fromLatin1(settings->get_levelCompleteSound()));
    m_cbSocketSound->setChecked(settings->have_socketSound());
    m_socketSound->setText(ccl::fromLatin1(settings->get_socketSound()));
    m_cbWallSound->setChecked(settings->have_wallSound());
    m_wallSound->setText(ccl::fromLatin1(settings->get_wallSound()));
    m_cbThiefSound->setChecked(settings->have_thiefSound());
    m_thiefSound->setText(ccl::fromLatin1(settings->get_thiefSound()));
    m_cbSoundOnSound->setChecked(settings->have_soundOnSound());
    m_soundOnSound->setText(ccl::fromLatin1(settings->get_soundOnSound()));
    m_cbChipSound->setChecked(settings->have_chipSound());
    m_chipSound->setText(ccl::fromLatin1(settings->get_chipSound()));
    m_cbButtonSound->setChecked(settings->have_buttonSound());
    m_buttonSound->setText(ccl::fromLatin1(settings->get_buttonSound()));
    m_cbWaterSound->setChecked(settings->have_waterSound());
    m_waterSound->setText(ccl::fromLatin1(settings->get_waterSound()));
    m_cbBombSound->setChecked(settings->have_bombSound());
    m_bombSound->setText(ccl::fromLatin1(settings->get_bombSound()));
    m_cbTeleportSound->setChecked(settings->have_teleportSound());
    m_teleportSound->setText(ccl::fromLatin1(settings->get_teleportSound()));
    m_cbTimerTickSound->setChecked(settings->have_timerTickSound());
    m_timerTickSound->setText(ccl::fromLatin1(settings->get_timerTickSound()));
    m_cbTimesUpSound->setChecked(settings->have_timesUpSound());
    m_timesUpSound->setText(ccl::fromLatin1(settings->get_timesUpSound()));
    m_cbMidi_1->setChecked(settings->have_midi_1());
    m_midi_1->setText(ccl::fromLatin1(settings->get_midi_1()));
    m_cbMidi_2->setChecked(settings->have_midi_2());
    m_midi_2->setText(ccl::fromLatin1(settings->get_midi_2()));
    m_cbMidi_3->setChecked(settings->have_midi_3());
    m_midi_3->setText(ccl::fromLatin1(settings->get_midi_3()));
}

void CCHack::PageSounds::setDefaults(HackSettings* settings)
{
    m_defToolSound->setText(ccl::fromLatin1(settings->get_toolSound()));
    m_defDoorSound->setText(ccl::fromLatin1(settings->get_doorSound()));
    m_defDeathSound->setText(ccl::fromLatin1(settings->get_deathSound()));
    m_defLevelCompleteSound->setText(ccl::fromLatin1(settings->get_levelCompleteSound()));
    m_defSocketSound->setText(ccl::fromLatin1(settings->get_socketSound()));
    m_defWallSound->setText(ccl::fromLatin1(settings->get_wallSound()));
    m_defThiefSound->setText(ccl::fromLatin1(settings->get_thiefSound()));
    m_defSoundOnSound->setText(ccl::fromLatin1(settings->get_soundOnSound()));
    m_defChipSound->setText(ccl::fromLatin1(settings->get_chipSound()));
    m_defButtonSound->setText(ccl::fromLatin1(settings->get_buttonSound()));
    m_defWaterSound->setText(ccl::fromLatin1(settings->get_waterSound()));
    m_defBombSound->setText(ccl::fromLatin1(settings->get_bombSound()));
    m_defTeleportSound->setText(ccl::fromLatin1(settings->get_teleportSound()));
    m_defTimerTickSound->setText(ccl::fromLatin1(settings->get_timerTickSound()));
    m_defTimesUpSound->setText(ccl::fromLatin1(settings->get_timesUpSound()));
    m_defMidi_1->setText(ccl::fromLatin1(settings->get_midi_1()));
    m_defMidi_2->setText(ccl::fromLatin1(settings->get_midi_2()));
    m_defMidi_3->setText(ccl::fromLatin1(settings->get_midi_3()));
}

void CCHack::PageSounds::saveTo(HackSettings* settings)
{
    if (m_cbToolSound->isChecked())
        settings->set_toolSound(ccl::toLatin1(m_toolSound->text()));
    if (m_cbDoorSound->isChecked())
        settings->set_doorSound(ccl::toLatin1(m_doorSound->text()));
    if (m_cbDeathSound->isChecked())
        settings->set_deathSound(ccl::toLatin1(m_deathSound->text()));
    if (m_cbLevelCompleteSound->isChecked())
        settings->set_levelCompleteSound(ccl::toLatin1(m_levelCompleteSound->text()));
    if (m_cbSocketSound->isChecked())
        settings->set_socketSound(ccl::toLatin1(m_socketSound->text()));
    if (m_cbWallSound->isChecked())
        settings->set_wallSound(ccl::toLatin1(m_wallSound->text()));
    if (m_cbThiefSound->isChecked())
        settings->set_thiefSound(ccl::toLatin1(m_thiefSound->text()));
    if (m_cbSoundOnSound->isChecked())
        settings->set_soundOnSound(ccl::toLatin1(m_soundOnSound->text()));
    if (m_cbChipSound->isChecked())
        settings->set_chipSound(ccl::toLatin1(m_chipSound->text()));
    if (m_cbButtonSound->isChecked())
        settings->set_buttonSound(ccl::toLatin1(m_buttonSound->text()));
    if (m_cbWaterSound->isChecked())
        settings->set_waterSound(ccl::toLatin1(m_waterSound->text()));
    if (m_cbBombSound->isChecked())
        settings->set_bombSound(ccl::toLatin1(m_bombSound->text()));
    if (m_cbTeleportSound->isChecked())
        settings->set_teleportSound(ccl::toLatin1(m_teleportSound->text()));
    if (m_cbTimerTickSound->isChecked())
        settings->set_timerTickSound(ccl::toLatin1(m_timerTickSound->text()));
    if (m_cbTimesUpSound->isChecked())
        settings->set_timesUpSound(ccl::toLatin1(m_timesUpSound->text()));
    if (m_cbMidi_1->isChecked())
        settings->set_midi_1(ccl::toLatin1(m_midi_1->text()));
    if (m_cbMidi_2->isChecked())
        settings->set_midi_2(ccl::toLatin1(m_midi_2->text()));
    if (m_cbMidi_3->isChecked())
        settings->set_midi_3(ccl::toLatin1(m_midi_3->text()));
}
