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

#include "ResizeDialog.h"

#include <QLabel>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QGridLayout>

ResizeDialog::ResizeDialog(const QSize& curSize, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Resize Map"));

    auto curSizeLabel = new QLabel(tr("Current Size:"), this);
    auto curSizeValue = new QLabel(tr("%1 x %2")
            .arg(curSize.width()).arg(curSize.height()), this);

    auto widthLabel = new QLabel(tr("New &Width: "), this);
    m_width = new QSpinBox(this);
    m_width->setRange(10, 100);
    m_width->setValue(curSize.width());
    widthLabel->setBuddy(m_width);
    auto widthRangeLabel = new QLabel(tr("(10 - 100)"), this);

    auto heightLabel = new QLabel(tr("New &Height:"), this);
    m_height = new QSpinBox(this);
    m_height->setRange(10, 100);
    m_height->setValue(curSize.height());
    heightLabel->setBuddy(m_height);
    auto heightRangeLabel = new QLabel(tr("(10 - 100)"), this);

    auto buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(8);
    layout->setHorizontalSpacing(8);
    layout->addWidget(curSizeLabel, 0, 0);
    layout->addWidget(curSizeValue, 0, 1);
    layout->addWidget(widthLabel, 1, 0);
    layout->addWidget(m_width, 1, 1);
    layout->addWidget(widthRangeLabel, 1, 2);
    layout->addWidget(heightLabel, 2, 0);
    layout->addWidget(m_height, 2, 1);
    layout->addWidget(heightRangeLabel, 2, 2);
    layout->addWidget(buttons, 3, 0, 1, 3);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QSize ResizeDialog::requestedSize() const
{
    return QSize(m_width->value(), m_height->value());
}
