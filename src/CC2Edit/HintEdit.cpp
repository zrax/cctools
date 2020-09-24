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

#include "HintEdit.h"

#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QStyle>
#include <QtMath>

HintTextEdit::HintTextEdit(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fixedFont.setPointSize((fixedFont.pointSize() * 3) / 2);
    setFont(fixedFont);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // TODO: Make the text centered to match the game display.
}

QSize HintTextEdit::sizeHint() const
{
    // Set a default size of 20 cols by 8 rows, to match the available
    // font rendering area in the game.  We don't restrict resize, so
    // the user can make this larger or smaller if necessary...
    QFontMetricsF fm(font());
    return { qCeil(fm.width(QString(20, 'x')) + (contentOffset().x() * 2)
                   + (document()->documentMargin() * 2))
                   + viewportMargins().left() + viewportMargins().right()
                   + style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2,
             qCeil((fm.height() * 8) + (contentOffset().y() * 2)
                   + (document()->documentMargin() * 2))
                   + viewportMargins().top() + viewportMargins().bottom() + 2 };
}

HintEditDialog::HintEditDialog(int x, int y, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Edit Hint Text"));

    auto label = new QLabel(tr("Hint text for (%1, %2):").arg(x).arg(y));
    m_editor = new HintTextEdit(this);
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                        Qt::Horizontal, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    layout->addWidget(label);
    layout->addWidget(m_editor);
    layout->addWidget(buttons);
}
