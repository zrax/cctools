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

#ifndef _HINTEDIT_H
#define _HINTEDIT_H

#include <QPlainTextEdit>
#include <QDialog>

class HintTextEdit : public QPlainTextEdit {
public:
    explicit HintTextEdit(QWidget* parent);

    QSize sizeHint() const override;
};

class HintEditDialog : public QDialog {
    Q_OBJECT

public:
    HintEditDialog(int x, int y, QWidget* parent);

    void setText(const QString& text) { m_editor->setPlainText(text); }
    QString text() const { return m_editor->toPlainText(); }

private:
    HintTextEdit* m_editor;
};

#endif
