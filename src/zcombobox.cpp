#include "zcombobox.h"



ZComboBox::ZComboBox(QWidget *parent) :
    QComboBox(parent)
{

}

void ZComboBox::setCurrentIndexByUserData(const QVariant &targetData) {
    for (int i = 0; i < count(); ++i) {
        if (itemData(i) == targetData) {
            setCurrentIndex(i);
            break;
        }
    }
}

