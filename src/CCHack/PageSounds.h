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

#ifndef _PAGE_SOUNDS_H
#define _PAGE_SOUNDS_H

#include <QCheckBox>
#include <QLineEdit>
#include "HackSettings.h"

namespace CCHack {

class PageSounds : public HackPage {
    Q_OBJECT

public:
    explicit PageSounds(QWidget* parent = nullptr);
    void setValues(HackSettings* settings) override;
    void setDefaults(HackSettings* settings) override;
    void saveTo(HackSettings* settings) override;

private:
    QCheckBox* m_cbToolSound;
    QLineEdit* m_toolSound, * m_defToolSound;
    QCheckBox* m_cbDoorSound;
    QLineEdit* m_doorSound, * m_defDoorSound;
    QCheckBox* m_cbDeathSound;
    QLineEdit* m_deathSound, * m_defDeathSound;
    QCheckBox* m_cbLevelCompleteSound;
    QLineEdit* m_levelCompleteSound, * m_defLevelCompleteSound;
    QCheckBox* m_cbSocketSound;
    QLineEdit* m_socketSound, * m_defSocketSound;
    QCheckBox* m_cbWallSound;
    QLineEdit* m_wallSound, * m_defWallSound;
    QCheckBox* m_cbThiefSound;
    QLineEdit* m_thiefSound, * m_defThiefSound;
    QCheckBox* m_cbSoundOnSound;
    QLineEdit* m_soundOnSound, * m_defSoundOnSound;
    QCheckBox* m_cbChipSound;
    QLineEdit* m_chipSound, * m_defChipSound;
    QCheckBox* m_cbButtonSound;
    QLineEdit* m_buttonSound, * m_defButtonSound;
    QCheckBox* m_cbWaterSound;
    QLineEdit* m_waterSound, * m_defWaterSound;
    QCheckBox* m_cbBombSound;
    QLineEdit* m_bombSound, * m_defBombSound;
    QCheckBox* m_cbTeleportSound;
    QLineEdit* m_teleportSound, * m_defTeleportSound;
    QCheckBox* m_cbTimerTickSound;
    QLineEdit* m_timerTickSound, * m_defTimerTickSound;
    QCheckBox* m_cbTimesUpSound;
    QLineEdit* m_timesUpSound, * m_defTimesUpSound;
    QCheckBox* m_cbMidi_1;
    QLineEdit* m_midi_1, * m_defMidi_1;
    QCheckBox* m_cbMidi_2;
    QLineEdit* m_midi_2, * m_defMidi_2;
    QCheckBox* m_cbMidi_3;
    QLineEdit* m_midi_3, * m_defMidi_3;
};

}

#endif
