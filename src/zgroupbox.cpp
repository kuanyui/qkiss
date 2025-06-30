
#include "zgroupbox.h"


ZGroupBox::ZGroupBox(const QString &title, const QIcon &icon, QWidget *parent)
    : QGroupBox(title, parent)
{
    m_iconLabel = new QLabel();
    m_iconLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setIcon(icon);

    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setAlignment(Qt::AlignTop);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addWidget(m_iconLabel);
    mainLayout->addLayout(m_contentLayout);

    setLayout(mainLayout);
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
    QLayoutItem *child;
    while ((child = m_contentLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    QWidget *container = new QWidget();
    container->setLayout(layout);

    m_contentLayout->addWidget(container);
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
