#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

#include <QByteArray>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <xxhash.hpp>

namespace Ui {
class ResultDialog;
}

class ResultDialog : public QDialog {
    Q_OBJECT

   public:
    explicit ResultDialog(QWidget *parent = nullptr,
                          QDir const *origin = nullptr);
    ~ResultDialog();

    void appendData(std::map<xxh::hash64_t, std::vector<std::string>> *hashes,
                    qint64 fileSize);

   private slots:
    void on_returnButton_clicked();

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

   private:
    Ui::ResultDialog *ui;
    QDir const *root;
};

#endif  // RESULTDIALOG_H
