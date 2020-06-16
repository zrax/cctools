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

#include "libcc1/Levelset.h"

struct CCEHistoryNode {
    enum Type {
        HistInit, HistDraw, HistClear, HistPaste, HistConnect, HistDisconnect,
        HistEditMech, HistToggleWalls,
    };

    explicit CCEHistoryNode(Type type)
        : m_type(type), m_before(), m_after(), m_prev(), m_next()
    { }

    ~CCEHistoryNode()
    {
        if (m_before)
            m_before->unref();
        if (m_after)
            m_after->unref();
    }

    Type m_type;
    ccl::LevelData* m_before;
    ccl::LevelData* m_after;
    CCEHistoryNode* m_prev;
    CCEHistoryNode* m_next;
};

class CCEHistory {
public:
    CCEHistory();
    ~CCEHistory();

    void clear();
    bool canUndo() const { return m_present->m_prev != nullptr; }
    bool canRedo() const { return m_present->m_next != nullptr; }
    ccl::LevelData* undo();
    ccl::LevelData* redo();

    void beginEdit(CCEHistoryNode::Type type, ccl::LevelData* before);
    void endEdit(ccl::LevelData* after);
    void cancelEdit();

private:
    CCEHistoryNode* m_history;
    CCEHistoryNode* m_present;
    CCEHistoryNode* m_temp;
    int m_entryCount;
};

#endif
