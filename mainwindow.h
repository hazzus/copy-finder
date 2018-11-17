#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDirIterator>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QTime>
#include <QTreeWidgetItem>
#include <QDesktopServices>
#include <map>
#include "reader.h"
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

private:
    Ui::MainWindow* ui;

    void addItemToTree(
        qint64 fileSize,
        std::map<xxh::hash64_t, std::vector<std::string>> const& hashes,
        QDir const& dir);
};

#endif  // MAINWINDOW_H
