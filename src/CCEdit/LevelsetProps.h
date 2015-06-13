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

#ifndef _LEVELSETPROPS_H
#define _LEVELSETPROPS_H

#include <QDialog>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include "libcc1/Levelset.h"
#include "libcc1/DacFile.h"

class LevelsetProps : public QDialog {
    Q_OBJECT

public:
    LevelsetProps(QWidget* parent = 0);

    void setLevelset(ccl::Levelset* levelset);
    void setDacFile(ccl::DacFile* dac);

    int levelsetType() const;
    bool useDac() const { return m_dacGroup->isChecked(); }
    QString dacFilename() const { return m_dacFilename->text(); }
    int dacRuleset() const;
    int lastLevel() const { return m_lastLevel->value(); }
    bool usePasswords() const { return m_usePasswords->isChecked(); }

private:
    ccl::Levelset* m_levelset;
    ccl::DacFile* m_dacFile;

    QComboBox* m_levelsetType;
    QGroupBox* m_dacGroup;
    QLineEdit* m_dacFilename;
    QComboBox* m_dacRuleset;
    QSpinBox* m_lastLevel;
    QCheckBox* m_usePasswords;
};

#endif
