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
    #define EXE_FILTER "Executables (*.exe *.bat *.jar)"
    #define WINEXE_FILTER "Executables (*.exe)"
    #define EXE_LIST QStringList() << "*.exe"
#else
    #define EXE_FILTER "Executables (*)"
    #define WINEXE_FILTER "Windows Executables (*.exe *.EXE)"
    #define EXE_LIST QStringList()
#endif

void SettingsDialog::CheckTools(QSettings& settings)
{
    if (settings.contains(QStringLiteral("EditorNames")))
        return;

    // Add default CCEdit entry
#if defined(Q_OS_WIN)
    QString cceditPath = QDir(QApplication::applicationDirPath()).absoluteFilePath("CCEdit.exe");
#elif defined(Q_OS_MAC)
    QDir appPath(QApplication::applicationDirPath());
    appPath.cdUp();     // <bundle>.app/Contents/MacOS/<executable>
    appPath.cdUp();
    appPath.cdUp();
    QString cceditPath = QDir().absoluteFilePath("CCEdit.app/Contents/MacOS/CCEdit");
#else
    QString cceditPath = QDir(QApplication::applicationDirPath()).absoluteFilePath("CCEdit");
#endif
    settings.setValue("EditorNames", QStringList{ QStringLiteral("CCEdit") });
    settings.setValue("EditorPaths", QStringList{ cceditPath + QStringLiteral("|%F %L") });
    settings.setValue("EditorIcons", QStringList{ QStringLiteral("CCEdit") });
}

QIcon SettingsDialog::IconForTool(const QString& iconName)
{
    if (iconName == QStringLiteral("CCEdit"))
        return QIcon(":/res/edit-ccedit.png");
    if (iconName == QStringLiteral("ChipEdit"))
        return QIcon(":/res/edit-chipedit.png");
    if (iconName == QStringLiteral("ChipW"))
        return QIcon(":/res/edit-chipw.png");
    if (iconName == QStringLiteral("CCDesign"))
        return QIcon(":/res/edit-ccdesign.png");
    if (iconName == QStringLiteral("Generic"))
        return QIcon(":/res/application-x-executable.png");
    if (iconName == QStringLiteral("GenericScript"))
        return QIcon(":/res/application-x-executable-script.png");
    if (iconName == QStringLiteral("JavaJar"))
        return QIcon(":/res/application-x-java.png");
    return QIcon();
}


SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("CCPlay Settings");

    m_actions[ActionEditTool] = new QAction(QIcon(":/res/document-properties-sm.png"), tr("Add"), this);
    m_actions[ActionEditTool]->setEnabled(false);
    m_actions[ActionAddTool] = new QAction(QIcon(":/res/list-add.png"), tr("Add"), this);
    m_actions[ActionAddTool]->setShortcut(Qt::Key_Insert);
    m_actions[ActionDelTool] = new QAction(QIcon(":/res/list-remove.png"), tr("Remove"), this);
    m_actions[ActionDelTool]->setShortcut(Qt::Key_Delete);
    m_actions[ActionDelTool]->setEnabled(false);
    m_actions[ActionToolUp] = new QAction(QIcon(":/res/arrow-up.png"), tr("Move Up"), this);
    m_actions[ActionToolUp]->setShortcut(Qt::CTRL | Qt::Key_Up);
    m_actions[ActionToolUp]->setEnabled(false);
    m_actions[ActionToolDown] = new QAction(QIcon(":/res/arrow-down.png"), tr("Move Down"), this);
    m_actions[ActionToolDown]->setShortcut(Qt::CTRL | Qt::Key_Down);
    m_actions[ActionToolDown]->setEnabled(false);

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
    m_defaultGame->addItems(QStringList{ tr("MSCC"), tr("Tile World"), tr("Tile World 2") });
    m_defaultGame->setCurrentIndex(
                settings.value("DefaultGame").toString() == "TWorld2" ? 2 :
                settings.value("DefaultGame").toString() == "TWorld"  ? 1 : 0);
    QLabel* lblDefaultGame = new QLabel(tr("&Default Game:"), tabPlay);
    lblDefaultGame->setBuddy(m_defaultGame);

    QGridLayout* layPlay = new QGridLayout(tabPlay);
    layPlay->setContentsMargins(8, 8, 8, 8);
    layPlay->setVerticalSpacing(4);
    layPlay->setHorizontalSpacing(4);
    int row = -1;
#ifndef Q_OS_WIN
    layPlay->addWidget(lblWinePath, ++row, 0);
    layPlay->addWidget(m_winePath, row, 1);
    layPlay->addWidget(browseWine, row, 2);
#endif
    layPlay->addWidget(lblMsccPath, ++row, 0);
    layPlay->addWidget(m_msccPath, row, 1);
    layPlay->addWidget(browseChips, row, 2);
    layPlay->addWidget(m_useCCPatch, ++row, 1);
    layPlay->addWidget(lblTWorldPath, ++row, 0);
    layPlay->addWidget(m_tworldPath, row, 1);
    layPlay->addWidget(browseTWorld, row, 2);
    layPlay->addWidget(lblTWorld2Path, ++row, 0);
    layPlay->addWidget(m_tworld2Path, row, 1);
    layPlay->addWidget(browseTWorld2, row, 2);
    layPlay->addWidget(lblDefaultGame, ++row, 0);
    layPlay->addWidget(m_defaultGame, row, 1);
    layPlay->addWidget(new QLabel(tr("Cheats:"), tabPlay), ++row, 0);
    layPlay->addWidget(m_cheatIgnorePasswords, row, 1);
    layPlay->addWidget(m_cheatAlwaysFirstTry, ++row, 1);
    layPlay->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding,
                                     QSizePolicy::MinimumExpanding), ++row, 0, 1, 3);
    dlgTabs->addTab(tabPlay, tr("&Game Settings"));

    auto tabTools = new QWidget(dlgTabs);
    m_toolsList = new QListWidget(tabTools);
    m_toolsList->setIconSize(QSize(32, 32));
    auto lblTools = new QLabel(tr("Available &Tools:\n\n(First item is\n  default)"), tabTools);
    lblTools->setBuddy(m_toolsList);
    m_tools = settings.value("EditorNames").toStringList();
    m_toolIcons = settings.value("EditorIcons").toStringList();
    m_toolPaths = settings.value("EditorPaths").toStringList();
    refreshTools();
    auto tbTools = new QToolBar(tabTools);
    tbTools->setOrientation(Qt::Vertical);
    tbTools->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tbTools->setIconSize(QSize(22, 22));
    tbTools->addAction(m_actions[ActionEditTool]);
    tbTools->addSeparator();
    tbTools->addAction(m_actions[ActionAddTool]);
    tbTools->addAction(m_actions[ActionDelTool]);
    tbTools->addSeparator();
    tbTools->addAction(m_actions[ActionToolUp]);
    tbTools->addAction(m_actions[ActionToolDown]);

    auto layTools = new QGridLayout(tabTools);
    layTools->setContentsMargins(8, 8, 8, 8);
    layTools->setVerticalSpacing(4);
    layTools->setHorizontalSpacing(4);
    layTools->addWidget(lblTools, 0, 0);
    layTools->setAlignment(lblTools, Qt::AlignTop);
    layTools->addWidget(m_toolsList, 0, 1);
    layTools->addWidget(tbTools, 0, 2);
    dlgTabs->addTab(tabTools, tr("&Tool Settings"));

    QWidget* tabAbout = new QWidget(dlgTabs);
    QLabel* lblAIcon = new QLabel(tabAbout);
    lblAIcon->setPixmap(QPixmap(":/icons/chip-48.png"));
    lblAIcon->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    auto lblLicense = new QLabel(this);
    lblLicense->setTextFormat(Qt::RichText);
    lblLicense->setOpenExternalLinks(true);
    lblLicense->setWordWrap(true);
    lblLicense->setText(tr(
        "CCTools is free software: you can redistribute it and/or modify "
        "it under the terms of the GNU General Public License as published by "
        "the Free Software Foundation, either version 3 of the License, or "
        "(at your option) any later version.<br />"
        "<br />"
        "This program is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
        "GNU General Public License for more details.<br />"
        "<br />"
        "You should have received a copy of the GNU General Public License "
        "along with this program.  If not, see "
        "&lt;<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>&gt;."));

    QGridLayout* layAbout = new QGridLayout(tabAbout);
    layAbout->setContentsMargins(8, 8, 8, 8);
    layAbout->setVerticalSpacing(8);
    layAbout->setHorizontalSpacing(8);
    layAbout->addWidget(lblAIcon, 0, 0, 3, 1);
    layAbout->addWidget(new QLabel("CCPlay 2.0.95", tabAbout), 0, 1);
    layAbout->addWidget(new QLabel("Part of CCTools 2.1", tabAbout), 1, 1);
    layAbout->addWidget(new QLabel(tr("Copyright (C) 2020  Michael Hansen"), tabAbout), 2, 1);
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
            tr("Note: Leave executable paths empty to try system-installed locations"),
            this), 1, 0);
#else
    layout->addWidget(new QLabel(
            tr("Note: MSCC will not work on 64-bit Windows platforms"),
            this), 1, 0);
#endif
    layout->addWidget(buttons, 2, 0);

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::onSaveSettings);
#ifndef Q_OS_WIN
    connect(browseWine, &QToolButton::clicked, this, &SettingsDialog::onBrowseWine);
#endif
    connect(browseChips, &QToolButton::clicked, this, &SettingsDialog::onBrowseChips);
    connect(browseTWorld, &QToolButton::clicked, this, &SettingsDialog::onBrowseTWorld);
    connect(browseTWorld2, &QToolButton::clicked, this, &SettingsDialog::onBrowseTWorld2);
    connect(m_toolsList, &QListWidget::currentRowChanged, this, &SettingsDialog::onSelectTool);
    connect(m_actions[ActionEditTool], &QAction::triggered, this, &SettingsDialog::onEditTool);
    connect(m_actions[ActionAddTool], &QAction::triggered, this, &SettingsDialog::onAddTool);
    connect(m_actions[ActionDelTool], &QAction::triggered, this, &SettingsDialog::onDelTool);
    connect(m_actions[ActionToolUp], &QAction::triggered, this, &SettingsDialog::onToolUp);
    connect(m_actions[ActionToolDown], &QAction::triggered, this, &SettingsDialog::onToolDown);
}

void SettingsDialog::refreshTools()
{
    m_toolsList->clear();
    for (int i = 0; i < m_tools.size(); ++i)
        new QListWidgetItem(IconForTool(m_toolIcons[i]), m_tools[i], m_toolsList);
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
    // Legacy names kept for backwards compatibility
    settings.setValue("EditorNames", m_tools);
    settings.setValue("EditorIcons", m_toolIcons);
    settings.setValue("EditorPaths", m_toolPaths);
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

void SettingsDialog::onSelectTool(int toolId)
{
    if (toolId < 0) {
        m_actions[ActionEditTool]->setEnabled(false);
        m_actions[ActionDelTool]->setEnabled(false);
        m_actions[ActionToolUp]->setEnabled(false);
        m_actions[ActionToolDown]->setEnabled(false);
    } else {
        m_actions[ActionEditTool]->setEnabled(true);
        m_actions[ActionDelTool]->setEnabled(true);
        m_actions[ActionToolUp]->setEnabled(toolId > 0);
        m_actions[ActionToolDown]->setEnabled(toolId < m_toolsList->count() - 1);
    }
}

void SettingsDialog::onEditTool()
{
    if (!m_toolsList->currentItem())
        return;

    int toolId = m_toolsList->currentRow();
    QStringList params = m_toolPaths[toolId].split('|');

    ConfigToolDialog dlg;
    dlg.setName(m_tools[toolId]);
    dlg.setIcon(m_toolIcons[toolId]);
    dlg.setPath(params[0]);
    dlg.setArgs(params[1]);
    if (dlg.exec() == QDialog::Accepted) {
        m_tools[toolId] = dlg.name();
        m_toolIcons[toolId] = dlg.icon();
        m_toolPaths[toolId] = dlg.path() + "|" + dlg.args();
        refreshTools();
    }
}

void SettingsDialog::onAddTool()
{
    ConfigToolDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        m_tools << dlg.name();
        m_toolIcons << dlg.icon();
        m_toolPaths << dlg.path() + "|" + dlg.args();
        refreshTools();
    }
}

void SettingsDialog::onDelTool()
{
    if (!m_toolsList->currentItem())
        return;

    if (QMessageBox::question(this, tr("Remove Tool"),
                tr("Are you sure you want to remove this tool from the list?"),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        int toolId = m_toolsList->currentRow();
        m_tools.removeAt(toolId);
        m_toolIcons.removeAt(toolId);
        m_toolPaths.removeAt(toolId);
        refreshTools();
    }
}

void SettingsDialog::onToolUp()
{
    if (!m_toolsList->currentItem() || m_toolsList->currentRow() < 1)
        return;

    int toolId = m_toolsList->currentRow();
    m_tools.insert(toolId - 1, m_tools.takeAt(toolId));
    m_toolIcons.insert(toolId - 1, m_toolIcons.takeAt(toolId));
    m_toolPaths.insert(toolId - 1, m_toolPaths.takeAt(toolId));
    refreshTools();
    m_toolsList->setCurrentRow(toolId - 1);
}

void SettingsDialog::onToolDown()
{
    if (!m_toolsList->currentItem() || m_toolsList->currentRow() > m_toolsList->count() - 1)
        return;

    int toolId = m_toolsList->currentRow();
    m_tools.insert(toolId + 1, m_tools.takeAt(toolId));
    m_toolIcons.insert(toolId + 1, m_toolIcons.takeAt(toolId));
    m_toolPaths.insert(toolId + 1, m_toolPaths.takeAt(toolId));
    refreshTools();
    m_toolsList->setCurrentRow(toolId + 1);
}


ConfigToolDialog::ConfigToolDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Configure Tool"));

    QCompleter* exeCompleter = new QCompleter(this);
    exeCompleter->setModel(new QDirModel(EXE_LIST,
            QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Executable,
            QDir::Name, exeCompleter));

    m_name = new QLineEdit(this);
    QLabel* lblName = new QLabel(tr("&Name:"), this);
    lblName->setBuddy(m_name);
    m_icon = new QComboBox(this);
    m_icon->setIconSize(QSize(32, 32));
    static const QString iconNames[] = {
        QStringLiteral("CCEdit"), QStringLiteral("ChipEdit"),
        QStringLiteral("ChipW"), QStringLiteral("CCDesign"),
        QStringLiteral("Generic"), QStringLiteral("GenericScript"),
        QStringLiteral("JavaJar")
    };
    for (const QString& icon : iconNames)
        m_icon->addItem(SettingsDialog::IconForTool(icon), QString(), icon);
    QLabel* lblIcon = new QLabel(tr("&Icon:"), this);
    lblIcon->setBuddy(m_icon);
    m_path = new QLineEdit(this);
    m_path->setCompleter(exeCompleter);
    QLabel* lblPath = new QLabel(tr("Tool &Path:"), this);
    lblPath->setBuddy(m_path);
    auto browseTool = new QToolButton(this);
    browseTool->setIcon(QIcon(":/res/document-open-folder.png"));
    browseTool->setAutoRaise(true);
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
    layout->addWidget(browseTool, 3, 3);
    layout->addWidget(lblArgs, 4, 0);
    layout->addWidget(m_args, 4, 1, 1, 2);
    layout->addWidget(new QLabel(tr("%F = filename, %L = level number"), this), 5, 1, 1, 2);
    layout->addWidget(buttons, 6, 0, 1, 4);
    resize(400, sizeHint().height());

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(browseTool, &QToolButton::clicked, this, &ConfigToolDialog::onBrowseTool);
}

void ConfigToolDialog::setIcon(const QString& icon)
{
    for (int i = 0; i < m_icon->count(); ++i) {
        if (m_icon->itemData(i) == icon)
            m_icon->setCurrentIndex(i);
    }
}

QString ConfigToolDialog::icon() const
{
    return m_icon->currentData().toString();
}

void ConfigToolDialog::onBrowseTool()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Browse for tool executable"),
                                m_path->text(), EXE_FILTER);
    if (!path.isEmpty())
        m_path->setText(path);
}
