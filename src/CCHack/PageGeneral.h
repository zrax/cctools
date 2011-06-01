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

namespace CCHack {

class PageGeneral : public QObject {
    Q_OBJECT

public:
    PageGeneral(QWidget* parent);

private:
    QLineEdit* m_title,    * m_defTitle;
    QLineEdit* m_iniFile,  * m_defIniFile;
    QLineEdit* m_iniEntry, * m_defIniEntry;
    QLineEdit* m_datFile,  * m_defDatFile;

    QCheckBox* m_alwaysFirstTry;
    QCheckBox* m_ccPatch;
    QCheckBox* m_pgChips;

    QLineEdit* m_fakeLastLevel, * m_defFakeLastLevel;
    QLineEdit* m_realLastLevel, * m_defRealLastLevel;
};

}

#endif
