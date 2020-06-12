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
#include <QSplitter>
#include <QToolBar>
#include <QAction>
#include <QGridLayout>
#include "PageGeneral.h"
#include "About.h"

enum PageType {
    PageNothing = 0,
    PageGeneral = QTreeWidgetItem::UserType, PageSound,
    PageMenus, PageStory, PageEndLevel, PageEndGame, PageMisc,
    PageVGATS, PageEGATS, PageMonoTS, PageBackground, PageEndGfx, PageDigits,
};

CCHackMain::CCHackMain(QWidget* parent)
    : QMainWindow(parent), m_page(0)
{
    setWindowTitle("CCHack 2.1");
    QIcon appicon(":/icons/sock-48.png");
    appicon.addFile(":/icons/sock-32.png");
    appicon.addFile(":/icons/sock-24.png");
    appicon.addFile(":/icons/sock-16.png");
    setWindowIcon(appicon);

    QSplitter* split = new QSplitter(this);
    QTreeWidget* pager = new QTreeWidget(split);
    pager->setHeaderHidden(true);
    pager->setRootIsDecorated(false);
    pager->setSelectionMode(QAbstractItemView::SingleSelection);
    pager->setItemsExpandable(false);

    // TODO: page graphics
    QTreeWidgetItem* tiGeneral = new QTreeWidgetItem(pager, QStringList(tr("General")), PageNothing);
    new QTreeWidgetItem(tiGeneral, QStringList(tr("General")), PageGeneral);
    new QTreeWidgetItem(tiGeneral, QStringList(tr("Sounds & Music")), PageSound);
    QTreeWidgetItem* tiStrings = new QTreeWidgetItem(pager, QStringList(tr("Strings")), PageNothing);
    new QTreeWidgetItem(tiStrings, QStringList(tr("Menus")), PageMenus);
    new QTreeWidgetItem(tiStrings, QStringList(tr("Storyline")), PageStory);
    new QTreeWidgetItem(tiStrings, QStringList(tr("After Level")), PageEndLevel);
    new QTreeWidgetItem(tiStrings, QStringList(tr("End Game")), PageEndGame);
    new QTreeWidgetItem(tiStrings, QStringList(tr("Miscellaneous")), PageMisc);
    QTreeWidgetItem* tiGraphics = new QTreeWidgetItem(pager, QStringList(tr("Graphics")), PageNothing);
    new QTreeWidgetItem(tiGraphics, QStringList(tr("VGA Tileset")), PageVGATS);
    new QTreeWidgetItem(tiGraphics, QStringList(tr("EGA Tileset")), PageEGATS);
    new QTreeWidgetItem(tiGraphics, QStringList(tr("Mono Tileset")), PageMonoTS);
    new QTreeWidgetItem(tiGraphics, QStringList(tr("Background")), PageBackground);
    new QTreeWidgetItem(tiGraphics, QStringList(tr("Endgame")), PageEndGfx);
    new QTreeWidgetItem(tiGraphics, QStringList(tr("Digits")), PageDigits);
    pager->expandAll();

    QFont tiFont = tiGeneral->font(0);
    tiFont.setBold(true);
    tiFont.setItalic(true);
    tiGeneral->setFont(0, tiFont);
    tiStrings->setFont(0, tiFont);
    tiGraphics->setFont(0, tiFont);

    QWidget* right = new QWidget(split);
    m_container = new QWidget(right);
    m_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_container->setContentsMargins(0, 0, 0, 0);
    new QGridLayout(m_container);

    QAction* acSave = new QAction(QIcon(":/res/document-save.png"), tr("&Save Patch"), this);
    QAction* acLoad = new QAction(QIcon(":/res/document-open.png"), tr("&Load Patch"), this);
    QAction* acWriteExe = new QAction(QIcon(":/res/document-save-as.png"), tr("&Write EXE"), this);
    QAction* acReadExe = new QAction(QIcon(":/res/document-open.png"), tr("&Read EXE"), this);
    QAction* acAbout = new QAction(QIcon(":/res/help-about.png"), tr("&About"), this);

    QToolBar* tools = new QToolBar(right);
    tools->setIconSize(QSize(22, 22));
    tools->setMovable(false);
    tools->setFloatable(false);
    tools->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    tools->addAction(acSave);
    tools->addAction(acLoad);
    tools->addSeparator();
    tools->addAction(acWriteExe);
    tools->addAction(acReadExe);
    QWidget* tbSpacer = new QWidget(right);
    tbSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tools->addWidget(tbSpacer);
    tools->addAction(acAbout);

    QGridLayout* rightLayout = new QGridLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addWidget(m_container, 0, 0);
    rightLayout->addWidget(tools, 1, 0);

    setCentralWidget(split);
    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 5);
    split->setContentsMargins(4, 4, 4, 4);
    resize(800, 480);
    split->setSizes(QList<int>() << 200 << 600);

    connect(pager, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(onChangePage(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(acAbout, &QAction::triggered, this, [this] {
        AboutDialog about(this);
        about.exec();
    });

    m_defaults.setKnownDefaults();
    m_settings.clearAll();
}

void CCHackMain::loadFile(const QString& filename)
{
    //TODO
}

void CCHackMain::onChangePage(QTreeWidgetItem* page, QTreeWidgetItem*)
{
    if (page->type() == PageNothing) {
        //TODO: Actually show the selection correctly...  If Qt will let us
        page = page->child(0);
    }

    // Remove the old page contents
    QGridLayout* layout = (QGridLayout*)m_container->layout();
    while (QLayoutItem* item = layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    delete m_page;

    switch (page->type()) {
    case PageGeneral:
        m_page = new CCHack::PageGeneral(m_container);
        break;
    default:
        m_page = 0;
    }

    if (m_page) {
        m_page->setDefaults(&m_defaults);
        m_page->setValues(&m_settings);
    }
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    CCHackMain mainWin;
    mainWin.show();

    if (argc > 1)
        mainWin.loadFile(argv[1]);
    return app.exec();
}
