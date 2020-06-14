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

#ifndef _CC2EDIT_H
#define _CC2EDIT_H

#include <QMainWindow>
#include "libcc2/Map.h"
#include "libcc2/Tileset.h"
#include "ScriptEditor.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QListWidget;
class QPlainTextEdit;
class QActionGroup;

class CC2EditorWidget;

class CC2EditMain : public QMainWindow {
    Q_OBJECT

public:
    CC2EditMain(QWidget* parent = nullptr);

    void createNewMap();
    void createNewScript();
    void loadFile(const QString& filename);
    void loadMap(const QString& filename);
    void loadScript(const QString& filename);
    void editScript(const QString& filename);
    void findTilesets();
    void loadTileset(CC2ETileset* tileset);

    CC2EditorWidget* getEditorAt(int idx);
    CC2EditorWidget* addEditor(cc2::Map* map, const QString& filename);
    CC2ScriptEditor* getScriptEditorAt(int idx);
    CC2ScriptEditor* addScriptEditor(const QString& filename);
    void closeAllTabs();

Q_SIGNALS:
    void tilesetChanged(CC2ETileset*);
    void foregroundChanged(const cc2::Tile*);
    void backgroundChanged(const cc2::Tile*);

protected:
    void resizeEvent(QResizeEvent*) override;

private Q_SLOTS:
    void onOpenAction();
    void onCloseAction();
    void setZoomFactor(double);
    void onZoomCust();
    void onZoomFit();
    void onTilesetMenu(QAction*);

    void onDockChanged(Qt::DockWidgetArea);
    void onCloseTab(int);
    void onTabChanged(int);

    void setForeground(const cc2::Tile*);
    void setBackground(const cc2::Tile*);

private:
    enum ActionType {
        ActionNewMap, ActionNewScript, ActionOpen, ActionSave, ActionSaveAs, ActionClose,
        ActionGenReport, ActionExit, ActionSelect, ActionCut, ActionCopy,
        ActionPaste, ActionClear, ActionUndo, ActionRedo, ActionDrawPencil,
        ActionDrawLine, ActionDrawFill, ActionPathMaker, ActionDrawWire,
        ActionToggleWalls, ActionViewActivePlayer, ActionViewViewport,
        ActionViewMonsterPaths,
        ActionZoom100, ActionZoom75, ActionZoom50, ActionZoom25, ActionZoom125,
        ActionZoomCust, ActionZoomFit, ActionAbout,
        ActionReloadScript, ActionEditScript,
        NUM_ACTIONS
    };

    QAction* m_actions[NUM_ACTIONS];
    QMenu* m_tilesetMenu;
    QActionGroup* m_tilesetGroup;
    CC2ETileset* m_currentTileset;
    double m_zoomFactor;

    QTabWidget* m_toolTabs;

    // Game (c2g) properties
    QWidget* m_gameProperties;
    QLabel* m_gameName;
    QListWidget* m_gameMapList;
    QString m_currentGameScript;

    // Map properties
    QWidget* m_mapProperties;
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
    QPlainTextEdit* m_clue;
    QPlainTextEdit* m_note;

    cc2::Tile m_foreground, m_background;

    QTabWidget* m_editorTabs;

    QString m_dialogDir;

    void registerTileset(const QString& filename);
};

#endif // _CC2EDIT_H
