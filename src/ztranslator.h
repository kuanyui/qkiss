/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2025 ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef ZTRANSLATOR_H
#define ZTRANSLATOR_H

#include <QTranslator>
#include <QHash>

/** Fallback of built-in widgets (ex: context menu) & languages against Qt official language pack.*/
class ZTranslator : public QTranslator
{
public:
    explicit ZTranslator(QObject *parent = nullptr);
    explicit ZTranslator(const QString &lang, QObject *parent = nullptr);
    bool setLang(const QString &lang);
    QString translate(const char *context, const char *sourceText, const char *disambiguation = nullptr, int n = -1) const override;

private:
    void initExtraTranslations();
    QString m_curLang = "en-US";
    bool m_extraTranslationsInitialized = false;
    QHash<QString, QHash<QString, QString>> m_extraTranslations;
};

#endif // ZTRANSLATOR_H
