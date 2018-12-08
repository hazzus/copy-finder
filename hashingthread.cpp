#include "hashingthread.h"

HashingThread::HashingThread(QString dirname) : dirname(dirname) {}

HashingThread::~HashingThread() {}

bool byteCompare(std::string orig, std::string dup) {
    reader original(orig);
    reader duplicate(dup);
    while (!original.eof() && !duplicate.eof()) {
        if (original.read_byte_data(2048) != duplicate.read_byte_data(2048)) return false;
    }
    return true;
}

// Now takes non-cryptographic xxHash, can be changed
std::pair<std::string, xxh::hash64_t> takeHashOfFile(QString filename) {
    xxh::hash_state64_t hash_stream;
    reader in(filename.toStdString());
    while (!in.eof()) {
        hash_stream.update(in.read_byte_data(2048));
    }
    return {filename.toStdString(), hash_stream.digest()};
}

QMap<qint64, QVector<QString>> groupFilesBySize(
    QDir const& directory) {
    QMap<qint64, QVector<QString>> result;
    QDirIterator fs_it(directory, QDirIterator::Subdirectories);
    while (fs_it.hasNext()) {
        fs_it.next();
        qint64 fileSize = fs_it.fileInfo().size();
        auto it = result.find(fileSize);
        if (it == result.end()) {
            result.insert(fileSize, QVector<QString>(1, fs_it.filePath()));
        } else {
            it->push_back(fs_it.filePath());
        }
    }
    return result;
}


void addToMap(std::map<xxh::hash64_t, std::vector<std::string>>& hashes, std::pair<std::string, xxh::hash64_t> const& info) {
    try {
        auto it = hashes.find(info.second);
        if (it == hashes.end()) {
            hashes.insert({info.second, std::vector<std::string>(1, info.first)});
        } else {
            // Do we really need to compare them by bytes?
            if (byteCompare((*it).second[0], info.first))
                (*it).second.push_back(info.first);
        }
    } catch(std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}


// Algorithm:
//      1. Go through the path recursively, count files amount and create map
//      sorted by size
//      2. If 1 in map[key_size] then skip
//      3. Walk over file groups and count map of hashes
//      4. Create result
void HashingThread::process() {
    QDir directory(dirname);
    if (!directory.exists()) {
        QMessageBox warning;
        warning.setText("No such directory! Select another one!");
        warning.exec();
    } else {
        // preparations
        directory.setFilter(QDir::Files | QDir::Hidden);
        QTime timer;
        timer.start();

        // Counting and sorting
        auto fileGroups = groupFilesBySize(directory);
        emit prepareProgressBar(fileGroups.size() - 1);

        // Hashing
        for (auto groupKey : fileGroups.keys()) {
            if (QThread::currentThread()->isInterruptionRequested()) break;
            auto value = fileGroups.value(groupKey);
            emit increaseProgressBar();
            if (value.size() == 1)
                continue;  // TODO is it optimisation to kick it out for branchh
                           // prediction?
            // maybe put in map QTreeWidgetItem* ?

            // THIS IS THREADING
            QFuture<std::map<xxh::hash64_t, std::vector<std::string>>> hashFuture = QtConcurrent::mappedReduced(value, takeHashOfFile, addToMap);

            // send signal to add item to tree
            // TODO get key
            hashFuture.waitForFinished();

            auto hashes = hashFuture.result();
            emit changeTree(groupKey, hashes, directory);
        }
        std::cout << timer.elapsed() / 1000.0 << " seconds passed\n";
    }
    emit finished();
}
