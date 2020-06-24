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

#ifndef _CC2_HISTORY_H
#define _CC2_HISTORY_H

#include <QUndoCommand>

namespace cc2
{
    class Map;
}

namespace CC2EditHistory {
    enum Type {
        EditMap, EditTitle, EditAuthor, EditLock, EditVersion, EditTime,
        EditOptions, EditClue, EditNotes,
    };
}

class MapUndoCommand : public QUndoCommand {
public:
    MapUndoCommand(CC2EditHistory::Type type, cc2::Map* before);
    ~MapUndoCommand() override;

    int id() const override { return m_type; }
    bool mergeWith(const QUndoCommand* command) override;

    void enter() { ++m_enter; }
    bool leave(cc2::Map* after);

    void undo() override;
    void redo() override;

private:
    int m_enter;
    int m_type;
    cc2::Map* m_targetMap;
    cc2::Map* m_before;
    cc2::Map* m_after;
};

#endif
