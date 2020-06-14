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

#ifndef _CC2GAMESCRIPT_H
#define _CC2GAMESCRIPT_H

#include <string>
#include <list>

namespace cc2 {

class C2GNode {
public:
    enum NodeType {
        // Identify junk for the checker to flag
        NodeJunk,

        // We treat constants and variables the same for our purposes
        NodeIdentifier,

        // Basic syntactic elements
        NodeOperator, NodeNumber, NodeString,
        NodeLabel,

        // Commands
        NodeMusic, NodeMap, NodeGoto, NodeDo, NodeEnd, NodeScript,
        NodeEdit, NodeChain, NodeChdir, NodeArt, NodeWav, NodeMain,
        NodeGame, NodeDLC,
    };

    enum CommentType {
        NoComment           = 0,
        CommentSemi         = 0x01000000,
        CommentCStyle       = 0x02000000,
        CommentColumnMask   = 0x00ffffff,
        CommentTypeMask     = 0x0f000000,
    };

    C2GNode(int type, int line) : m_type(type), m_line(line) { }
    virtual ~C2GNode() = default;

    int type() const { return m_type; }

private:
    int m_type;
    int m_line;
};

class JunkNode : public C2GNode {
public:
    JunkNode(std::string junk, int line)
        : C2GNode(NodeJunk, line), m_junk(std::move(junk)) { }

    const std::string& junk() const { return m_junk; }

private:
    std::string m_junk;
};

class IdentifierNode : public C2GNode {
public:
    IdentifierNode(std::string name, int line)
        : C2GNode(NodeIdentifier, line), m_name(std::move(name)) { }

    const std::string& name() const { return m_name; }

private:
    std::string m_name;
};

class OperatorNode : public C2GNode {
public:
    OperatorNode(std::string token, C2GNode* op1, C2GNode* op2, int line)
        : C2GNode(NodeOperator, line), m_token(std::move(token))
    {
        m_operands[0] = op1;
        m_operands[1] = op2;
    }

    ~OperatorNode() override
    {
        for (auto op : m_operands)
            delete op;
    }

    const std::string& token() const { return m_token; }
    C2GNode* operand(size_t which) const { return m_operands[which]; }

private:
    std::string m_token;
    C2GNode* m_operands[2];
};

class NumberNode : public C2GNode {
public:
    NumberNode(unsigned long value, int line)
        : C2GNode(NodeNumber, line), m_value(value) { }

    unsigned long value() const { return m_value; }

private:
    unsigned long m_value;
};

class StringNode : public C2GNode {
public:
    StringNode(std::string text, int line)
        : C2GNode(NodeString, line), m_text(std::move(text)) { }

    std::string text() const { return m_text; }

private:
    std::string m_text;
};

class LabelNode : public C2GNode {
public:
    LabelNode(std::string name, int line)
        : C2GNode(NodeLabel, line), m_name(std::move(name)) { }

    std::string name() const { return m_name; }

private:
    std::string m_name;
};

class CommandNode : public C2GNode {
public:
    CommandNode(int type, int line) : C2GNode(type, line), m_param() { }
    ~CommandNode() override { delete m_param; }

    void setParam(C2GNode* param) { m_param = param; }
    C2GNode* param() const { return m_param; }

private:
    C2GNode* m_param;
};

class ScriptNode : public C2GNode {
public:
    explicit ScriptNode(int line) : C2GNode(NodeScript, line) { }

    ~ScriptNode() override
    {
        for (auto line : m_lines)
            delete line;
    }

    const std::list<C2GNode*>& lines() const { return m_lines; }
    void addLine(C2GNode* line) { m_lines.push_back(line); }

private:
    std::list<C2GNode*> m_lines;
};

class GameScript {
public:
    GameScript() = default;

    ~GameScript()
    {
        for (auto node : m_nodes)
            delete node;
    }

    void read(const char* filename);

    const std::list<C2GNode *>& nodes() const { return m_nodes; }

private:
    std::list<C2GNode *> m_nodes;
};

}

#endif
