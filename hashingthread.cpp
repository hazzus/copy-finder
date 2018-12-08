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
xxh::hash64_t takeHashOfFile(std::string filename) {
    xxh::hash_state64_t hash_stream;
    reader in(filename);
    while (!in.eof()) {
        hash_stream.update(in.read_byte_data(2048));
    }
    return hash_stream.digest();
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
        for (auto group : fileGroups) {
            if (QThread::currentThread()->isInterruptionRequested()) break;
            emit increaseProgressBar();
            if (group.second.size() == 1)
                continue;  // TODO is it optimisation to kick it out for branchh
                           // prediction?

            std::map<xxh::hash64_t, std::vector<std::string>>
                hashes;  // maybe put in map QTreeWidgetItem* ?

            for (std::string filename : group.second) {
                if (QThread::currentThread()->isInterruptionRequested()) break;
                try {
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
                } catch(std::runtime_error e) {
                    std::cerr << e.what() << std::endl;
                }
            }
            // send signal to add item to tree
            emit changeTree(group.first, hashes, directory);
        }
        std::cout << timer.elapsed() / 1000.0 << " seconds passed\n";
    }
    emit finished();
}
