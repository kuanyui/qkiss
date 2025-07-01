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
    explicit ZGroupBox(const QString &title, QWidget *parent = nullptr);
    void setIcon(const QIcon &icon);
    void setContentLayout(QLayout *layout);
    template<typename T = QLayout> T *contentLayout();

private:
    QLabel *m_iconLabel;
    QHBoxLayout *m_rootLayout;
    QLayout *m_contentLayout;
    QIcon m_icon;
    int m_iconSize = 48;
};

#endif // ZGROUPBOX_H
