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

#ifndef _PAGE_SCORES_H
#define _PAGE_SCORES_H

#include <QLineEdit>
#include <QCheckBox>
#include "HackSettings.h"
#include "CommonWidgets/LLTextEdit.h"

namespace CCHack {

class PageScores : public HackPage {
    Q_OBJECT

public:
    explicit PageScores(QWidget* parent = nullptr);
    void setValues(HackSettings* settings) override;
    void setDefaults(HackSettings* settings) override;
    void saveTo(HackSettings* settings) override;

private:
    QCheckBox* m_cbFirstTry;
    QLineEdit* m_firstTry, * m_defFirstTry;
    QCheckBox* m_cbThirdTry;
    QLineEdit* m_thirdTry, * m_defThirdTry;
    QCheckBox* m_cbFifthTry;
    QLineEdit* m_fifthTry, * m_defFifthTry;
    QCheckBox* m_cbFinalTry;
    QLineEdit* m_finalTry, *m_defFinalTry;

    QCheckBox* m_cbEstTimeRecord;
    QLineEdit* m_estTimeRecord, * m_defEstTimeRecord;
    QCheckBox* m_cbBeatTimeRecord;
    QLineEdit* m_beatTimeRecord, * m_defBeatTimeRecord;
    QCheckBox* m_cbIncreasedScore;
    QLineEdit* m_increasedScore, * m_defIncreasedScore;

    QCheckBox* m_cbEndgameScore;
    LLTextEdit* m_endgameScore;
    QPlainTextEdit* m_defEndgameScore;
};

}

#endif
