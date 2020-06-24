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

#ifndef _SCRIPTEDITOR_H
#define _SCRIPTEDITOR_H

#include "qtextpad/syntaxtextedit.h"

class CC2ScriptEditor : public SyntaxTextEdit {
    Q_OBJECT

public:
    explicit CC2ScriptEditor(QWidget* parent = nullptr);

    QString filename() const { return m_filename; }
    void setFilename(const QString& filename) { m_filename = filename; }

    bool canUndo() const;
    bool canRedo() const;

private:
    QString m_filename;
};

#endif
