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

#ifndef _ERRORCHECK_H
#define _ERRORCHECK_H

#include <QDialog>
#include <QComboBox>
#include <QTreeWidget>
#include "../Levelset.h"
#include "../DacFile.h"

class ErrorCheckDialog : public QDialog {
    Q_OBJECT

public:
    ErrorCheckDialog(QWidget* parent = 0);

    void setLevelsetInfo(ccl::Levelset* levelset, ccl::DacFile* dac);

private slots:
    void onCheck();

private:
    ccl::Levelset* m_levelset;
    ccl::DacFile* m_dacFile;

    QComboBox* m_checkMode;
    QComboBox* m_checkTarget;
    QTreeWidget* m_errors;

    void reportError(QString section, QString text);
    void checkLevel(int level);
};

#endif
