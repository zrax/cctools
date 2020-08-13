/* Copyright (C) 2001-2010 by Madhav Shanbhag,
 * under the GNU General Public License. No warranty. See COPYING for details.
 */

#ifndef CCMETADATA_H
#define CCMETADATA_H


#include <QString>
#include <QColor>

#include <QDomElement>

#include <vector>


namespace CCX {

struct Levelset;

enum Compatibility {
    COMPAT_UNKNOWN,
    COMPAT_NO,
    COMPAT_YES,
};

struct RulesetCompatibility {
    Compatibility eMS, eLynx, ePedantic;

    RulesetCompatibility()
        : eMS(COMPAT_UNKNOWN), eLynx(COMPAT_UNKNOWN), ePedantic(COMPAT_UNKNOWN)
    { }

    void readXML(const QDomElement& elm);
};

enum TextFormat {
    TEXT_PLAIN,
    TEXT_HTML,
};

struct PageProperties {
    TextFormat eFormat;
    Qt::AlignmentFlag align, valign;
    QColor color, bgcolor;

    PageProperties()
        : eFormat(TEXT_PLAIN), align(Qt::AlignLeft), valign(Qt::AlignTop),
          color(Qt::white), bgcolor(Qt::black) { }

    void readXML(const QDomElement& elm);
};

struct Page {
    QString sText;
    PageProperties pageProps;

    void readXML(const QDomElement& elm, const Levelset& levelset);
};

struct Text {
    std::vector<Page> vecPages;
    bool bSeen;

    Text() : bSeen() { }

    void readXML(const QDomElement& elm, const Levelset& levelset);
};

struct Level {
    QString sAuthor;
    RulesetCompatibility ruleCompat;
    Text txtPrologue, txtEpilogue;

    void readXML(const QDomElement& elm, const Levelset& levelset);
};

struct Levelset {
    QString sDescription;
    QString sCopyright;
    QString sAuthor;
    RulesetCompatibility ruleCompat;
    PageProperties pageProps;
    QString sStyleSheet;

    std::vector<Level> vecLevels;

    void readXML(const QDomElement& elm);
    bool readFile(const QString& sFilePath, int nLevels);
    void clear();
};

}

#endif
