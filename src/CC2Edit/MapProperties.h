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

#ifndef _CC2_MAP_PROPERTIES_H
#define _CC2_MAP_PROPERTIES_H

#include <QWidget>

#include "libcc2/Map.h"

class QLineEdit;
class QSpinBox;
class QComboBox;
class QCheckBox;

class CC2ScriptEditor;

class MapProperties : public QWidget
{
    Q_OBJECT

public:
    MapProperties(QWidget* parent);

    void clearAll();
    void updateCounters(const cc2::MapData& mapData);
    void updateMapProperties(cc2::Map* map);

    // Helper functions
    static QString formatChips(const std::tuple<int, int>& chips);
    static QString formatPoints(const std::tuple<int, int>& points);

Q_SIGNALS:
    void titleChanged(const std::string&);
    void authorChanged(const std::string&);
    void lockTextChanged(const std::string&);
    void editorVersionChanged(const std::string&);
    void timeLimitChanged(int);
    void viewportChanged(cc2::MapOption::Viewport);
    void blobPatternChanged(cc2::MapOption::BlobPattern);
    void hideLogicChanged(bool);
    void cc1BootsChanged(bool);
    void readOnlyChanged(bool);
    void clueChanged(const std::string&);
    void noteChanged(const std::string&);
    void mapResizeRequested();

private:
    QLineEdit* m_title;
    QLineEdit* m_author;
    QLineEdit* m_lockText;
    QLineEdit* m_editorVersion;
    QLineEdit* m_mapSize;
    QLineEdit* m_chipCounter;
    QLineEdit* m_pointCounter;
    QSpinBox* m_timeLimit;
    QComboBox* m_viewport;
    QComboBox* m_blobPattern;
    QCheckBox* m_hideLogic;
    QCheckBox* m_cc1Boots;
    QCheckBox* m_readOnly;
    CC2ScriptEditor* m_clue;
    CC2ScriptEditor* m_note;
};

#endif // _CC2_MAP_PROPERTIES_H
