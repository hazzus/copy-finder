#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

#include <QByteArray>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QMessageBox>
#include <QTreeWidgetItem>

namespace Ui {
class ResultDialog;
}

class ResultDialog : public QDialog {
    Q_OBJECT

   public:
    explicit ResultDialog(QWidget *parent = nullptr,
                          QMap<QByteArray, QVector<QString>> *copies = nullptr,
                          QDir const *origin = nullptr);
    ~ResultDialog();

   private slots:
    void on_returnButton_clicked();

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

   private:
    Ui::ResultDialog *ui;
    QDir const *root;
};

#endif  // RESULTDIALOG_H
