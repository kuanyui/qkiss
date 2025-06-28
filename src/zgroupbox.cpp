
#include "zgroupbox.h"


ZGroupBox::ZGroupBox(const QString &title, const QIcon &icon, QWidget *parent)
    : QGroupBox(title, parent)
{
    m_iconLabel = new QLabel();
    m_iconLabel->setPixmap(icon.pixmap(48, 48));
    m_iconLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setAlignment(Qt::AlignTop);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addWidget(m_iconLabel);
    mainLayout->addLayout(m_contentLayout);
    mainLayout->setSpacing(10);

    setLayout(mainLayout);
}

void ZGroupBox::setContentLayout(QLayout *layout) {
    QLayoutItem *child;
    while ((child = m_contentLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    QWidget *container = new QWidget;
    container->setLayout(layout);
    m_contentLayout->addWidget(container);
}
