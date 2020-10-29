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

#include "GameScript.h"
#include "libcc1/Errors.h"

#include <memory>
#include <vector>
#include <cstring>

#ifdef _WIN32
#  define stricmp _stricmp
#else
#  define stricmp strcasecmp
#endif

std::string matchOperator(const char* lineBuffer)
{
    switch (lineBuffer[0]) {
    case '<':
    case '>':
    case '=':
        if (lineBuffer[1] == '=')
            return std::string(lineBuffer, 2);
        return std::string(lineBuffer, 1);
    case '!':
        if (lineBuffer[1] == '=')
            return std::string(lineBuffer, 2);
        break;
    case '*':
    case '/':
    case '+':
    case '-':
    case '%':
    case '^':
        return std::string(lineBuffer, 1);
    case '&':
    case '|':
        if (lineBuffer[1] == lineBuffer[0])
            return std::string(lineBuffer, 2);
        return std::string(lineBuffer, 1);
    default:
        break;
    }

    // Not an operator token
    return std::string();
}

void cc2::GameScript::read(const char* filename)
{
    std::unique_ptr<FILE, decltype(&fclose)> stream(fopen(filename, "r"), &fclose);
    if (!stream)
        throw ccl::IOError(ccl::RuntimeError::tr("Could not open file for reading"));

    int lineNo = 0;
    std::vector<char> lineBuffer;
    lineBuffer.resize(128);
    for ( ;; ) {
        if (fgets(&lineBuffer[0], lineBuffer.size(), stream.get()) == nullptr) {
            if (ferror(stream.get()))
                throw ccl::IOError(ccl::RuntimeError::tr("Error reading from script file"));
            break;
        }
        ++lineNo;

        size_t length = std::char_traits<char>::length(&lineBuffer[0]);
        if (length == 0)
            break;

        while (lineBuffer[length - 1] != '\r' && lineBuffer[length - 1] != '\n') {
            // Incomplete line, or unterminated final line
            lineBuffer.resize(lineBuffer.size() * 2);
            if (fgets(&lineBuffer[length], lineBuffer.size() - length, stream.get()) == nullptr) {
                if (ferror(stream.get()))
                    throw ccl::IOError(ccl::RuntimeError::tr("Error reading from script file"));
                break;
            }
            length = std::char_traits<char>::length(&lineBuffer[0]);
        }
        // Strip any newline characters
        while (length > 0 && (lineBuffer[length - 1] == '\r' || lineBuffer[length - 1] == '\n'))
            lineBuffer[--length] = 0;

        if (length == 0)
            continue;

        C2GNode* currentNode = nullptr;
        int column = 0;
        while (lineBuffer[column] != 0) {
            // Scan for a token
            while (std::isspace(lineBuffer[column]))
                ++column;

            if (lineBuffer[column] == ';'
                    || (lineBuffer[column] == '/' && lineBuffer[column + 1] == '/')) {
                // The rest of this line is a comment
                break;
            } else if (std::isdigit(lineBuffer[column])) {
                char* end = nullptr;
                unsigned long value = strtoul(&lineBuffer[column], &end, 10);
                currentNode = new NumberNode(value, lineNo);
                m_nodes.push_back(currentNode);
                column = end - &lineBuffer[0];
            } else if (lineBuffer[column] == '"') {
                int endColumn = ++column;
                while (lineBuffer[endColumn] && lineBuffer[endColumn] != '"')
                    ++endColumn;
                currentNode = new StringNode(std::string(&lineBuffer[column], endColumn - column), lineNo);
                if (!m_nodes.empty()) {
                    C2GNode* lastNode = m_nodes.back();
                    switch (lastNode->type()) {
                    case C2GNode::NodeMusic:
                    case C2GNode::NodeMap:
                    case C2GNode::NodeChain:
                    case C2GNode::NodeChdir:
                    case C2GNode::NodeArt:
                    case C2GNode::NodeWav:
                    case C2GNode::NodeGame:
                    case C2GNode::NodeDLC:
                        if (!static_cast<CommandNode*>(lastNode)->param())
                            static_cast<CommandNode*>(lastNode)->setParam(currentNode);
                        else
                            m_nodes.push_back(currentNode);
                        break;
                    case C2GNode::NodeScript:
                        static_cast<ScriptNode*>(lastNode)->addLine(currentNode);
                        break;
                    default:
                        m_nodes.push_back(currentNode);
                        break;
                    }
                }
                column = endColumn + 1;
            } else if (lineBuffer[column] == '#') {
                int end = ++column;
                while (std::isalnum(lineBuffer[end]) || lineBuffer[end] == '_')
                    ++end;
                currentNode = new LabelNode(std::string(&lineBuffer[column], end - column), lineNo);
                m_nodes.push_back(currentNode);
                column = end;
            } else if (std::isalpha(lineBuffer[column]) || lineBuffer[column] == '_') {
                int end = column;
                while (std::isalnum(lineBuffer[end]) || lineBuffer[end] == '_')
                    ++end;
                std::string identifier(&lineBuffer[column], end - column);

                if (stricmp(identifier.c_str(), "music") == 0)
                    currentNode = new CommandNode(C2GNode::NodeMusic, lineNo);
                else if (stricmp(identifier.c_str(), "map") == 0)
                    currentNode = new CommandNode(C2GNode::NodeMap, lineNo);
                else if (stricmp(identifier.c_str(), "goto") == 0)
                    currentNode = new CommandNode(C2GNode::NodeGoto, lineNo);
                else if (stricmp(identifier.c_str(), "do") == 0)
                    currentNode = new CommandNode(C2GNode::NodeDo, lineNo);
                else if (stricmp(identifier.c_str(), "end") == 0)
                    currentNode = new CommandNode(C2GNode::NodeEnd, lineNo);
                else if (stricmp(identifier.c_str(), "script") == 0)
                    currentNode = new ScriptNode(lineNo);
                else if (stricmp(identifier.c_str(), "edit") == 0)
                    currentNode = new CommandNode(C2GNode::NodeEdit, lineNo);
                else if (stricmp(identifier.c_str(), "chain") == 0)
                    currentNode = new CommandNode(C2GNode::NodeChain, lineNo);
                else if (stricmp(identifier.c_str(), "chdir") == 0)
                    currentNode = new CommandNode(C2GNode::NodeChdir, lineNo);
                else if (stricmp(identifier.c_str(), "art") == 0)
                    currentNode = new CommandNode(C2GNode::NodeArt, lineNo);
                else if (stricmp(identifier.c_str(), "wav") == 0)
                    currentNode = new CommandNode(C2GNode::NodeWav, lineNo);
                else if (stricmp(identifier.c_str(), "main") == 0)
                    currentNode = new CommandNode(C2GNode::NodeMain, lineNo);
                else if (stricmp(identifier.c_str(), "game") == 0)
                    currentNode = new CommandNode(C2GNode::NodeGame, lineNo);
                else if (stricmp(identifier.c_str(), "dlc") == 0)
                    currentNode = new CommandNode(C2GNode::NodeDLC, lineNo);
                else
                    currentNode = new IdentifierNode(identifier, lineNo);

                if (currentNode->type() == C2GNode::NodeIdentifier &&
                        !m_nodes.empty() && m_nodes.back()->type() == C2GNode::NodeScript) {
                    // TODO: Need to see how the actual game engine handles these
                    static_cast<ScriptNode*>(m_nodes.back())->addLine(currentNode);
                } else {
                    m_nodes.push_back(currentNode);
                }
                column = end;
            } else {
                std::string opToken = matchOperator(&lineBuffer[column]);
                if (!opToken.empty() && m_nodes.size() >= 2) {
                    C2GNode* op1 = m_nodes.back();
                    m_nodes.pop_back();
                    C2GNode* op2 = m_nodes.back();
                    m_nodes.pop_back();
                    currentNode = new OperatorNode(opToken, op1, op2, lineNo);
                    m_nodes.push_back(currentNode);
                    column += opToken.size();
                } else {
                    // Treat anything we don't recognize as junk, and store
                    // it for the checker to flag
                    currentNode = new JunkNode(&lineBuffer[column], lineNo);
                    m_nodes.push_back(currentNode);
                    break;
                }
            }
        }
    }
}
