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

#ifndef _LL_TEXT_EDIT_H
#define _LL_TEXT_EDIT_H

#include <QPlainTextEdit>

/* A QPlainTextEdit which allows for a maximum content length. */
class LLTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit LLTextEdit(QWidget* parent = nullptr);

    QSize sizeHint() const override;

    int maxLength() const { return m_maxLength; }
    void setMaxLength(int maxLength);

private:
    int m_maxLength;
    QString m_previousText;
};

#endif
