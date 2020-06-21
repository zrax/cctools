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

#ifndef _CCEDIT_H
#define _CCEDIT_H

#include <QMainWindow>
#include <QProcess>
#include "EditorWidget.h"
#include "History.h"
#include "libcc1/Levelset.h"
#include "libcc1/DacFile.h"
#include "libcc1/CCMetaData.h"

class QMenu;
class QAction;
class QActionGroup;
class QTabWidget;
class QListWidget;
class QLineEdit;
class QSpinBox;
class QUndoStack;

class EditorTabWidget;

class CCEditMain : public QMainWindow {
    Q_OBJECT

public:
    explicit CCEditMain(QWidget* parent = nullptr);

    void loadLevelset(const QString& filename);
    void saveLevelset(const QString& filename);
    bool closeLevelset();
    void loadTileset(CCETileset* tileset);
    void findTilesets();

    void loadLevel(int level);
    void loadLevel(ccl::LevelData* levelPtr);

    EditorWidget* getEditorAt(int idx);
    EditorWidget* currentEditor();
    EditorWidget* addEditor(ccl::LevelData* level);
    void closeAllTabs();

signals:
    void tilesetChanged(CCETileset*);
    void foregroundChanged(tile_t);
    void backgroundChanged(tile_t);

private:
    enum ActionType {
        ActionNew, ActionOpen, ActionSave, ActionSaveAs, ActionClose,
        ActionGenReport, ActionExit, ActionSelect, ActionCut, ActionCopy,
        ActionPaste, ActionClear, ActionUndo, ActionRedo, ActionDrawPencil,
        ActionDrawLine, ActionDrawFill, ActionPathMaker, ActionConnect,
        ActionAdvancedMech, ActionToggleWalls, ActionCheckErrors,
        ActionViewButtons, ActionViewMovers, ActionViewActivePlayer,
        ActionViewViewport, ActionViewMonsterPaths, ActionViewErrors,
        ActionZoom100, ActionZoom75, ActionZoom50, ActionZoom25, ActionZoom125,
        ActionZoomCust, ActionZoomFit, ActionTestChips, ActionTestTWorldCC,
        ActionTestTWorldLynx, ActionTestTWorld2CC, ActionTestTWorld2Lynx,
        ActionTestSetup, ActionAbout,
        ActionAddLevel, ActionDelLevel, ActionMoveUp, ActionMoveDown,
        ActionProperties, ActionOrganize,
        NUM_ACTIONS
    };

    QAction* m_actions[NUM_ACTIONS];
    QMenu* m_tilesetMenu;
    QActionGroup* m_tilesetGroup;
    QUndoStack* m_undoStack;
    EditorUndoCommand* m_undoCommand;
    CCETileset* m_currentTileset;
    ActionType m_savedDrawMode;
    EditorWidget::DrawMode m_currentDrawMode;
    double m_zoomFactor;

    EditorTabWidget* m_editorTabs;
    QTabWidget* m_toolTabs;
    QListWidget* m_levelList;
    QLineEdit* m_nameEdit;
    QLineEdit* m_passwordEdit;
    QSpinBox* m_chipEdit;
    QSpinBox* m_timeEdit;
    QLineEdit* m_hintEdit;
    tile_t m_foreground, m_background;

    ccl::Levelset* m_levelset;
    unsigned int m_dirtyFlag;
    ccl::DacFile m_dacInfo;
    CCX::Levelset m_ccxFile;
    QString m_levelsetFilename;
    QString m_dialogDir;
    bool m_useDac, m_haveCcx;
    bool m_checkSave;

    enum SubprocType { SubprocMSCC, SubprocTWorld };
    QProcess* m_subProc;
    SubprocType m_subProcType;
    QString m_tempExe, m_tempDat, m_tempIni;

    int levelIndex(ccl::LevelData* level);
    void updateForUndoCommand(const QUndoCommand*);

protected:
    void registerTileset(const QString& filename);
    void doLevelsetLoad();
    void setLevelsetFilename(const QString& filename);
    void closeEvent(QCloseEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private slots:
    void onNewAction();
    void onOpenAction();
    void onSaveAction();
    void onSaveAsAction();
    void onReportAction();
    void onSelectToggled(bool);
    void onCutAction();
    void onCopyAction();
    void onPasteAction();
    void onClearAction();
    void onUndoAction();
    void onRedoAction();
    void onDrawPencilAction(bool);
    void onDrawLineAction(bool);
    void onDrawFillAction(bool);
    void onPathMakerToggled(bool);
    void onConnectToggled(bool);
    void onAdvancedMechAction();
    void onToggleWallsAction();
    void onCheckErrorsAction();
    void onViewButtonsToggled(bool);
    void onViewMoversToggled(bool);
    void onViewActivePlayerToggled(bool);
    void onViewViewportToggled(bool);
    void onViewMonsterPathsToggled(bool);
    void onViewErrorsToggled(bool);
    void setZoomFactor(double);
    void onZoomCust();
    void onZoomFit();
    void onTilesetMenu(QAction*);
    void onTestChips();
    void onTestTWorld(unsigned int levelsetType, bool tworld2);

    void beginEdit(CCEditHistory::Type type);
    void endEdit();
    void cancelEdit();
    void onCleanChanged(bool);

    void onAddLevelAction();
    void onDelLevelAction();
    void onMoveUpAction();
    void onMoveDownAction();
    void onPropertiesAction();
    void onOrganizeAction();

    void onSelectLevel(int);
    void onPasswordGenAction();
    void onChipCountAction();
    void onNameChanged(const QString&);
    void onPasswordChanged(const QString&);
    void onChipsChanged(int);
    void onTimerChanged(int);
    void onHintChanged(const QString&);
    void onClipboardDataChanged();

    void onTabChanged(int);
    void onDockChanged(Qt::DockWidgetArea);

    void setForeground(tile_t);
    void setBackground(tile_t);

    void onProcessFinished(int);
    void onProcessError(QProcess::ProcessError);
};

#endif
