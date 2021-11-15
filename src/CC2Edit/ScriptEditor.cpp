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

#include "ScriptEditor.h"

#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Theme>

static KSyntaxHighlighting::Repository* SyntaxRepo()
{
    static KSyntaxHighlighting::Repository* s_repo = SyntaxTextEdit::syntaxRepo();
    static bool s_firstInit = true;
    if (s_firstInit) {
        s_firstInit = false;
        s_repo->addCustomSearchPath(QStringLiteral(":/cc2-syntax"));
    }
    return s_repo;
}

CC2ScriptEditor::CC2ScriptEditor(Mode mode, QWidget* parent)
    : SyntaxTextEdit(parent)
{
    setTabWidth(4);

#if defined(_WIN32)
    // Included in Vista or Office 2007, both of which are "Old Enough" (2018)
    static const QString defaultFontName = QStringLiteral("Consolas");
    static const int defaultFontSize = 10;
#elif defined(__APPLE__)
    static const QString defaultFontName = QStringLiteral("Menlo");
    static const int defaultFontSize = 12;
#else
    static const QString defaultFontName = QStringLiteral("Monospace");
    static const int defaultFontSize = 10;
#endif
    QFont editFont(defaultFontName, defaultFontSize);
    editFont.setFixedPitch(true);
    setDefaultFont(editFont);

    QString syntaxName;
    switch (mode) {
    case ScriptMode:
        setHighlightCurrentLine(true);
        setShowLineNumbers(true);
        syntaxName = QStringLiteral("CC2 Game Script");
        break;
    case NotesMode:
        setShowFolding(true);
        syntaxName = QStringLiteral("CC2 Game Notes");
        break;
    case PlainMode:
        // Just for visual consistency with NotesMode
        setShowFolding(true);
        break;
    default:
        // Missing mode case
        Q_ASSERT(false);
    }

    auto syntaxDef = SyntaxRepo()->definitionForName(syntaxName);
    if (!syntaxDef.isValid())
        qDebug("Warning: Could not find syntax defintion for \"%s\"", qPrintable(syntaxName));
    setSyntax(syntaxDef);
}

bool CC2ScriptEditor::canUndo() const
{
    return document()->isUndoAvailable();
}

bool CC2ScriptEditor::canRedo() const
{
    return document()->isRedoAvailable();
}

bool CC2ScriptEditor::isModified() const
{
    return document()->isModified();
}

void CC2ScriptEditor::setClean()
{
    document()->setModified(false);
}
