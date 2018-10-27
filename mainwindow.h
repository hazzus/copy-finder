#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <resultdialog.h>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QMainWindow>
#include <QMap>
#include <QMessageBox>
#include <thread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

   private slots:
    void on_selectDirectory_clicked();

    void on_findCopies_clicked();

   private:
    Ui::MainWindow* ui;

    void startRecursiveHashing(QDir const& dir);

    void recursiveHash(QDir const& dir,
                       QMap<QByteArray, QVector<QString>>& hashes);
};

#endif  // MAINWINDOW_H
