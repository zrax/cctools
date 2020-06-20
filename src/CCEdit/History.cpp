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

#include "History.h"
#include "libcc1/Levelset.h"

EditorUndoCommand::EditorUndoCommand(Type type, ccl::LevelData* before)
    : m_enter(1), m_type(type), m_levelPtr(before),
      m_before(new ccl::LevelData), m_after()
{
    m_levelPtr->ref();
    m_before->copyFrom(before);
}

EditorUndoCommand::~EditorUndoCommand()
{
    m_before->unref();
    if (m_after)
        m_after->unref();
    m_levelPtr->unref();
}

bool EditorUndoCommand::mergeWith(const QUndoCommand* command)
{
    if (command->id() != id() || m_type == EditMap)
        return false;

    auto editorCmd = dynamic_cast<const EditorUndoCommand*>(command);
    Q_ASSERT(editorCmd);
    m_after->copyFrom(editorCmd->m_after);

#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    // Don't bother comparing map edits, since those are never merged
    if (m_before->name() == m_after->name()
            && m_before->password() == m_after->password()
            && m_before->chips() == m_after->chips()
            && m_before->timer() == m_after->chips()
            && m_before->hint() == m_after->hint())
        setObsolete(true);
#endif

    return true;
}

bool EditorUndoCommand::leave(ccl::LevelData* after)
{
    if (--m_enter == 0) {
        Q_ASSERT(!m_after);
        if (after) {
            m_after = new ccl::LevelData;
            m_after->copyFrom(after);
        }
        return true;
    }
    return false;
}

void EditorUndoCommand::undo()
{
    m_levelPtr->copyFrom(m_before);
}

void EditorUndoCommand::redo()
{
    m_levelPtr->copyFrom(m_after);
}
