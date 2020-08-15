/* Copyright (C) 2001-2010 by Madhav Shanbhag,
 * under the GNU General Public License. No warranty. See COPYING for details.
 */

#include "CCMetaData.h"

#include <QFile>
#include <QDomDocument>
#include <QDomElement>

static bool readElmAttr(const QDomElement& elm, const QString& attr, QString& rValue)
{
    if (!elm.hasAttribute(attr))
        return false;
    rValue = elm.attribute(attr);
    return true;
}

template <typename T, typename ParseFn>
static bool readElmAttr(const QDomElement& elm, const QString& attr,
                        T& rValue, const ParseFn& parsefn)
{
    QString strValue;
    if (!readElmAttr(elm, attr, strValue))
        return false;
    rValue = parsefn(strValue);
    return true;
}

static bool writeElmAttr(QDomElement& elm, const QString& attr, const QString& value)
{
    if (value.isEmpty())
        return false;
    elm.setAttribute(attr, value);
    return true;
}

template <typename T, typename WriteFn>
static bool writeElmAttr(QDomElement& elm, const QString& attr,
                         const T& value, const WriteFn& writefn)
{
    const QString strValue = writefn(value);
    return writeElmAttr(elm, attr, strValue);
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

static inline QString writeHAlign(const Qt::AlignmentFlag& value)
{
    switch (value) {
    case Qt::AlignCenter:
        return QStringLiteral("center");
    case Qt::AlignRight:
        return QStringLiteral("right");
    default:
        return QString();
    }
}

static inline Qt::AlignmentFlag parseVAlign(const QString& s)
{
    return (s == "bottom") ? Qt::AlignBottom
         : (s == "middle") ? Qt::AlignVCenter
         : Qt::AlignTop;
}

static inline QString writeVAlign(const Qt::AlignmentFlag& value)
{
    switch (value) {
    case Qt::AlignBottom:
        return QStringLiteral("bottom");
    case Qt::AlignVCenter:
        return QStringLiteral("middle");
    default:
        return QString();
    }
}

static inline CCX::Compatibility parseCompat(const QString& s)
{
    return (s == "yes") ? CCX::COMPAT_YES
         : (s == "no")  ? CCX::COMPAT_NO
         : CCX::COMPAT_UNKNOWN;
}

static inline QString writeCompat(const CCX::Compatibility& compat)
{
    switch (compat) {
    case CCX::COMPAT_YES:
        return QStringLiteral("yes");
    case CCX::COMPAT_NO:
        return QStringLiteral("no");
    default:
        return QString();
    }
}

static inline CCX::TextFormat parseFormat(const QString& s)
{
    return (s == "html") ? CCX::TEXT_HTML : CCX::TEXT_PLAIN;
}

static inline QString writeFormat(const CCX::TextFormat& format)
{
    return (format == CCX::TEXT_HTML) ? QStringLiteral("html") : QString();
}


void CCX::RulesetCompatibility::copyUnique(const RulesetCompatibility& copyFrom,
                                           const RulesetCompatibility& parent)
{
    if (copyFrom.m_msCompat != parent.m_msCompat)
        m_msCompat = copyFrom.m_msCompat;
    if (copyFrom.m_lynxCompat != parent.m_lynxCompat)
        m_lynxCompat = copyFrom.m_lynxCompat;
    if (copyFrom.m_pedanticCompat != parent.m_pedanticCompat)
        m_pedanticCompat = copyFrom.m_pedanticCompat;
}

void CCX::RulesetCompatibility::readXML(const QDomElement& elm)
{
    readElmAttr(elm, "ms",       m_msCompat,       &parseCompat);
    readElmAttr(elm, "lynx",     m_lynxCompat,     &parseCompat);
    readElmAttr(elm, "pedantic", m_pedanticCompat, &parseCompat);
}

void CCX::RulesetCompatibility::writeXML(QDomElement& elm) const
{
    writeElmAttr(elm, "ms",       m_msCompat,       &writeCompat);
    writeElmAttr(elm, "lynx",     m_lynxCompat,     &writeCompat);
    writeElmAttr(elm, "pedantic", m_pedanticCompat, &writeCompat);
}


void CCX::PageProperties::copyUnique(const PageProperties& copyFrom,
                                     const PageProperties& parent)
{
    if (copyFrom.m_textFormat != parent.m_textFormat)
        m_textFormat = copyFrom.m_textFormat;
    if (copyFrom.m_align != parent.m_align)
        m_align = copyFrom.m_align;
    if (copyFrom.m_valign != parent.m_valign)
        m_valign = copyFrom.m_valign;
    if (copyFrom.m_color != parent.m_color)
        m_color = copyFrom.m_color;
    if (copyFrom.m_bgcolor != parent.m_bgcolor)
        m_bgcolor = copyFrom.m_bgcolor;
}

void CCX::PageProperties::readXML(const QDomElement& elm)
{
    readElmAttr(elm, "format",  m_textFormat, &parseFormat);
    readElmAttr(elm, "align",   m_align,      &parseHAlign);
    readElmAttr(elm, "valign",  m_valign,     &parseVAlign);
    readElmAttr(elm, "color",   m_color,      &parseColor);
    readElmAttr(elm, "bgcolor", m_bgcolor,    &parseColor);
}

void CCX::PageProperties::writeXML(QDomElement& elm) const
{
    writeElmAttr(elm, "format",  m_textFormat,  &writeFormat);
    writeElmAttr(elm, "align",   m_align,       &writeHAlign);
    writeElmAttr(elm, "valign",  m_valign,      &writeVAlign);
    writeElmAttr(elm, "color",   m_color, [](const QColor& color) -> QString {
        return (color == Qt::white) ? QString() : color.name();
    });
    writeElmAttr(elm, "bgcolor", m_bgcolor, [](const QColor& color) -> QString {
        return (color == Qt::black) ? QString() : color.name();
    });
}


void CCX::Page::readXML(const QDomElement& elm, const Levelset& levelset)
{
    m_text = elm.text();

    m_pageProps = levelset.m_pageProps;
    m_pageProps.readXML(elm);
}

void CCX::Page::writeXML(QDomDocument& doc, QDomElement& elm,
                         const Levelset& levelset) const
{
    QDomCDATASection textNode = doc.createCDATASection(m_text);
    elm.appendChild(textNode);

    PageProperties writeProps;
    writeProps.copyUnique(m_pageProps, levelset.m_pageProps);
    writeProps.writeXML(elm);
}


void CCX::Text::readXML(const QDomElement& elm, const Levelset& levelset)
{
    QDomNodeList lstElmPages = elm.elementsByTagName("page");
    m_pages.resize(lstElmPages.length());
    for (int i = 0; i < lstElmPages.length(); ++i) {
        QDomElement elmPage = lstElmPages.item(i).toElement();
        m_pages[i].readXML(elmPage, levelset);
    }
}

void CCX::Text::writeXML(QDomDocument& doc, QDomElement& elm,
                         const Levelset& levelset) const
{
    for (const Page& page : m_pages) {
        QDomElement elmPage = doc.createElement("page");
        page.writeXML(doc, elmPage, levelset);
        elm.appendChild(elmPage);
    }
}


void CCX::Level::readXML(const QDomElement& elm, const Levelset& levelset)
{
    m_author = levelset.m_author;
    readElmAttr(elm, "author", m_author);

    m_ruleCompat = levelset.m_ruleCompat;
    m_ruleCompat.readXML(elm);

    QDomNodeList lstElm;
    lstElm = elm.elementsByTagName("prologue");
    if (lstElm.length() != 0)
        m_prologue.readXML(lstElm.item(0).toElement(), levelset);
    lstElm = elm.elementsByTagName("epilogue");
    if (lstElm.length() != 0)
        m_epilogue.readXML(lstElm.item(0).toElement(), levelset);
}

void CCX::Level::writeXML(QDomDocument& doc, QDomElement& elm,
                          const Levelset& levelset) const
{
    if (m_author != levelset.m_author)
        writeElmAttr(elm, "author", m_author);

    RulesetCompatibility writeCompat;
    writeCompat.copyUnique(m_ruleCompat, levelset.m_ruleCompat);
    writeCompat.writeXML(elm);

    if (!m_prologue.m_pages.empty()) {
        QDomElement elmPrologue = doc.createElement("prologue");
        m_prologue.writeXML(doc, elmPrologue, levelset);
        elm.appendChild(elmPrologue);
    }
    if (!m_epilogue.m_pages.empty()) {
        QDomElement elmEpilogue = doc.createElement("epilogue");
        m_epilogue.writeXML(doc, elmEpilogue, levelset);
        elm.appendChild(elmEpilogue);
    }
}


void CCX::Levelset::readXML(const QDomElement& elm)
{
    readElmAttr(elm, "description", m_description);
    readElmAttr(elm, "copyright",   m_copyright);
    readElmAttr(elm, "author",      m_author);

    m_ruleCompat.readXML(elm);
    m_pageProps.readXML(elm);

    for (Level& rLevel : m_levels) {
        rLevel.m_author = m_author;
        rLevel.m_ruleCompat = m_ruleCompat;
    }

    static const auto parseLevelNum = [](const QString& s) -> int {
        return s.toInt() - 1;
    };

    QDomNodeList lstElmLevels = elm.elementsByTagName("level");
    for (int i = 0; i < lstElmLevels.length(); ++i) {
        QDomElement elmLevel = lstElmLevels.item(i).toElement();
        int levelNum = -1;
        if (!readElmAttr(elmLevel, "number", levelNum, parseLevelNum))
            continue;
        if (levelNum < 0 || levelNum >= int(m_levels.size()))
            continue;
        m_levels[levelNum].readXML(elmLevel, *this);
    }

    QDomNodeList lstElmStyle = elm.elementsByTagName("style");
    if (lstElmStyle.length() != 0) {
        QDomElement elmStyle = lstElmStyle.item(0).toElement();
        if (elmStyle.parentNode() == elm)
            m_styleSheet = elmStyle.text();
    }
}

bool CCX::Levelset::readFile(const QString& filePath, int nLevels)
{
    clear();

    m_levels.resize(nLevels);

    QFile file(filePath);
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

void CCX::Levelset::writeXML(QDomDocument& doc, QDomElement& elm) const
{
    writeElmAttr(elm, "description", m_description);
    writeElmAttr(elm, "copyright",   m_copyright);
    writeElmAttr(elm, "author",      m_author);

    m_ruleCompat.writeXML(elm);
    m_pageProps.writeXML(elm);

    static const auto writeLevelNum = [](const int& num) -> QString {
        return (num >= 0) ? QString::number(num + 1) : QString();
    };

    for (int i = 0; i < int(m_levels.size()); ++i) {
        QDomElement elmLevel = doc.createElement("level");
        writeElmAttr(elmLevel, "number", i, writeLevelNum);
        m_levels[i].writeXML(doc, elmLevel, *this);
        elm.appendChild(elmLevel);
    }

    if (!m_styleSheet.isEmpty()) {
        QDomElement elmStyle = doc.createElement("style");
        QDomText styleText = doc.createTextNode(m_styleSheet);
        elmStyle.appendChild(styleText);
        elm.appendChild(elmStyle);
    }
}

bool CCX::Levelset::writeFile(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QDomDocument doc;
    QDomElement elmRoot = doc.createElement("levelset");
    writeXML(doc, elmRoot);
    doc.appendChild(elmRoot);

    file.write(doc.toByteArray());
    file.close();

    return true;
}

void CCX::Levelset::clear()
{
    *this = Levelset();
}
