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

#ifndef _PLAYSETTINGS_H
#define _PLAYSETTINGS_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QComboBox>
#include <QAction>
#include <QSettings>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    void refreshTools();

    static void CheckTools(QSettings& settings);
    static QIcon IconForTool(const QString& iconName);

private:
    QLineEdit* m_winePath;
    QLineEdit* m_msccPath;
    QLineEdit* m_tworldPath;
    QComboBox* m_defaultGame;
    QCheckBox* m_useCCPatch;
    QCheckBox* m_cheatIgnorePasswords;
    QCheckBox* m_cheatAlwaysFirstTry;
    QListWidget* m_toolsList;

    enum {
        ActionEditTool, ActionAddTool, ActionDelTool,
        ActionToolUp, ActionToolDown,
        NUM_ACTIONS
    };
    QAction* m_actions[NUM_ACTIONS];

    QStringList m_tools;
    QStringList m_toolIcons;
    QStringList m_toolPaths;

private slots:
    void onSaveSettings();
    void onBrowseWine();
    void onBrowseChips();
    void onBrowseTWorld();
    void onSelectTool(int);

    void onEditTool();
    void onAddTool();
    void onDelTool();
    void onToolUp();
    void onToolDown();
};


class ConfigToolDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigToolDialog(QWidget* parent = nullptr);

    void setName(const QString& name) { m_name->setText(name); }
    void setPath(const QString& path) { m_path->setText(path); }
    void setArgs(const QString& args) { m_args->setText(args); }
    void setIcon(const QString& icon);

    QString name() const { return m_name->text(); }
    QString path() const { return m_path->text(); }
    QString args() const { return m_args->text(); }
    QString icon() const;

private:
    QLineEdit* m_name;
    QLineEdit* m_path;
    QLineEdit* m_args;
    QComboBox* m_icon;

private slots:
    void onBrowseTool();
};

#endif
