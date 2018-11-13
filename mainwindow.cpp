#include "mainwindow.h"
#include "ui_mainwindow.h"

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

std::map<qint64, std::vector<std::string>> listUniqueFilesBySize(
    QDir const& dir) {
    std::map<qint64, std::vector<std::string>> fileGroups;
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        qint64 fileSize = it.fileInfo().size();
        if (fileGroups.find(fileSize) == fileGroups.end()) {
            fileGroups.insert({fileSize, std::vector<std::string>(
                                             1, it.filePath().toStdString())});
        } else {
            fileGroups[fileSize].push_back(it.filePath().toStdString());
        }
    }
    return fileGroups;
}

// Algorithm:
//      1. Go through the path recursively, count files amount and create map
//      sorted by size
//      2. If 1 in map[key_size] then skip
//      3. Walk over file groups and count map of hashes
//      3. Create result
void MainWindow::on_findCopies_clicked() {
    ui->logger->clear();
    ui->progressBar->reset();
    QString path = ui->directoryPath->text();
    QDir directory(path);
    if (!directory.exists()) {
        ui->logger->insertHtml(
            "<div style=\"color: red;\"><b>WARNING:</b> No such "
            "directory</div>");
    } else {
        // Start hashing and so on
        ui->logger->append("Starting process...");
        directory.setFilter(QDir::Hidden | QDir::Files);
        ui->logger->append("Counting files...");
        QTime timer;
        timer.start();
        auto fileGroups = listUniqueFilesBySize(directory);

        ui->progressBar->setMaximum(fileGroups.size());

        ResultDialog resDia(this, &directory);
        for (auto group : fileGroups) {
            ui->progressBar->setValue(ui->progressBar->value() + 1);
            if (group.second.size() == 1) continue;

            // TODO empty files optimization
            std::map<xxh::hash64_t, std::vector<std::string>> hashes;

            xxh::hash_state64_t hash_stream;
            for (std::string filename : group.second) {
                hash_stream.reset();
                std::ifstream file(filename, std::ios::binary);
                std::vector<unsigned char> buffer(
                    std::istreambuf_iterator<char>(file), {});
                hash_stream.update(buffer);
                xxh::hash64_t result = hash_stream.digest();
                if (hashes.find(result) == hashes.end()) {
                    hashes.insert(
                        {result, std::vector<std::string>(1, filename)});
                } else {
                    // TODO byte-compare if collision
                    hashes[result].push_back(filename);
                }
            }
            resDia.appendData(&hashes, group.first);
        }
        ui->logger->append(QString("Process finished in %1 seconds")
                               .arg(timer.elapsed() / 1000.0));
        resDia.exec();
    }
}
