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

class TestSetupDialog : public QDialog {
    Q_OBJECT

public:
    TestSetupDialog(QWidget* parent = 0);

private:
#ifndef Q_OS_WIN32
    QLineEdit* m_winePath;
#endif
    QLineEdit* m_msccPath;
    QLineEdit* m_tworldPath;

private slots:
    void onSaveSettings();
#ifndef Q_OS_WIN32
    void onBrowseWine();
#endif
    void onBrowseChips();
    void onBrowseTWorld();
};

#endif
