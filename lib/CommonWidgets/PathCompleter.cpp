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

#include "PathCompleter.h"

// TODO: QDirModel is deprecated, but QFileSystemModel doesn't work correctly
// for QCompleter, at least not in Qt 5.15.0
#include <QDirModel>

DirCompleter::DirCompleter(QObject* parent)
    : QCompleter(parent)
{
    auto model = new QDirModel(this);
    model->setFilter(QDir::AllDirs | QDir::Drives | QDir::NoDotAndDotDot);
    model->setSorting(QDir::Name | QDir::IgnoreCase);
    setModel(model);
    setCaseSensitivity(Qt::CaseInsensitive);
}

FileCompleter::FileCompleter(const QStringList& filters, QObject* parent)
    : QCompleter(parent)
{
    auto model = new QDirModel(this);
    model->setFilter(QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot);
    model->setNameFilters(filters);
    model->setSorting(QDir::Name | QDir::IgnoreCase);
    setModel(model);
    setCaseSensitivity(Qt::CaseInsensitive);
}
