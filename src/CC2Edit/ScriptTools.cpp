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

#include "ScriptTools.h"
#include "libcc2/GameScript.h"
#include "libcc2/Map.h"

#include <QMessageBox>
#include <QDir>
#include <stdexcept>
#include <unordered_map>

static void add_constants(std::unordered_map<std::string, unsigned long>& locals)
{
    // TODO: This should be read-only
    locals["male"] = cc2::Tile::Player;
    locals["female"] = cc2::Tile::Player2;
    locals["continue"] = 0x1;
    locals["replay"] = 0x2;
    locals["silent"] = 0x4;
    locals["ktools"] = 0x10;
    locals["ktime"] = 0x20;
    locals["no_bonus"] = 0x40;
}

static unsigned long
c2g_evaluate(std::unordered_map<std::string, unsigned long>& locals, cc2::C2GNode* node)
{
    switch (node->type()) {
    case cc2::C2GNode::NodeNumber:
        return static_cast<cc2::NumberNode*>(node)->value();
    case cc2::C2GNode::NodeIdentifier:
        return locals[static_cast<cc2::IdentifierNode*>(node)->name()];
    case cc2::C2GNode::NodeOperator:
        {
            auto opNode = static_cast<cc2::OperatorNode*>(node);
            unsigned long lhs = c2g_evaluate(locals, opNode->operand(0));
            unsigned long rhs = c2g_evaluate(locals, opNode->operand(1));
            if (opNode->token() == "<")
                return (lhs < rhs) ? 1 : 0;
            else if (opNode->token() == "<=")
                return (lhs <= rhs) ? 1 : 0;
            else if (opNode->token() == ">")
                return (lhs > rhs) ? 1 : 0;
            else if (opNode->token() == ">=")
                return (lhs >= rhs) ? 1 : 0;
            else if (opNode->token() == "==")
                return (lhs == rhs) ? 1 : 0;
            else if (opNode->token() == "!=")
                return (lhs != rhs) ? 1 : 0;
            else if (opNode->token() == "*")
                return lhs * rhs;
            else if (opNode->token() == "/") {
                if (rhs == 0) {
                    fprintf(stderr, "Divide by zero in evaluate()\n");
                    return 0;
                }
                return lhs / rhs;
            } else if (opNode->token() == "+")
                return lhs + rhs;
            else if (opNode->token() == "-")
                return lhs - rhs;
            else if (opNode->token() == "%") {
                if (rhs == 0) {
                    fprintf(stderr, "Divide by zero in evaluate()\n");
                    return 0;
                }
                return lhs % rhs;
            } else if (opNode->token() == "^")
                return lhs ^ rhs;
            else if (opNode->token() == "&&")
                return (lhs && rhs) ? 1 : 0;
            else if (opNode->token() == "&")
                return lhs & rhs;
            else if (opNode->token() == "||")
                return (lhs || rhs) ? 1 : 0;
            else if (opNode->token() == "|")
                return lhs | rhs;
            else {
                fprintf(stderr, "Unexpected operator \"%s\" in evaluate()", opNode->token().c_str());
                return 0;
            }
        }
        break;
    default:
        fprintf(stderr, "Unexpected node type %d in evaluate()\n", node->type());
        return 0;
    }
}

bool ScriptMapLoader::loadScript(const QString& filename)
{
    cc2::GameScript script;
    try {
        script.read(filename.toLocal8Bit().constData());
    } catch (const std::runtime_error &err) {
        QMessageBox::critical(nullptr, tr("Error loading script"), err.what());
        return false;
    }

    QDir scriptDir(filename);
    scriptDir.cdUp();

    std::unordered_map<std::string, unsigned long> locals;
    add_constants(locals);
    locals["level"] = 1;

    for (auto* node : script.nodes()) {
        switch (node->type()) {
        case cc2::C2GNode::NodeMap:
        case cc2::C2GNode::NodeGame:
            {
                auto cmdNode = static_cast<cc2::CommandNode*>(node);
                if (!cmdNode->param() || cmdNode->param()->type() != cc2::C2GNode::NodeString)
                    continue;
                auto paramNode = static_cast<cc2::StringNode*>(cmdNode->param());
                QString param = QString::fromStdString(paramNode->text());
                if (node->type() == cc2::C2GNode::NodeMap)
                    emit mapAdded(static_cast<int>(locals["level"]++),
                                  scriptDir.absoluteFilePath(param));
                else if (node->type() == cc2::C2GNode::NodeGame)
                    emit gameName(param);
            }
            break;
        case cc2::C2GNode::NodeOperator:
            {
                auto opNode = static_cast<cc2::OperatorNode*>(node);
                if (opNode->token() == "=") {
                    if (opNode->operand(0)->type() != cc2::C2GNode::NodeIdentifier)
                        continue;
                    auto identNode = static_cast<cc2::IdentifierNode*>(opNode->operand(0));
                    locals[identNode->name()] = c2g_evaluate(locals, opNode->operand(1));
                }
            }
            break;
        default:
            // Ignore this node
            break;
        }
    }

    return true;
}
