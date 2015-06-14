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

#include "PlaySettings.h"

#include <QApplication>
#include <QTabWidget>
#include <QLabel>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QToolBar>
#include <QGridLayout>
#include <QCompleter>
#include <QDirModel>
#include <QFileDialog>
#include <QMessageBox>

#ifdef Q_OS_WIN
    #define EXE_FILTER "Executables (*.exe)"
    #define WINEXE_FILTER "Executables (*.exe)"
    #define EXE_LIST QStringList() << "*.exe"
#else
    #define EXE_FILTER "Executables (*)"
    #define WINEXE_FILTER "Windows Executables (*.exe *.EXE)"
    #define EXE_LIST QStringList()
#endif

void SettingsDialog::CheckEditors(QSettings& settings)
{
    if (settings.contains("EditorNames"))
        return;

    // Add default CCEdit entry
#if defined(Q_OS_WIN)
    QString cceditPath = QDir(qApp->applicationDirPath()).absoluteFilePath("CCEdit.exe");
#elif defined(Q_OS_MAC)
    QDir appPath(qApp->applicationDirPath());
    appPath.cdUp();     // <bundle>.app/Contents/MacOS/<executable>
    appPath.cdUp();
    appPath.cdUp();
    QString cceditPath = QDir().absoluteFilePath("CCEdit.app/Contents/MacOS/CCEdit");
#else
    QString cceditPath = QDir(qApp->applicationDirPath()).absoluteFilePath("CCEdit");
#endif
    settings.setValue("EditorNames", QStringList() << "CCEdit");
    settings.setValue("EditorPaths", QStringList() << cceditPath + "|%F %L");
    settings.setValue("EditorIcons", QStringList() << "CCEdit");
}

QIcon SettingsDialog::IconForEditor(QString iconName)
{
    if (iconName == "CCEdit")
        return QIcon(":/res/edit-ccedit.png");
    if (iconName == "ChipEdit")
        return QIcon(":/res/edit-chipedit.png");
    if (iconName == "ChipW")
        return QIcon(":/res/edit-chipw.png");
    if (iconName == "CCDesign")
        return QIcon(":/res/edit-ccdesign.png");
    return QIcon();
}


SettingsDialog::SettingsDialog(QWidget* parent)
              : QDialog(parent)
{
    setWindowTitle("CCPlay Settings");

    m_actions[ActionEditEditor] = new QAction(QIcon(":/res/document-properties-sm.png"), tr("Add"), this);
    m_actions[ActionEditEditor]->setEnabled(false);
    m_actions[ActionAddEditor] = new QAction(QIcon(":/res/list-add.png"), tr("Add"), this);
    m_actions[ActionAddEditor]->setShortcut(Qt::Key_Insert);
    m_actions[ActionDelEditor] = new QAction(QIcon(":/res/list-remove.png"), tr("Remove"), this);
    m_actions[ActionDelEditor]->setShortcut(Qt::Key_Delete);
    m_actions[ActionDelEditor]->setEnabled(false);
    m_actions[ActionEditorUp] = new QAction(QIcon(":/res/arrow-up.png"), tr("Move Up"), this);
    m_actions[ActionEditorUp]->setShortcut(Qt::CTRL | Qt::Key_Up);
    m_actions[ActionEditorUp]->setEnabled(false);
    m_actions[ActionEditorDown] = new QAction(QIcon(":/res/arrow-down.png"), tr("Move Down"), this);
    m_actions[ActionEditorDown]->setShortcut(Qt::CTRL | Qt::Key_Down);
    m_actions[ActionEditorDown]->setEnabled(false);

    QSettings settings("CCTools", "CCPlay");
    QCompleter* exeCompleter = new QCompleter(this);
    exeCompleter->setModel(new QDirModel(EXE_LIST,
            QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Executable,
            QDir::Name, exeCompleter));
    QCompleter* winExeCompleter = new QCompleter(this);
    winExeCompleter->setModel(new QDirModel(QStringList() << "*.exe",
            QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot,
            QDir::Name, winExeCompleter));

    QTabWidget* dlgTabs = new QTabWidget(this);
    QWidget* tabPlay = new QWidget(dlgTabs);
#ifndef Q_OS_WIN
    m_winePath = new QLineEdit(settings.value("WineExe").toString(), tabPlay);
    m_winePath->setCompleter(exeCompleter);
    QLabel* lblWinePath = new QLabel(tr("&WINE Path:"), tabPlay);
    lblWinePath->setBuddy(m_winePath);
    QToolButton* browseWine = new QToolButton(tabPlay);
    browseWine->setIcon(QIcon(":/res/document-open-folder.png"));
    browseWine->setAutoRaise(true);
#endif

    m_msccPath = new QLineEdit(settings.value("ChipsExe").toString(), tabPlay);
    m_msccPath->setCompleter(winExeCompleter);
    QLabel* lblMsccPath = new QLabel(tr("MS&CC Path:"), tabPlay);
    lblMsccPath->setBuddy(m_msccPath);
    QToolButton* browseChips = new QToolButton(tabPlay);
    browseChips->setIcon(QIcon(":/res/document-open-folder.png"));
    browseChips->setAutoRaise(true);
    m_tworldPath = new QLineEdit(settings.value("TWorldExe").toString(), tabPlay);
    m_tworldPath->setCompleter(exeCompleter);
    QLabel* lblTWorldPath = new QLabel(tr("&Tile World Path:"), tabPlay);
    lblTWorldPath->setBuddy(m_tworldPath);
    QToolButton* browseTWorld = new QToolButton(tabPlay);
    browseTWorld->setIcon(QIcon(":/res/document-open-folder.png"));
    browseTWorld->setAutoRaise(true);
    m_tworld2Path = new QLineEdit(settings.value("TWorld2Exe").toString(), tabPlay);
    m_tworld2Path->setCompleter(exeCompleter);
    QLabel* lblTWorld2Path = new QLabel(tr("Tile &World 2 Path:"), tabPlay);
    lblTWorld2Path->setBuddy(m_tworld2Path);
    QToolButton* browseTWorld2 = new QToolButton(tabPlay);
    browseTWorld2->setIcon(QIcon(":/res/document-open-folder.png"));
    browseTWorld2->setAutoRaise(true);

    m_useCCPatch = new QCheckBox(tr("MSCC: Use CCPatch"), tabPlay);
    m_useCCPatch->setChecked(settings.value("UseCCPatch", true).toBool());
    m_cheatIgnorePasswords = new QCheckBox(tr("&Ignore Passwords"), tabPlay);
    m_cheatIgnorePasswords->setChecked(settings.value("UseIgnorePasswords", false).toBool());
    m_cheatAlwaysFirstTry = new QCheckBox(tr("MSCC: Always give &First Try bonus"), tabPlay);
    m_cheatAlwaysFirstTry->setChecked(settings.value("UseAlwaysFirstTry", false).toBool());

    m_defaultGame = new QComboBox(tabPlay);
    m_defaultGame->addItems(QStringList() << "MSCC" << "Tile World" << "Tile World 2");
    m_defaultGame->setCurrentIndex(
                settings.value("DefaultGame").toString() == "TWorld2" ? 2 :
                settings.value("DefaultGame").toString() == "TWorld"  ? 1 : 0);
    QLabel* lblDefaultGame = new QLabel(tr("&Default Game:"), tabPlay);
    lblDefaultGame->setBuddy(m_defaultGame);

    QGridLayout* layPlay = new QGridLayout(tabPlay);
    layPlay->setContentsMargins(8, 8, 8, 8);
    layPlay->setVerticalSpacing(4);
    layPlay->setHorizontalSpacing(4);
#ifndef Q_OS_WIN
    layPlay->addWidget(lblWinePath, 0, 0);
    layPlay->addWidget(m_winePath, 0, 1);
    layPlay->addWidget(browseWine, 0, 2);
#endif
    layPlay->addWidget(lblMsccPath, 1, 0);
    layPlay->addWidget(m_msccPath, 1, 1);
    layPlay->addWidget(browseChips, 1, 2);
    layPlay->addWidget(m_useCCPatch, 2, 1);
    layPlay->addWidget(lblTWorldPath, 3, 0);
    layPlay->addWidget(m_tworldPath, 3, 1);
    layPlay->addWidget(browseTWorld, 3, 2);
    layPlay->addWidget(lblTWorld2Path, 4, 0);
    layPlay->addWidget(m_tworld2Path, 4, 1);
    layPlay->addWidget(browseTWorld2, 4, 2);
    layPlay->addWidget(lblDefaultGame, 5, 0);
    layPlay->addWidget(m_defaultGame, 5, 1);
    layPlay->addWidget(new QLabel(tr("Cheats:"), tabPlay), 6, 0);
    layPlay->addWidget(m_cheatIgnorePasswords, 6, 1);
    layPlay->addWidget(m_cheatAlwaysFirstTry, 7, 1);
    layPlay->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding,
                                     QSizePolicy::MinimumExpanding), 8, 0, 1, 3);
    dlgTabs->addTab(tabPlay, tr("&Game Settings"));

    QWidget* tabEdit = new QWidget(dlgTabs);
    m_editorList = new QListWidget(tabEdit);
    m_editorList->setIconSize(QSize(32, 32));
    QLabel* lblEditors = new QLabel(tr("Available Edi&tors:\n\n(First item is\n  default)"), tabEdit);
    lblEditors->setBuddy(m_editorList);
    m_editors = settings.value("EditorNames").toStringList();
    m_editorIcons = settings.value("EditorIcons").toStringList();
    m_editorPaths = settings.value("EditorPaths").toStringList();
    refreshEditors();
    QToolBar* tbEditors = new QToolBar(tabEdit);
    tbEditors->setOrientation(Qt::Vertical);
    tbEditors->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tbEditors->setIconSize(QSize(22, 22));
    tbEditors->addAction(m_actions[ActionEditEditor]);
    tbEditors->addSeparator();
    tbEditors->addAction(m_actions[ActionAddEditor]);
    tbEditors->addAction(m_actions[ActionDelEditor]);
    tbEditors->addSeparator();
    tbEditors->addAction(m_actions[ActionEditorUp]);
    tbEditors->addAction(m_actions[ActionEditorDown]);

    QGridLayout* layEdit = new QGridLayout(tabEdit);
    layEdit->setContentsMargins(8, 8, 8, 8);
    layEdit->setVerticalSpacing(4);
    layEdit->setHorizontalSpacing(4);
    layEdit->addWidget(lblEditors, 0, 0);
    layEdit->setAlignment(lblEditors, Qt::AlignTop);
    layEdit->addWidget(m_editorList, 0, 1);
    layEdit->addWidget(tbEditors, 0, 2);
    dlgTabs->addTab(tabEdit, tr("&Editor Settings"));

    QWidget* tabAbout = new QWidget(dlgTabs);
    QLabel* lblAIcon = new QLabel(tabAbout);
    lblAIcon->setPixmap(QPixmap(":/icons/chip-48.png"));
    lblAIcon->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    QLabel* lblLicense = new QLabel(tabAbout);
    lblLicense->setText(tr("\
CCTools is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program.  If not, see <http://www.gnu.org/licenses/>."));

    QGridLayout* layAbout = new QGridLayout(tabAbout);
    layAbout->setContentsMargins(8, 8, 8, 8);
    layAbout->setVerticalSpacing(8);
    layAbout->setHorizontalSpacing(8);
    layAbout->addWidget(lblAIcon, 0, 0, 3, 1);
    layAbout->addWidget(new QLabel("CCPlay 2.1.0", tabAbout), 0, 1);
    layAbout->addWidget(new QLabel("Part of CCTools 2.1", tabAbout), 1, 1);
    layAbout->addWidget(new QLabel(tr("Copyright (C) 2010  Michael Hansen"), tabAbout), 2, 1);
    layAbout->addWidget(lblLicense, 4, 0, 1, 2);
    dlgTabs->addTab(tabAbout, tr("&About CCPlay"));

    QDialogButtonBox* buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(8);
    layout->addWidget(dlgTabs, 0, 0);
#ifndef Q_OS_WIN
    layout->addWidget(new QLabel(
            tr("Note: Leave WINE or Tile World paths empty to use system-installed locations"),
            this), 1, 0);
#else
    layout->addWidget(new QLabel(
            tr("Note: MSCC will not work on 64-bit Windows platforms"),
            this), 1, 0);
#endif
    layout->addWidget(buttons, 2, 0);

    connect(buttons, SIGNAL(rejected()), SLOT(reject()));
    connect(buttons, SIGNAL(accepted()), SLOT(onSaveSettings()));
#ifndef Q_OS_WIN
    connect(browseWine, SIGNAL(clicked()), SLOT(onBrowseWine()));
#endif
    connect(browseChips, SIGNAL(clicked()), SLOT(onBrowseChips()));
    connect(browseTWorld, SIGNAL(clicked()), SLOT(onBrowseTWorld()));
    connect(browseTWorld2, SIGNAL(clicked()), SLOT(onBrowseTWorld2()));
    connect(m_editorList, SIGNAL(currentRowChanged(int)), SLOT(onSelectEditor(int)));
    connect(m_actions[ActionEditEditor], SIGNAL(triggered()), SLOT(onEditEditor()));
    connect(m_actions[ActionAddEditor], SIGNAL(triggered()), SLOT(onAddEditor()));
    connect(m_actions[ActionDelEditor], SIGNAL(triggered()), SLOT(onDelEditor()));
    connect(m_actions[ActionEditorUp], SIGNAL(triggered()), SLOT(onEditorUp()));
    connect(m_actions[ActionEditorDown], SIGNAL(triggered()), SLOT(onEditorDown()));
}

void SettingsDialog::refreshEditors()
{
    m_editorList->clear();
    for (int i=0; i<m_editors.size(); ++i) {
        new QListWidgetItem(IconForEditor(m_editorIcons[i]),
                m_editors[i], m_editorList);
    }
}

void SettingsDialog::onSaveSettings()
{
    QSettings settings("CCTools", "CCPlay");
#ifndef Q_OS_WIN
    settings.setValue("WineExe", m_winePath->text());
#endif
    settings.setValue("ChipsExe", m_msccPath->text());
    settings.setValue("TWorldExe", m_tworldPath->text());
    settings.setValue("TWorld2Exe", m_tworld2Path->text());
    settings.setValue("UseCCPatch", m_useCCPatch->isChecked());
    settings.setValue("UseIgnorePasswords", m_cheatIgnorePasswords->isChecked());
    settings.setValue("UseAlwaysFirstTry", m_cheatAlwaysFirstTry->isChecked());
    settings.setValue("DefaultGame",
                      m_defaultGame->currentIndex() == 2 ? "TWorld2" :
                      m_defaultGame->currentIndex() == 1 ? "TWorld"  : "MSCC");
    settings.setValue("EditorNames", m_editors);
    settings.setValue("EditorIcons", m_editorIcons);
    settings.setValue("EditorPaths", m_editorPaths);
    accept();
}

void SettingsDialog::onBrowseWine()
{
#ifndef Q_OS_WIN
    QString path = QFileDialog::getOpenFileName(this, tr("Browse for Wine executable"),
                                m_winePath->text(), EXE_FILTER);
    if (!path.isEmpty())
        m_winePath->setText(path);
#else
    qCritical("onBrowseWine: Not supported on Windows platforms");
#endif
}

void SettingsDialog::onBrowseChips()
{
    QString path = QFileDialog::getOpenFileName(this,
                                tr("Browse for MSCC executable"),
                                m_msccPath->text(), WINEXE_FILTER);
    if (!path.isEmpty())
        m_msccPath->setText(path);
}

void SettingsDialog::onBrowseTWorld()
{
    QString path = QFileDialog::getOpenFileName(this,
                                tr("Browse for Tile World executable"),
                                m_tworldPath->text(), EXE_FILTER);
    if (!path.isEmpty())
        m_tworldPath->setText(path);
}

void SettingsDialog::onBrowseTWorld2()
{
    QString path = QFileDialog::getOpenFileName(this,
                                tr("Browse for Tile World 2 executable"),
                                m_tworldPath->text(), EXE_FILTER);
    if (!path.isEmpty())
        m_tworld2Path->setText(path);
}

void SettingsDialog::onSelectEditor(int editorId)
{
    if (editorId < 0) {
        m_actions[ActionEditEditor]->setEnabled(false);
        m_actions[ActionDelEditor]->setEnabled(false);
        m_actions[ActionEditorUp]->setEnabled(false);
        m_actions[ActionEditorDown]->setEnabled(false);
    } else {
        m_actions[ActionEditEditor]->setEnabled(true);
        m_actions[ActionDelEditor]->setEnabled(true);
        m_actions[ActionEditorUp]->setEnabled(editorId > 0);
        m_actions[ActionEditorDown]->setEnabled(editorId < m_editorList->count() - 1);
    }
}

void SettingsDialog::onEditEditor()
{
    if (m_editorList->currentItem() == 0)
        return;

    int editorId = m_editorList->currentRow();
    QStringList params = m_editorPaths[editorId].split('|');

    ConfigEditorDialog dlg;
    dlg.setName(m_editors[editorId]);
    dlg.setIcon(m_editorIcons[editorId]);
    dlg.setPath(params[0]);
    dlg.setArgs(params[1]);
    if (dlg.exec() == QDialog::Accepted) {
        m_editors[editorId] = dlg.name();
        m_editorIcons[editorId] = dlg.icon();
        m_editorPaths[editorId] = dlg.path() + "|" + dlg.args();
        refreshEditors();
    }
}

void SettingsDialog::onAddEditor()
{
    ConfigEditorDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        m_editors << dlg.name();
        m_editorIcons << dlg.icon();
        m_editorPaths << dlg.path() + "|" + dlg.args();
        refreshEditors();
    }
}

void SettingsDialog::onDelEditor()
{
    if (m_editorList->currentItem() == 0)
        return;

    if (QMessageBox::question(this, tr("Remove Editor"),
                tr("Are you sure you want to remove this editor from the list?"),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        int editorId = m_editorList->currentRow();
        m_editors.removeAt(editorId);
        m_editorIcons.removeAt(editorId);
        m_editorPaths.removeAt(editorId);
        refreshEditors();
    }
}

void SettingsDialog::onEditorUp()
{
    if (m_editorList->currentItem() == 0 || m_editorList->currentRow() < 1)
        return;

    int editorId = m_editorList->currentRow();
    m_editors.insert(editorId - 1, m_editors.takeAt(editorId));
    m_editorIcons.insert(editorId - 1, m_editorIcons.takeAt(editorId));
    m_editorPaths.insert(editorId - 1, m_editorPaths.takeAt(editorId));
    refreshEditors();
    m_editorList->setCurrentRow(editorId - 1);
}

void SettingsDialog::onEditorDown()
{
    if (m_editorList->currentItem() == 0 || m_editorList->currentRow() > m_editorList->count() - 1)
        return;

    int editorId = m_editorList->currentRow();
    m_editors.insert(editorId + 1, m_editors.takeAt(editorId));
    m_editorIcons.insert(editorId + 1, m_editorIcons.takeAt(editorId));
    m_editorPaths.insert(editorId + 1, m_editorPaths.takeAt(editorId));
    refreshEditors();
    m_editorList->setCurrentRow(editorId + 1);
}



ConfigEditorDialog::ConfigEditorDialog(QWidget* parent)
                  : QDialog(parent)
{
    setWindowTitle(tr("Configure Editor"));

    QCompleter* exeCompleter = new QCompleter(this);
    exeCompleter->setModel(new QDirModel(EXE_LIST,
            QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Executable,
            QDir::Name, exeCompleter));

    m_name = new QLineEdit(this);
    QLabel* lblName = new QLabel(tr("&Name:"), this);
    lblName->setBuddy(m_name);
    m_icon = new QComboBox(this);
    m_icon->setIconSize(QSize(32, 32));
    m_icon->addItem(SettingsDialog::IconForEditor("CCEdit"), QString());
    m_icon->addItem(SettingsDialog::IconForEditor("ChipEdit"), QString());
    m_icon->addItem(SettingsDialog::IconForEditor("ChipW"), QString());
    m_icon->addItem(SettingsDialog::IconForEditor("CCDesign"), QString());
    QLabel* lblIcon = new QLabel(tr("&Icon:"), this);
    lblIcon->setBuddy(m_icon);
    m_path = new QLineEdit(this);
    m_path->setCompleter(exeCompleter);
    QLabel* lblPath = new QLabel(tr("Editor &Path:"), this);
    lblPath->setBuddy(m_path);
    QToolButton* browseEditor = new QToolButton(this);
    browseEditor->setIcon(QIcon(":/res/document-open-folder.png"));
    browseEditor->setAutoRaise(true);
    m_args = new QLineEdit(this);
    QLabel* lblArgs = new QLabel(tr("P&arameters:"), this);
    lblArgs->setBuddy(m_args);

    QDialogButtonBox* buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(8);
    layout->setHorizontalSpacing(8);
    layout->addWidget(lblName, 0, 0);
    layout->addWidget(m_name, 0, 1, 1, 2);
    layout->addWidget(lblIcon, 1, 0);
    layout->addWidget(m_icon, 1, 1);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding), 1, 2);
    layout->addItem(new QSpacerItem(0, 8), 2, 0, 1, 4);
    layout->addWidget(lblPath, 3, 0);
    layout->addWidget(m_path, 3, 1, 1, 2);
    layout->addWidget(browseEditor, 3, 3);
    layout->addWidget(lblArgs, 4, 0);
    layout->addWidget(m_args, 4, 1, 1, 2);
    layout->addWidget(new QLabel(tr("%F = filename, %L = level number"), this), 5, 1, 1, 2);
    layout->addWidget(buttons, 6, 0, 1, 4);
    resize(400, sizeHint().height());

    connect(buttons, SIGNAL(accepted()), SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), SLOT(reject()));
    connect(browseEditor, SIGNAL(clicked()), SLOT(onBrowseEditor()));
}

void ConfigEditorDialog::setIcon(QString icon)
{
    if (icon == "CCEdit")
        m_icon->setCurrentIndex(0);
    else if (icon == "ChipEdit")
        m_icon->setCurrentIndex(1);
    else if (icon == "ChipW")
        m_icon->setCurrentIndex(2);
    else if (icon == "CCDesign")
        m_icon->setCurrentIndex(3);
}

QString ConfigEditorDialog::icon() const
{
    switch (m_icon->currentIndex()) {
    case 1:
        return "ChipEdit";
    case 2:
        return "ChipW";
    case 3:
        return "CCDesign";
    default:
        return "CCEdit";
    }
}

void ConfigEditorDialog::onBrowseEditor()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Browse for editor executable"),
                                m_path->text(), EXE_FILTER);
    if (!path.isEmpty())
        m_path->setText(path);
}
