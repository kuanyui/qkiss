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

#include "translationmanager.h"
#include <QEvent>


TranslationManager &TranslationManager::instance() {
    static TranslationManager instance;
    return instance;
}

void TranslationManager::registerLanguageChangeCallback(QObject *target, std::function<void ()> langCb) {
    if (!target) return;

    m_updates.append({target, langCb});

    // Auto-cleanup before target is destroyed
    connect(target, &QObject::destroyed, this, [this]() {
        cleanup();
    });

    // Execute immediately
    langCb();
}

void TranslationManager::retranslateAll() {
    cleanup();
    for (const auto& entry : std::as_const(m_updates)) {
        if (!entry.target.isNull()) {
            entry.updateFunc();
        }
    }
}

bool TranslationManager::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateAll();
    }
    return QObject::eventFilter(obj, event);
}

void TranslationManager::cleanup() {
    m_updates.removeIf([](const UpdateEntry& entry) {
        return entry.target.isNull();
    });
}
