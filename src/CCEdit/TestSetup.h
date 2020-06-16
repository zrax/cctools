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

#ifndef _TESTSETUP_H
#define _TESTSETUP_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>

class TestSetupDialog : public QDialog {
    Q_OBJECT

public:
    TestSetupDialog(QWidget* parent = nullptr);

private:
#ifndef Q_OS_WIN
    QLineEdit* m_winePath;
#endif
    QLineEdit* m_msccPath;
    QLineEdit* m_tworldPath;
    QLineEdit* m_tworld2Path;

    QCheckBox* m_useCCPatch;
    QCheckBox* m_usePGPatch;

private slots:
    void onSaveSettings();
    void onBrowseWine();
    void onBrowseChips();
    void onBrowseTWorld();
    void onBrowseTWorld2();
};

#endif
