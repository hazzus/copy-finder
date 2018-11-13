#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <resultdialog.h>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QTime>
#include <fstream>
#include <iterator>
#include <map>
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

   private:
    Ui::MainWindow* ui;
};

#endif  // MAINWINDOW_H
