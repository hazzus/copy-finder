#include "resultdialog.h"
#include "ui_resultdialog.h"

#include <QDebug>

class FileTreeWidgetItem : public QTreeWidgetItem {
   public:
    FileTreeWidgetItem(QTreeWidget* parent) : QTreeWidgetItem(parent) {}

   private:
    bool operator<(QTreeWidgetItem const& other) const {
        int column = treeWidget()->sortColumn();
        if (column == 1)
            return text(column).toInt() < other.text(column).toInt();
        else
            return text(column) < other.text(column);
    }
};

ResultDialog::ResultDialog(QWidget* parent,
                           QMap<QByteArray, QVector<QString>>* copies,
                           QDir const* root)
    : QDialog(parent), ui(new Ui::ResultDialog), root(root) {
    ui->setupUi(this);
    ui->treeWidget->clear();
    for (auto copyGroup : copies->keys()) {
        if (copies->value(copyGroup).size() == 1) continue;
        QTreeWidgetItem* item = new FileTreeWidgetItem(ui->treeWidget);

        item->setText(0, QString(copyGroup.toHex()).left(6));
        item->setText(1, QString::number(copies->value(copyGroup).size()));
        for (auto file : copies->value(copyGroup)) {
            QTreeWidgetItem* subItem = new QTreeWidgetItem(item);
            subItem->setText(0, root->relativeFilePath(file));
            item->addChild(subItem);
        }
        ui->treeWidget->addTopLevelItem(item);
    }
    ui->treeWidget->setSortingEnabled(true);
    ui->treeWidget->sortItems(1, Qt::DescendingOrder);
}

ResultDialog::~ResultDialog() { delete ui; }

void ResultDialog::on_returnButton_clicked() { close(); }

void ResultDialog::on_treeWidget_itemClicked(QTreeWidgetItem* item,
                                             int column) {
    QTreeWidgetItem* parent = item->parent();
    if (column != 0 || !parent) return;
    QMessageBox question;
    question.setText(
        QString("Do you want to delete %1?").arg(item->text(column)));
    question.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    question.setDefaultButton(QMessageBox::Cancel);
    int ret = question.exec();
    if (ret == QMessageBox::Ok) {
        QFile(root->absoluteFilePath(item->text(0))).remove();
        parent->removeChild(item);
        parent->setText(1, QString::number(parent->text(1).toInt() - 1));
        if (parent->childCount() == 1) {
            ui->treeWidget->invisibleRootItem()->removeChild(parent);
        }
    }
}
