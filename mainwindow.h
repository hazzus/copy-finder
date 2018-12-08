#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDirIterator>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QTime>
#include <QTreeWidgetItem>
#include <QDesktopServices>
#include <QThread>
#include <map>
#include "hashingthread.h"
#include "xxhash.hpp"

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

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_stopButton_clicked();

public slots:
    void clearBar(size_t count);
    void incBar();
    void addItemToTree(
        qint64 fileSize,
        map_hash_t const& hashes,
        QDir const& dir);
    void disableStop();

   private:
    Ui::MainWindow* ui;
    QThread* hashThread;
};

#endif  // MAINWINDOW_H
