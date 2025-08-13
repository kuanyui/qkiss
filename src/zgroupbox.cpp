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


#include "zgroupbox.h"


ZGroupBox::ZGroupBox(const QString &title, const QIcon &icon, QWidget *parent)
    : QGroupBox(title, parent)
{
    m_iconLabel = new QLabel();
    m_iconLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setIcon(icon);

    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setAlignment(Qt::AlignTop);

    m_rootLayout = new QHBoxLayout();
    m_rootLayout->addWidget(m_iconLabel);
    m_rootLayout->addLayout(m_contentLayout);

    setLayout(m_rootLayout);
}

ZGroupBox::ZGroupBox(const QString &title, QWidget *parent) :
    ZGroupBox(title, QIcon(), parent)
{

}

void ZGroupBox::setIcon(const QIcon &icon)
{
    m_icon = icon;
    if (m_icon.isNull()) {
        m_iconLabel->hide();
    } else {
        m_iconLabel->setPixmap(m_icon.pixmap(m_iconSize, m_iconSize));
    }
}

void ZGroupBox::setContentLayout(QLayout *layout) {
    QLayoutItem *oldLayout = m_contentLayout->takeAt(1);
    if (oldLayout != nullptr) {
        delete oldLayout->layout();    // Delete the QLayout in the QLayoutItem
        delete oldLayout;              // Delete QLayoutItem
        m_contentLayout = nullptr;
    }
    m_rootLayout->addLayout(layout);
}

template<typename T>
T *ZGroupBox::contentLayout()
{
    static_assert(std::is_base_of<QLayout, T>::value, "T must be a pointer to a subclass of QLayout");
    T *castedLayout = dynamic_cast<T *>(m_contentLayout);
    if (castedLayout == nullptr) {
        throw std::runtime_error("[To Developer] contentLayout<T>() dynamic_cast failed.");
    }
    return castedLayout;
}
