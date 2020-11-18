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

#ifndef _PAGE_MISC_H
#define _PAGE_MISC_H

#include <QLineEdit>
#include <QCheckBox>
#include "HackSettings.h"
#include "CommonWidgets/LLTextEdit.h"

namespace CCHack {

class PageMisc : public HackPage {
    Q_OBJECT

public:
    explicit PageMisc(QWidget* parent = nullptr);
    void setValues(HackSettings* settings) override;
    void setDefaults(HackSettings* settings) override;
    void saveTo(HackSettings* settings) override;

private:
    QCheckBox* m_cbFireDeath;
    QLineEdit* m_fireDeath, * m_defFireDeath;
    QCheckBox* m_cbWaterDeath;
    QLineEdit* m_waterDeath, * m_defWaterDeath;
    QCheckBox* m_cbBombDeath;
    QLineEdit* m_bombDeath, * m_defBombDeath;
    QCheckBox* m_cbBlockDeath;
    QLineEdit* m_blockDeath, * m_defBlockDeath;
    QCheckBox* m_cbCreatureDeath;
    QLineEdit* m_creatureDeath, * m_defCreatureDeath;
    QCheckBox* m_cbTimeLimit;
    QLineEdit* m_timeLimit, * m_defTimeLimit;

    QCheckBox* m_cbNewGameConfirm;
    LLTextEdit* m_newGameConfirm;
    QPlainTextEdit* m_defNewGameConfirm;
    QCheckBox* m_cbSkipLevel;
    LLTextEdit* m_skipLevel;
    QPlainTextEdit* m_defSkipLevel;
    QCheckBox* m_cbNotEnoughTimers;
    QLineEdit* m_notEnoughTimers, * m_defNotEnoughTimers;
    QCheckBox* m_cbNotEnoughMemory;
    QLineEdit* m_notEnoughMemory, * m_defNotEnoughMemory;
    QCheckBox* m_cbCorruptDataFile;
    QLineEdit* m_corruptDataFile, * m_defCorruptDataFile;
};

}

#endif
