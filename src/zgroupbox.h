#ifndef ZGROUPBOX_H
#define ZGROUPBOX_H

#include <QGroupBox>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ZGroupBox : public QGroupBox
{
public:
    explicit ZGroupBox(const QString &title, const QIcon &icon, QWidget *parent = nullptr);

    void setContentLayout(QLayout *layout);

private:
    QLabel *m_iconLabel;
    QVBoxLayout *m_contentLayout;
};

#endif // ZGROUPBOX_H
