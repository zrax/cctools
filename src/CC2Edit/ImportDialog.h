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

#ifndef _IMPORTDIALOG_H
#define _IMPORTDIALOG_H

#include <QDialog>

namespace ccl { class Levelset; }
namespace cc2 { class Map; }

class QComboBox;

class ImportDialog : public QDialog {
    Q_OBJECT;

public:
    ImportDialog(QWidget* parent = nullptr);
    ~ImportDialog() override;

    bool loadLevelset(const QString& filename);
    cc2::Map* importMap(int* levelNum);

private:
    ccl::Levelset* m_levelset;
    QComboBox* m_levelSelect;
};

#endif
