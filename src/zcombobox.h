#ifndef ZCOMBOBOX_H
#define ZCOMBOBOX_H

#include <QWidget>
#include <QComboBox>

class ZComboBox : public QComboBox
{
public:
    explicit ZComboBox(QWidget *parent = nullptr);
    void setCurrentIndexByUserData(const QVariant& targetData);
};

#endif // ZCOMBOBOX_H
