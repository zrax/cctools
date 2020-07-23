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

#ifndef _PAGE_GENERAL_H
#define _PAGE_GENERAL_H

#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include "HackSettings.h"

namespace CCHack {

class PageGeneral : public HackPage {
    Q_OBJECT

public:
    explicit PageGeneral(QWidget* parent = nullptr);
    void setValues(HackSettings* settings) override;
    void setDefaults(HackSettings* settings) override;
    void saveTo(HackSettings* settings) override;

private:
    QCheckBox* m_cbTitle;
    QLineEdit* m_title,    * m_defTitle;
    QCheckBox* m_cbIniFile;
    QLineEdit* m_iniFile,  * m_defIniFile;
    QCheckBox* m_cbIniEntry;
    QLineEdit* m_iniEntry, * m_defIniEntry;
    QCheckBox* m_cbDatFile;
    QLineEdit* m_datFile,  * m_defDatFile;

    QCheckBox* m_alwaysFirstTry;
    QCheckBox* m_ccPatch;
    QCheckBox* m_pgChips;

    QCheckBox* m_cbFakeLastLevel;
    QSpinBox*  m_fakeLastLevel;
    QLineEdit* m_defFakeLastLevel;
    QCheckBox* m_cbRealLastLevel;
    QSpinBox*  m_realLastLevel;
    QLineEdit* m_defRealLastLevel;
};

}

#endif
