#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFuture>
#include <QtConcurrent/QtConcurrent>

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

int countAll(QDir const& dir) {
    int result = 0;
    QDirIterator it(dir);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isDir()) {
            if (it.fileName() != "." && it.fileName() != "..")
                result += countAll(QDir(it.filePath()));
        } else {
            result++;
        }
    }
    return result;
}

void MainWindow::recursiveHash(QDir const& dir,
                               QMap<QByteArray, QVector<QString>>& hashes) {
    QDirIterator it(dir);
    while (it.hasNext()) {
        it.next();
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        if (it.fileInfo().isDir()) {
            if (it.fileName() != "." && it.fileName() != "..")
                recursiveHash(QDir(it.filePath()), hashes);
        } else {
            QFile f(it.filePath());
            if (f.open(QFile::ReadOnly)) {
                QCryptographicHash hash(QCryptographicHash::Sha256);
                hash.addData(&f);
                QByteArray result = hash.result();
                if (hashes.find(result) == hashes.end()) {
                    hashes.insert(result, QVector<QString>(1, f.fileName()));
                } else {
                    hashes[result].push_back(f.fileName());
                    ui->logger->append(f.fileName() + " is a copy");
                }
            }
        }
    }
}

void MainWindow::startRecursiveHashing(QDir const& dir) {
    ui->progressBar->setValue(0);
    ui->logger->insertHtml(QString("Starting process for %1").arg(dir.path()));
    ui->logger->append("Counting files in directory...");
    int amount = countAll(dir);
    if (amount > 5000) {
        QMessageBox question;
        question.setText(
            QString(
                "There is %1 files in this directory, operation might take a "
                "long time.")
                .arg(amount));
        question.setInformativeText(
            "Are you sure you want to find all copies in this directory?");
        question.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        question.setDefaultButton(QMessageBox::Cancel);
        int ret = question.exec();
        if (ret == QMessageBox::Cancel) {
            ui->logger->append("Process canceled by user");
            return;
        }
    }

    ui->progressBar->setMaximum(amount);

    QMap<QByteArray, QVector<QString>> hashes;

    // QFuture<void> f = QtConcurrent::run(recursiveHash, dir, hashes);

    recursiveHash(dir, hashes);

    ui->logger->append("Process finished");

    ResultDialog result(nullptr, &hashes, &dir);
    result.exec();
}

void MainWindow::on_findCopies_clicked() {
    ui->logger->clear();
    QString path = ui->directoryPath->text();
    QDir directory(path);
    if (!directory.exists()) {
        ui->logger->insertHtml(
            "<div style=\"color: red;\"><b>WARNING:</b> No such "
            "directory</div>");
    } else {
        startRecursiveHashing(directory);
    }
}
