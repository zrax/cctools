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

CCEHistory::CCEHistory() : m_temp(0)
{
    m_history = new CCEHistoryNode(CCEHistoryNode::HistInit);
    m_present = m_history;
}

CCEHistory::~CCEHistory()
{
    CCEHistoryNode* node = m_history;
    while (node != 0) {
        CCEHistoryNode* next = node->m_next;
        delete node;
        node = next;
    }
}

void CCEHistory::clear()
{
    CCEHistoryNode* node = m_history;
    while (node != 0) {
        CCEHistoryNode* next = node->m_next;
        delete node;
        node = next;
    }

    m_history = new CCEHistoryNode(CCEHistoryNode::HistInit);
    m_present = m_history;
}

ccl::LevelData* CCEHistory::undo()
{
    CCEHistoryNode* node = m_present;
    if (canUndo())
        m_present = m_present->m_prev;
    return node->m_before;
}

ccl::LevelData* CCEHistory::redo()
{
    if (canRedo())
        m_present = m_present->m_next;
    return m_present->m_after;
}

void CCEHistory::beginEdit(CCEHistoryNode::Type type, ccl::LevelData* before)
{
    m_temp = new CCEHistoryNode(type);
    m_temp->m_before = new ccl::LevelData(*before);
}

void CCEHistory::endEdit(ccl::LevelData* after)
{
    m_temp->m_after = new ccl::LevelData(*after);

    CCEHistoryNode* node = m_present->m_next;
    while (node != 0) {
        CCEHistoryNode* next = node->m_next;
        delete node;
        node = next;
    }
    m_temp->m_prev = m_present;
    m_present->m_next = m_temp;
    m_present = m_temp;
}

void CCEHistory::cancelEdit()
{
    delete m_temp;
}
