
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
