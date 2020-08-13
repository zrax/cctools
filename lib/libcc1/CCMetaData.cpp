/* Copyright (C) 2001-2010 by Madhav Shanbhag,
 * under the GNU General Public License. No warranty. See COPYING for details.
 */

#include "CCMetaData.h"

#include <QFile>
#include <QDomDocument>
#include <QDomElement>

template <typename T>
static bool readElmAttr(const QDomElement& elm, const QString& attr,
                        T (*parsefn)(const QString&), T& rValue)
{
    if (!elm.hasAttribute(attr))
       return false;
    rValue = parsefn(elm.attribute(attr));
    return true;
}

static inline QString parseString(const QString& s)
{
    return s;
}

static inline int parseInt(const QString& s)
{
    return s.toInt();
}

static inline QColor parseColor(const QString& s)
{
    return QColor(s);
}

static inline Qt::AlignmentFlag parseHAlign(const QString& s)
{
    return (s == "right")  ? Qt::AlignRight
         : (s == "center") ? Qt::AlignHCenter
         : Qt::AlignLeft;
}

static inline Qt::AlignmentFlag parseVAlign(const QString& s)
{
    return (s == "bottom") ? Qt::AlignBottom
         : (s == "middle") ? Qt::AlignVCenter
         : Qt::AlignTop;
}

static inline CCX::Compatibility parseCompat(const QString& s)
{
    return (s == "yes") ? CCX::COMPAT_YES
         : (s == "no")  ? CCX::COMPAT_NO
         : CCX::COMPAT_UNKNOWN;
}

static CCX::TextFormat parseFormat(const QString& s)
{
    return (s == "html") ? CCX::TEXT_HTML : CCX::TEXT_PLAIN;
}


void CCX::RulesetCompatibility::readXML(const QDomElement& elm)
{
    readElmAttr(elm, "ms",       &parseCompat, eMS);
    readElmAttr(elm, "lynx",     &parseCompat, eLynx);
    readElmAttr(elm, "pedantic", &parseCompat, ePedantic);
}


void CCX::PageProperties::readXML(const QDomElement& elm)
{
    readElmAttr(elm, "format",  &parseFormat, eFormat);
    readElmAttr(elm, "align",   &parseHAlign, align);
    readElmAttr(elm, "valign",  &parseVAlign, valign);
    readElmAttr(elm, "color",   &parseColor , color);
    readElmAttr(elm, "bgcolor", &parseColor , bgcolor);
}


void CCX::Page::readXML(const QDomElement& elm, const Levelset& levelset)
{
    sText = elm.text();

    pageProps = levelset.pageProps;
    pageProps.readXML(elm);
}


void CCX::Text::readXML(const QDomElement& elm, const Levelset& levelset)
{
    vecPages.clear();
    QDomNodeList lstElmPages = elm.elementsByTagName("page");
    vecPages.resize(lstElmPages.length());
    for (int i = 0; i < lstElmPages.length(); ++i) {
        QDomElement elmPage = lstElmPages.item(i).toElement();
        vecPages[i].readXML(elmPage, levelset);
    }
}


void CCX::Level::readXML(const QDomElement& elm, const Levelset& levelset)
{
    sAuthor = levelset.sAuthor;
    readElmAttr(elm, "author", &parseString, sAuthor);

    ruleCompat = levelset.ruleCompat;
    ruleCompat.readXML(elm);

    QDomNodeList lstElm;
    lstElm = elm.elementsByTagName("prologue");
    if (lstElm.length() != 0)
        txtPrologue.readXML(lstElm.item(0).toElement(), levelset);
    lstElm = elm.elementsByTagName("epilogue");
    if (lstElm.length() != 0)
        txtEpilogue.readXML(lstElm.item(0).toElement(), levelset);
}


void CCX::Levelset::readXML(const QDomElement& elm)
{
    readElmAttr(elm, "description", &parseString, sDescription);
    readElmAttr(elm, "copyright",   &parseString, sCopyright);
    readElmAttr(elm, "author",      &parseString, sAuthor);

    ruleCompat.readXML(elm);
    pageProps.readXML(elm);

    for (Level& rLevel : vecLevels) {
        rLevel.sAuthor = sAuthor;
        rLevel.ruleCompat = ruleCompat;
    }

    QDomNodeList lstElmLevels = elm.elementsByTagName("level");
    for (int i = 0; i < lstElmLevels.length(); ++i) {
        QDomElement elmLevel = lstElmLevels.item(i).toElement();
        int nNumber = 0;
        if (!readElmAttr(elmLevel, "number", &parseInt, nNumber))
            continue;
        if (nNumber < 1 || nNumber >= int(vecLevels.size()))
            continue;
        vecLevels[nNumber].readXML(elmLevel, *this);
    }

    QDomNodeList lstElmStyle = elm.elementsByTagName("style");
    if (lstElmStyle.length() != 0) {
        QDomElement elmStyle = lstElmStyle.item(0).toElement();
        if (elmStyle.parentNode() == elm)
            sStyleSheet = elmStyle.text();
    }
}


bool CCX::Levelset::readFile(const QString& sFilePath, int nLevels)
{
    clear();

    vecLevels.resize(1 + nLevels);

    QFile file(sFilePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QDomDocument doc;
    if (!doc.setContent(&file))
        return false;

    QDomElement elmRoot = doc.documentElement();
    if (elmRoot.tagName() != "levelset")
        return false;

    readXML(elmRoot);
    file.close();

    return true;
}

void CCX::Levelset::clear()
{
    *this = Levelset();
}
