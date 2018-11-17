#include "mainwindow.h"
#include "ui_mainwindow.h"

// Only for beta
#include <fstream>
#include <iostream>
#include <iterator>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setValue(0);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_selectDirectory_clicked() {
    QString directory = QFileDialog::getExistingDirectory(this);
    ui->directoryPath->clear();
    ui->directoryPath->insert(directory);
    ui->progressBar->setValue(0);
}

std::map<qint64, std::vector<std::string>> groupFilesBySize(
    QDir const& directory) {
    std::map<qint64, std::vector<std::string>> result;
    QDirIterator fs_it(directory, QDirIterator::Subdirectories);
    while (fs_it.hasNext()) {
        fs_it.next();
        qint64 fileSize = fs_it.fileInfo().size();
        auto it = result.find(fileSize);
        if (it == result.end()) {
            result.insert({fileSize, std::vector<std::string>(
                                         1, fs_it.filePath().toStdString())});
        } else {
            (*it).second.push_back(fs_it.filePath().toStdString());
        }
    }
    return result;
}

// Now takes non-cryptographic xxHash, can be changed
xxh::hash64_t takeHashOfFile(std::string filename) {
    xxh::hash_state64_t hash_stream;
    reader in(filename);
    while (!in.eof()) {
        hash_stream.update(in.read_byte_data(2048));
    }
    return hash_stream.digest();
}

bool byteCompare(std::string orig, std::string dup) {
    reader original(orig);
    reader duplicate(dup);
    while (!original.eof() && !duplicate.eof()) {
        if (original.read_byte_data(2048) != duplicate.read_byte_data(2048)) return false;
    }
    return true;
}

void MainWindow::addItemToTree(
    qint64 fileSize,
    std::map<xxh::hash64_t, std::vector<std::string>> const& hashes,
    QDir const& dir) {
    QTreeWidgetItem* treeItem = new QTreeWidgetItem(ui->treeWidget);
    treeItem->setText(0, QString("%1 bytes").arg(fileSize));
    for (auto copies : hashes) {
        if (copies.second.size() == 1) continue;
        QTreeWidgetItem* item = new QTreeWidgetItem(treeItem);
        item->setText(0, QString::number(copies.first));
        item->setText(1, QString::number(copies.second.size()));
        for (std::string file : copies.second) {
            QTreeWidgetItem* subItem = new QTreeWidgetItem(item);
            subItem->setText(
                0, dir.relativeFilePath(QString::fromStdString(file)));
        }
    }
    if (treeItem->childCount() == 0) delete treeItem;
}

// Algorithm:
//      1. Go through the path recursively, count files amount and create map
//      sorted by size
//      2. If 1 in map[key_size] then skip
//      3. Walk over file groups and count map of hashes
//      4. Create result
void MainWindow::on_findCopies_clicked() {
    QDir directory(ui->directoryPath->text());
    if (!directory.exists()) {
        QMessageBox warning;
        warning.setText("No such directory! Select another one!");
        warning.exec();
    } else {
        // preparations
        ui->progressBar->reset();
        ui->treeWidget->clear();
        directory.setFilter(QDir::Files | QDir::Hidden);
        QTime timer;
        timer.start();

        // Counting and sorting
        auto fileGroups = groupFilesBySize(directory);
        ui->progressBar->setMaximum(fileGroups.size() - 1);

        // Hashing
        for (auto group : fileGroups) {
            ui->progressBar->setValue(ui->progressBar->value() + 1);
            if (group.second.size() == 1)
                continue;  // TODO is it optimisation to kick it out for branchh
                           // prediction?

            std::map<xxh::hash64_t, std::vector<std::string>>
                hashes;  // maybe put in map QTreeWidgetItem* ?

            for (std::string filename : group.second) {
                xxh::hash64_t hash = takeHashOfFile(filename);
                auto it = hashes.find(hash);
                if (it == hashes.end()) {
                    hashes.insert(
                        {hash, std::vector<std::string>(1, filename)});
                } else {
                    // Do we really need to compare them by bytes?
                    if (byteCompare((*it).second[0], filename))
                        (*it).second.push_back(filename);
                }
            }
            addItemToTree(group.first, hashes, directory);
        }
        std::cout << timer.elapsed() / 1000.0 << " seconds passed\n";
    }
}

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    size_t d = 0;
    auto itemCopy = item;
    for (;itemCopy != nullptr; d++, itemCopy = itemCopy->parent());
    std::cout << d << std::endl;
    if (d == 3) {
        QString path = ui->directoryPath->text() + QString::fromStdString("/") + item->text(0);
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        QDesktopServices::openUrl(QUrl::fromLocalFile(path.left(path.lastIndexOf('/'))));
    }
}
