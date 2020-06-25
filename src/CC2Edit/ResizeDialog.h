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

#ifndef _CC2_RESIZEDLG_H
#define _CC2_RESIZEDLG_H

#include <QDialog>

class QSpinBox;

class ResizeDialog : public QDialog {
public:
    explicit ResizeDialog(const QSize& curSize, QWidget* parent = nullptr);

    QSize requestedSize() const;

private:
    QSpinBox* m_width;
    QSpinBox* m_height;
};

#endif
