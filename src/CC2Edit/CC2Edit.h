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
#include <QProcess>
#include "libcc2/Map.h"
#include "libcc2/Tileset.h"
#include "EditorWidget.h"
#include "ScriptEditor.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QListWidget;
class QPlainTextEdit;
class QActionGroup;

class EditorTabWidget;

class CC2EditMain : public QMainWindow {
    Q_OBJECT

public:
    explicit CC2EditMain(QWidget* parent = nullptr);

    void createNewMap();
    void createNewScript();
    void loadFile(const QString& filename);
    bool loadMap(const QString& filename);
    bool loadScript(const QString& filename);
    void editScript(const QString& filename);
    void closeScript();

    void findTilesets();
    void loadTileset(CC2ETileset* tileset);

    CC2EditorWidget* getEditorAt(int idx);
    CC2EditorWidget* currentEditor();
    CC2EditorWidget* addEditor(cc2::Map* map, const QString& filename);
    CC2ScriptEditor* getScriptEditorAt(int idx);
    CC2ScriptEditor* currentScriptEditor();
    CC2ScriptEditor* addScriptEditor(const QString& filename);
    void closeAllTabs();

signals:
    void tilesetChanged(CC2ETileset*);
    void leftTileChanged(const cc2::Tile*);
    void rightTileChanged(const cc2::Tile*);

protected:
    void closeEvent(QCloseEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private slots:
    void onOpenAction();
    void onImportCC1Action();
    void onCutAction();
    void onCopyAction();
    void onPasteAction();
    void onClearAction();
    void onUndoAction();
    void onRedoAction();

    void onInspectHints(bool);
    void onInspectTiles(bool);
    void onToggleGreensAction();

    void onViewViewportToggled(bool);

    void onTilePicked(int x, int y);

    void setZoomFactor(double);
    void onZoomCust();
    void onZoomFit();
    void onTilesetMenu(QAction*);
    void onTestChips2();

    void onDockChanged(Qt::DockWidgetArea);
    void onTabChanged(int);
    void updateMapProperties(cc2::Map* map);

    void setLeftTile(const cc2::Tile*);
    void setRightTile(const cc2::Tile*);

    void onTitleChanged(const QString&);
    void onAuthorChanged(const QString&);
    void onLockChanged(const QString&);
    void onEditorVersionChanged(const QString&);
    void onTimeLimitChanged(int);
    void onViewportChanged();
    void onBlobPatternChanged();
    void onHideLogicChanged();
    void onCC1BootsChanged();
    void onReadOnlyChanged();
    void onClueChanged();
    void onNoteChanged();
    void onResizeMap();

    void onProcessError(QProcess::ProcessError err);
    void onProcessFinished(int result, QProcess::ExitStatus status);

    void setGameName(const QString& name);

private:
    enum ActionType {
        ActionNewMap, ActionNewScript, ActionOpen, ActionImportCC1, ActionSave,
        ActionSaveAs, ActionClose, ActionGenReport, ActionExit,
        ActionSelect, ActionCut, ActionCopy, ActionPaste, ActionClear,
        ActionUndo, ActionRedo, ActionDrawPencil, ActionDrawLine, ActionDrawFill,
        ActionPathMaker, ActionDrawWire, ActionInspectHints, ActionInspectTiles,
        ActionToggleGreens, ActionViewButtons, ActionViewActivePlayer,
        ActionViewViewport, ActionViewMonsterPaths,
        ActionZoom100, ActionZoom75, ActionZoom50, ActionZoom25, ActionZoom125,
        ActionZoomCust, ActionZoomFit, ActionTest, ActionTestSetup, ActionAbout,
        ActionReloadScript, ActionEditScript,
        NUM_ACTIONS
    };

    QAction* m_actions[NUM_ACTIONS];
    QMenu* m_tilesetMenu;
    QActionGroup* m_tilesetGroup;
    CC2ETileset* m_currentTileset;
    ActionType m_savedDrawMode;
    CC2EditorWidget::DrawMode m_currentDrawMode;
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

    cc2::Tile m_leftTile, m_rightTile;

    EditorTabWidget* m_editorTabs;

    QString m_dialogDir;
    QProcess* m_subProc;
    QString m_testGameDir;

    void registerTileset(const QString& filename);
};

#endif // _CC2EDIT_H
