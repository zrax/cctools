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
        s_repo->addCustomSearchPath(":/cc2-syntax");
    }
    return s_repo;
}

CC2ScriptEditor::CC2ScriptEditor(QWidget* parent)
    : SyntaxTextEdit(parent)
{
    setTabWidth(4);
    setHighlightCurrentLine(true);

    setTheme((palette().color(QPalette::Base).lightness() < 128)
             ? SyntaxRepo()->defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
             : SyntaxRepo()->defaultTheme(KSyntaxHighlighting::Repository::LightTheme));

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

    auto syntaxDef = SyntaxRepo()->definitionForName("CC2 Game Script");
    if (!syntaxDef.isValid())
        qDebug("Warning: Could not find syntax defintion for .c2g files");
    setSyntax(syntaxDef);
}
