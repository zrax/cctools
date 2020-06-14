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

#include <QMessageBox>
#include <QDir>
#include <stdexcept>

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
                    emit mapAdded(scriptDir.absoluteFilePath(param));
                else if (node->type() == cc2::C2GNode::NodeGame)
                    emit gameName(param);
            }
            break;
        default:
            // Ignore this node
            break;
        }
    }

    return true;
}
