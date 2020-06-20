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

#ifndef _CCEHISTORY_H
#define _CCEHISTORY_H

#include <QUndoCommand>

namespace ccl { class LevelData; }

class EditorUndoCommand : public QUndoCommand {
public:
    enum Type {
        EditMap, EditName, EditPassword, EditChips, EditTimer, EditHint
    };

    explicit EditorUndoCommand(Type type, ccl::LevelData* before);
    ~EditorUndoCommand() override;

    int id() const override { return m_type; }
    bool mergeWith(const QUndoCommand* command) override;

    ccl::LevelData* levelPtr() const { return m_levelPtr; }

    void enter() { ++m_enter; }
    bool leave(ccl::LevelData* after);

    void undo() override;
    void redo() override;

private:
    int m_enter;
    int m_type;
    ccl::LevelData* m_levelPtr;
    ccl::LevelData* m_before;
    ccl::LevelData* m_after;
};

#endif
