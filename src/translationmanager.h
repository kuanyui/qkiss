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

#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>
#include <QPointer>

class TranslationManager : public QObject
{
    Q_OBJECT
    struct UpdateEntry {
        QPointer<QObject> target;
        std::function<void()> updateFunc;
    };
public:
    static TranslationManager& instance();

    /// - Universal registration - works for ANY QObject
    /// - Callback will also be called 1 time when register.
    void registerLanguageChangeCallback(QObject* target, std::function<void()> langCb);
    void retranslateAll();

protected:
    /// So that this class can be installed via `qApp->installEventFilter(&TranslationManager::instance());`
    bool eventFilter(QObject* obj, QEvent* event) override;

signals:
private:
    TranslationManager() = default;
    Q_DISABLE_COPY_MOVE(TranslationManager)

    QList<UpdateEntry> m_updates;
    void cleanup();
};

// Convenience macro (optional)
#define TR_REGISTER(target, ...) \
    TranslationManager::instance().registerLanguageChangeCallback(target, [=]() { __VA_ARGS__; })

#endif // TRANSLATIONMANAGER_H


