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

#include "LLTextEdit.h"

LLTextEdit::LLTextEdit(QWidget* parent)
    : QPlainTextEdit(parent), m_maxLength(32767)
{
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    connect(document(), &QTextDocument::contentsChange, this,
            [this](int position, int removed, int added) {
        if (added == 0)
            return;

        if (toPlainText().length() > m_maxLength) {
            // Don't just undo() anything, since that can undo more than just
            // the input event that pushed us over the limit.  Instead, only
            // use undo to "fix" text replacement events.
            if (removed) {
                undo();
            } else {
                QTextCursor cursor = textCursor();
                cursor.setPosition(position);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, added);
                cursor.removeSelectedText();
            }
        }
    });
}

QSize LLTextEdit::sizeHint() const
{
    QSize hint = QPlainTextEdit::sizeHint();
    QFontMetricsF metrics(font());
    return { hint.width(), int(metrics.lineSpacing() * 4) };
}

void LLTextEdit::setMaxLength(int maxLength)
{
    m_maxLength = maxLength;

    const QString documentText = toPlainText();
    if (documentText.length() > m_maxLength)
        setPlainText(documentText.left(m_maxLength));
}
