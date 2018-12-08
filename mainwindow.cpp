#include "mainwindow.h"
#include "ui_mainwindow.h"

// Only for beta
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), hashThread(nullptr) {
    ui->setupUi(this);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setValue(0);
    ui->treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_selectDirectory_clicked() {
    QString directory = QFileDialog::getExistingDirectory(this);
    ui->directoryPath->clear();
    ui->directoryPath->insert(directory);
    ui->progressBar->setValue(0);
}

QString createRepresentation(qint64 fileSize) {
    if (fileSize / 1024 == 0) {
        return QString("Files of size %1 bytes").arg(fileSize);
    } else if (fileSize / 1048576 == 0) {
        return QString("Files of size %1 Kbytes").arg(round(fileSize / 1024.0 * 100) / 100);
    } else if (fileSize / 1073741824 == 0) {
        return QString("Files of size %1 Mbytes").arg(round(fileSize / 1048576.0 * 100) / 100);
    } else {
        return QString("Files of size %1 Gbytes").arg(round(fileSize / 1073741824.0 * 100) / 100);
    }
}

class QFileHashTreeWidgetItem : public QTreeWidgetItem {
   public:
    explicit QFileHashTreeWidgetItem(QTreeWidgetItem* parent) : QTreeWidgetItem(parent) {}
   private:
    virtual bool operator<(QTreeWidgetItem const& other) const;
};

bool QFileHashTreeWidgetItem::operator<(QTreeWidgetItem const& other) const {
    int column = treeWidget()->sortColumn();
    if (column == 1)
        return text(column).toInt() < other.text(column).toInt();
    else
        return text(column) < other.text(column);
}

void MainWindow::addItemToTree(
    qint64 fileSize,
    std::map<xxh::hash64_t, std::vector<std::string>> const& hashes,
    QDir const& dir) {
    QTreeWidgetItem* treeItem = new QTreeWidgetItem(ui->treeWidget);
    treeItem->setText(0, createRepresentation(fileSize));
    for (auto copies : hashes) {
        if (copies.second.size() == 1) continue;
        QTreeWidgetItem* item = new QFileHashTreeWidgetItem(treeItem);
        item->setText(0, QString("Hash is %1").arg(copies.first));
        item->setText(1, QString::number(copies.second.size()));
        for (std::string file : copies.second) {
            QTreeWidgetItem* subItem = new QTreeWidgetItem(item);
            subItem->setText(
                0, dir.relativeFilePath(QString::fromStdString(file)));
        }
    }
    treeItem->sortChildren(1, Qt::DescendingOrder);
    if (treeItem->childCount() == 0) delete treeItem;
}

void MainWindow::incBar() {
    ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void MainWindow::clearBar(size_t count) {
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(count);
    ui->treeWidget->clear();
}

void MainWindow::disableStop() {
    ui->stopButton->setDisabled(true);
    ui->findCopies->setEnabled(true);
}

void MainWindow::on_findCopies_clicked() {
    // Threading shit
    HashingThread* worker = new HashingThread(ui->directoryPath->text());
    hashThread = new QThread;
    worker->moveToThread(hashThread);
    // connect threading shit
    connect(hashThread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker, SIGNAL(finished()), hashThread, SLOT(quit()));
    connect(hashThread, SIGNAL(finished()), hashThread, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished()), this, SLOT(disableStop()));
    // connect progressbar signals
    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<map_hash_t>("map_hash_t");
    qRegisterMetaType<QDir>("QDir");
    connect(worker, SIGNAL(prepareProgressBar(size_t)), this, SLOT(clearBar(size_t)));
    connect(worker, SIGNAL(increaseProgressBar()), this, SLOT(incBar()));
    // tree changing signals
    connect(worker, SIGNAL(
                changeTree(
                   qint64,
                   map_hash_t const&,
                   QDir const&
                )
            ),
            this, SLOT(
                addItemToTree(
                    qint64,
                    map_hash_t const&,
                    QDir const&
                )
            )
    );

    hashThread->start();
    ui->stopButton->setEnabled(true);
    ui->findCopies->setDisabled(true);
}

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column) {
    size_t d = 0;
    auto itemCopy = item;
    for (;itemCopy != nullptr; d++, itemCopy = itemCopy->parent());
    std::cout << d << std::endl;
    if (d == 3) {
        QMessageBox msgBox;
        msgBox.setText("What do you want to do with this file?");
        QPushButton* location = msgBox.addButton(tr("Open location"), QMessageBox::ActionRole);
        QPushButton* open = msgBox.addButton(tr("Open file"), QMessageBox::ActionRole);
        QPushButton* del =msgBox.addButton(tr("Delete file"), QMessageBox::ActionRole);
        msgBox.addButton(QMessageBox::Cancel);
        msgBox.exec();

        QString path = ui->directoryPath->text() + QString::fromStdString("/") + item->text(0);
        if (msgBox.clickedButton() == location) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path.left(path.lastIndexOf('/'))));
        } else if (msgBox.clickedButton() == open) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        } else if (msgBox.clickedButton() == del) {
            QFile(path).remove();
            if (item->parent()->childCount() == 2) {
                item = item->parent();
                if (item->parent()->childCount() == 1)
                    item = item->parent();
            }
            delete item;
        }
    }
}

void MainWindow::on_stopButton_clicked() {
    if (hashThread != nullptr) {
        hashThread->requestInterruption();
    }
}
