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
#include "libcc2/Map.h"

MapUndoCommand::MapUndoCommand(CC2EditHistory::Type type, cc2::Map* before)
    : m_enter(1), m_type(type), m_targetMap(before),
      m_before(new cc2::Map), m_after()
{
    m_targetMap->ref();
    m_before->copyFrom(before);
}

MapUndoCommand::~MapUndoCommand()
{
    m_before->unref();
    if (m_after)
        m_after->unref();
    m_targetMap->unref();
}

bool MapUndoCommand::mergeWith(const QUndoCommand* command)
{
    if (command->id() != id() || m_type == CC2EditHistory::EditMap
            || m_type == CC2EditHistory::EditOptions)
        return false;

    auto mapCommand = dynamic_cast<const MapUndoCommand*>(command);
    Q_ASSERT(mapCommand);
    m_after->copyFrom(mapCommand->m_after);

#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    // Don't bother comparing map edits, since those are never merged
    if (m_before->version() == m_after->version()
            && m_before->lock() == m_after->lock()
            && m_before->title() == m_after->title()
            && m_before->author() == m_after->author()
            && m_before->editorVersion() == m_after->editorVersion()
            && m_before->clue() == m_after->clue()
            && m_before->note() == m_after->note()
            && m_before->option().timeLimit() == m_after->option().timeLimit())
        setObsolete(true);
#endif

    return true;
}

bool MapUndoCommand::leave(cc2::Map* after)
{
    if (--m_enter == 0) {
        Q_ASSERT(!m_after);
        if (after) {
            m_after = new cc2::Map;
            m_after->copyFrom(after);
        }
        return true;
    }
    return false;
}

void MapUndoCommand::undo()
{
    m_targetMap->copyFrom(m_before);
}

void MapUndoCommand::redo()
{
    m_targetMap->copyFrom(m_after);
}
