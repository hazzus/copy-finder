#ifndef HASHINGTHREAD_H
#define HASHINGTHREAD_H

#include <QObject>
#include <QMainWindow>
#include <QDir>
#include <QMessageBox>
#include <QTime>
#include <QDirIterator>
#include <QMetaType>
#include <QThread>
#include <QMap>
#include <QVector>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <math.h>
#include "reader.h"
#include "xxhash.hpp"

typedef std::map<xxh::hash64_t, std::vector<std::string>> map_hash_t;
Q_DECLARE_METATYPE(map_hash_t)

class HashingThread : public QObject {
    Q_OBJECT

private:
    QString dirname;
public:
    HashingThread(QString dirname);
    ~HashingThread();

public slots:
    void process();

signals:
    void finished();
    void prepareProgressBar(size_t count);
    void increaseProgressBar();
    void changeTree(
        qint64 fileSize,
        map_hash_t const& hashes,
        QDir const& dir);
};

#endif // HASHINGTHREAD_H
