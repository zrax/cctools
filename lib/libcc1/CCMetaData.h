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
    Compatibility m_msCompat, m_lynxCompat, m_pedanticCompat;

    RulesetCompatibility()
        : m_msCompat(), m_lynxCompat(), m_pedanticCompat()
    { }

    void copyUnique(const RulesetCompatibility& copyFrom,
                    const RulesetCompatibility& parent);

    void readXML(const QDomElement& elm);
    void writeXML(QDomElement& elm) const;
};

enum TextFormat {
    TEXT_PLAIN,
    TEXT_HTML,
};

struct PageProperties {
    TextFormat m_textFormat;
    Qt::AlignmentFlag m_align, m_valign;
    QColor m_color, m_bgcolor;

    PageProperties()
        : m_textFormat(TEXT_PLAIN), m_align(Qt::AlignLeft), m_valign(Qt::AlignTop),
          m_color(Qt::white), m_bgcolor(Qt::black) { }

    void copyUnique(const PageProperties& copyFrom, const PageProperties& parent);

    void readXML(const QDomElement& elm);
    void writeXML(QDomElement& elm) const;
};

struct Page {
    QString m_text;
    PageProperties m_pageProps;

    void readXML(const QDomElement& elm, const Levelset& levelset);
    void writeXML(QDomDocument& doc, QDomElement& elm, const Levelset& levelset) const;
};

struct Text {
    std::vector<Page> m_pages;

    void readXML(const QDomElement& elm, const Levelset& levelset);
    void writeXML(QDomDocument& doc, QDomElement& elm, const Levelset& levelset) const;
};

struct Level {
    QString m_author;
    RulesetCompatibility m_ruleCompat;
    Text m_prologue, m_epilogue;

    void readXML(const QDomElement& elm, const Levelset& levelset);
    void writeXML(QDomDocument& doc, QDomElement& elm, const Levelset& levelset) const;
};

struct Levelset {
    QString m_description;
    QString m_copyright;
    QString m_author;
    RulesetCompatibility m_ruleCompat;
    PageProperties m_pageProps;
    QString m_styleSheet;
    std::vector<Level> m_levels;

    void readXML(const QDomElement& elm);
    bool readFile(const QString& filePath, int nLevels);

    void writeXML(QDomDocument& doc, QDomElement& elm) const;
    bool writeFile(const QString& filePath) const;

    void clear();
};

}

#endif
