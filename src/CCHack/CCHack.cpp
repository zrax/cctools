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

#include "CCHack.h"

#include <QApplication>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QSplitter>
#include <QToolBar>
#include <QAction>
#include <QGridLayout>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include "PageGeneral.h"
#include "PageSounds.h"
#include "PageMenus.h"
#include "PageStoryline.h"
#include "PageScores.h"
#include "PageBitmap.h"
#include "CommonWidgets/CCTools.h"

static void addPageWithType(QTreeWidgetItem* parent, const QString& name, int type)
{
    auto item = new QTreeWidgetItem(parent, QStringList{name});
    item->setData(0, Qt::UserRole, type);
}

CCHackMain::CCHackMain(QWidget* parent)
    : QMainWindow(parent), m_pages()
{
    setWindowTitle(QStringLiteral("CCHack " CCTOOLS_VERSION));

    auto split = new QSplitter(this);
    auto pager = new QTreeWidget(split);
    pager->setHeaderHidden(true);
    pager->setRootIsDecorated(false);
    pager->setSelectionMode(QAbstractItemView::SingleSelection);
    pager->setItemsExpandable(false);

    // TODO: page graphics (?)
    auto tiGeneral = new QTreeWidgetItem(pager, QStringList{tr("General")});
    addPageWithType(tiGeneral, tr("General"), PageGeneral);
    addPageWithType(tiGeneral, tr("Sounds & Music"), PageSound);
    auto tiStrings = new QTreeWidgetItem(pager, QStringList{tr("Strings")});
    addPageWithType(tiStrings, tr("Menus"), PageMenus);
    addPageWithType(tiStrings, tr("Storyline"), PageStory);
    addPageWithType(tiStrings, tr("Scores & Records"), PageScores);
    addPageWithType(tiStrings, tr("Miscellaneous"), PageMisc);
    auto tiGraphics = new QTreeWidgetItem(pager, QStringList{tr("Graphics")});
    addPageWithType(tiGraphics, tr("VGA Tileset"), PageVGATS);
    addPageWithType(tiGraphics, tr("EGA Tileset"), PageEGATS);
    addPageWithType(tiGraphics, tr("Mono Tileset"), PageMonoTS);
    addPageWithType(tiGraphics, tr("Background"), PageBackground);
    addPageWithType(tiGraphics, tr("Info Box"), PageInfoBox);
    addPageWithType(tiGraphics, tr("End Graphic"), PageEndGfx);
    addPageWithType(tiGraphics, tr("Digits"), PageDigits);
    pager->expandAll();

    QFont tiFont = tiGeneral->font(0);
    tiFont.setBold(true);
    tiFont.setItalic(true);
    tiGeneral->setFont(0, tiFont);
    tiStrings->setFont(0, tiFont);
    tiGraphics->setFont(0, tiFont);

    auto right = new QWidget(split);
    m_container = new QStackedWidget(right);
    m_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_container->setContentsMargins(0, 0, 0, 0);

    auto acSave = new QAction(ICON("document-save"), tr("&Save Patch"), this);
    auto acWriteExe = new QAction(ICON("document-save-as"), tr("&Write EXE"), this);
    auto acLoad = new QAction(ICON("document-open"), tr("&Load From..."), this);
    auto acReset = new QAction(ICON("edit-clear-list"), tr("&Clear All"), this);
    auto acAbout = new QAction(ICON("help-about"), tr("&About"), this);

    auto tools = new QToolBar(right);
    tools->setIconSize(QSize(22, 22));
    tools->setMovable(false);
    tools->setFloatable(false);
    tools->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    tools->addAction(acSave);
    tools->addAction(acWriteExe);
    tools->addSeparator();
    tools->addAction(acLoad);
    tools->addSeparator();
    tools->addAction(acReset);
    auto tbSpacer = new QWidget(right);
    tbSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tools->addWidget(tbSpacer);
    tools->addAction(acAbout);

    auto rightLayout = new QGridLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addWidget(m_container, 0, 0);
    rightLayout->addWidget(tools, 1, 0);

    setCentralWidget(split);
    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 5);
    split->setContentsMargins(4, 4, 4, 4);
    resize(800, 480);
    split->setSizes(QList<int>{ 200, 600 });

    connect(pager, &QTreeWidget::currentItemChanged, this, &CCHackMain::onChangePage);

    connect(acSave, &QAction::triggered, this, &CCHackMain::onSavePatchAction);
    connect(acWriteExe, &QAction::triggered, this, &CCHackMain::onWriteExeAction);
    connect(acLoad, &QAction::triggered, this, &CCHackMain::onLoadFromAction);
    connect(acReset, &QAction::triggered, this, &CCHackMain::onResetAction);

    connect(acAbout, &QAction::triggered, this, [this] {
        AboutDialog about(QStringLiteral("CCHack"),
                          QPixmap(QStringLiteral(":/icons/sock-48.png")), this);
        about.exec();
    });

    m_defaults.setKnownDefaults();
    m_settings.clearAll();

    m_pages[PageGeneral] = new CCHack::PageGeneral(this);
    m_pages[PageSound] = new CCHack::PageSounds(this);
    m_pages[PageMenus] = new CCHack::PageMenus(this);
    m_pages[PageStory] = new CCHack::PageStoryline(this);
    m_pages[PageScores] = new CCHack::PageScores(this);
    m_pages[PageMisc] = new PlaceholderPage(this);
    m_pages[PageVGATS] = new CCHack::PageBitmap(CCHack::PageBitmap::VgaTileset, this);
    m_pages[PageEGATS] = new CCHack::PageBitmap(CCHack::PageBitmap::EgaTileset, this);
    m_pages[PageMonoTS] = new CCHack::PageBitmap(CCHack::PageBitmap::MonoTileset, this);
    m_pages[PageBackground] = new CCHack::PageBitmap(CCHack::PageBitmap::Background, this);
    m_pages[PageInfoBox] = new CCHack::PageBitmap(CCHack::PageBitmap::InfoBox, this);
    m_pages[PageEndGfx] = new CCHack::PageBitmap(CCHack::PageBitmap::ChipEnd, this);
    m_pages[PageDigits] = new CCHack::PageBitmap(CCHack::PageBitmap::Digits, this);

    for (HackPage* page : m_pages) {
        m_container->addWidget(page);
        page->setValues(&m_settings);
        page->setDefaults(&m_defaults);
    }
}

void CCHackMain::loadFile(const QString& filename)
{
    QFileInfo info(filename);
    if (info.suffix().compare(QLatin1String("exe"), Qt::CaseInsensitive) == 0) {
        loadExecutable(filename);
    } else if (info.suffix().compare(QLatin1String("ccp"), Qt::CaseInsensitive) == 0) {
        loadPatchFile(filename);
    } else {
        QMessageBox::critical(this, tr("Invalid filename"),
                              tr("Unsupported file type for %1").arg(filename));
    }
}

void CCHackMain::loadExecutable(const QString& filename)
{
    HackSettings settings;
    try {
        if (!settings.loadFromExe(filename)) {
            QMessageBox::critical(this, tr("Error loading EXE"),
                                  tr("Could not open %1 for reading").arg(filename));
            return;
        }
    } catch (const ccl::RuntimeError& err) {
        QMessageBox::critical(this, tr("Error loading EXE"),
                              tr("Failed to load %1: %2").arg(filename).arg(err.message()));
        return;
    }

    m_settings = settings;
    for (HackPage* page : m_pages)
        page->setValues(&m_settings);
}

void CCHackMain::loadPatchFile(const QString& filename)
{
    HackSettings settings;
    try {
        if (!settings.loadFromPatch(filename)) {
            QMessageBox::critical(this, tr("Error loading patch"),
                                  tr("Could not open %1 for reading").arg(filename));
            return;
        }
    } catch (const ccl::RuntimeError& err) {
        QMessageBox::critical(this, tr("Error loading patch"),
                              tr("Failed to load %1: %2").arg(filename).arg(err.message()));
        return;
    }

    m_settings = settings;
    for (HackPage* page : m_pages)
        page->setValues(&m_settings);
}

void CCHackMain::onChangePage(QTreeWidgetItem* page, QTreeWidgetItem*)
{
    if (page->childCount() > 0) {
        // Focus the first child item instead of this category item.
        QTimer::singleShot(0, [page] {
            page->treeWidget()->setCurrentItem(page->child(0));
        });
        return;
    }

    const int pageType = page->data(0, Qt::UserRole).toInt();
    m_container->setCurrentIndex(pageType < m_container->count() ? pageType : 0);
}

void CCHackMain::onLoadFromAction()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Load from..."), QString(),
                                tr("Supported Files (*.EXE *.exe *.ccp);;"
                                   "EXE Files (*.EXE *.exe);;"
                                   "CCPatch Files (*.ccp)"));
    if (!filename.isEmpty())
        loadFile(filename);
}

void CCHackMain::onSavePatchAction()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Patch"),
                                QString(), tr("CCHack Patch Files (*.ccp)"));
    if (filename.isEmpty())
        return;

    HackSettings saveSettings;
    for (HackPage* page : m_pages)
        page->saveTo(&saveSettings);

    try {
        if (!saveSettings.writeToPatch(filename)) {
            QMessageBox::critical(this, tr("Error saving patch"),
                                  tr("Could not open %1 for writing").arg(filename));
            return;
        }
    } catch (const ccl::RuntimeError& err) {
        QMessageBox::critical(this, tr("Error saving patch"),
                              tr("Failed to write to %1: %2").arg(filename).arg(err.message()));
        return;
    }

    m_settings = saveSettings;
}

void CCHackMain::onWriteExeAction()
{
    QString exeFilename = QFileDialog::getOpenFileName(this, tr("Write to EXE"),
                                QString(), tr("EXE Files (*.exe)"));
    if (exeFilename.isEmpty())
        return;

    HackSettings saveSettings;
    for (HackPage* page : m_pages)
        page->saveTo(&saveSettings);

    try {
        if (!saveSettings.writeToExe(exeFilename)) {
            QMessageBox::critical(this, tr("Error updating EXE"),
                                  tr("Could not open %1 for writing").arg(exeFilename));
            return;
        }
    } catch (const ccl::RuntimeError& err) {
        QMessageBox::critical(this, tr("Error updating EXE"),
                              tr("Failed to write to %1: %2").arg(exeFilename).arg(err.message()));
        return;
    }

    m_settings = saveSettings;
}

void CCHackMain::onResetAction()
{
    auto response = QMessageBox::question(this, tr("Clear All?"),
                        tr("Are you sure you want to clear all fields and graphics?"));
    if (response == QMessageBox::Yes) {
        m_settings.clearAll();
        for (HackPage* page : m_pages)
            page->setValues(&m_settings);
    }
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("CCTools"));
    QApplication::setApplicationName(QStringLiteral("CCHack"));

    QIcon appicon(QStringLiteral(":/icons/sock-48.png"));
    appicon.addFile(QStringLiteral(":/icons/sock-32.png"));
    appicon.addFile(QStringLiteral(":/icons/sock-24.png"));
    appicon.addFile(QStringLiteral(":/icons/sock-16.png"));
    QApplication::setWindowIcon(appicon);

    CCHackMain mainWin;
    mainWin.show();

    QStringList qtArgs = QApplication::arguments();
    if (qtArgs.size() > 1)
        mainWin.loadFile(qtArgs[1]);

    return QApplication::exec();
}
