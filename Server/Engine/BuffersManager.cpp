#include "BuffersManager.h"

int BuffersManager::kMaxBuffersCount_ = 5;
std::multimap<int, BuffersManager::BufferData> BuffersManager::buffers_;
int BuffersManager::kBufferSize_ = 65536;
int BuffersManager::loadedFromFile = 0;
int BuffersManager::loadedFromFileWithoutOptimisation = 0;

char* BuffersManager::getOrCreateBuffer(const std::string& tablename, int position) {
    char* buffer = getBuffer(tablename, position);
    if (buffer == nullptr) {
        insertNewBuffer(tablename, position);
        buffer = getBuffer(tablename, position);
        loadedFromFile += kBufferSize_;
    }
    loadedFromFileWithoutOptimisation += kBufferSize_;
    return buffer;
}

char* BuffersManager::getBuffer(const std::string& tablename, int position) {
    for (auto& buffer : buffers_) {
        if (std::get<0>(buffer.second) == tablename && std::get<1>(buffer.second) == position) {
            char* result = std::get<2>(buffer.second);
            increaseUsageIfExists(tablename, position, 1);
            return result;
        }
    }
    return nullptr;
}

void BuffersManager::updateBufferIfExists(const std::string& tablename, int position, const char* new_buff) {
    char* buffer = getBuffer(tablename, position);
    if (buffer != nullptr) {
        memcpy(buffer, new_buff, kBufferSize_);
    }
}

void BuffersManager::insertNewBuffer(const std::string& tablename, int position) {
    if (buffers_.size() == kMaxBuffersCount_) {
        delete [] std::get<2>(buffers_.begin()->second);
        buffers_.erase(buffers_.begin());
    }
    char* buffer = new char[kBufferSize_]{};
    std::fstream file("DataBD/" + tablename, std::fstream::binary | std::fstream::in);
    file.seekp(position);
    file.read(buffer, kBufferSize_);
    buffers_.insert(std::make_pair(0, std::make_tuple(tablename, position, buffer)));
}

void BuffersManager::increaseUsageIfExists(const std::string& tablename, int position, int usage) {
    for (auto it = buffers_.begin(); it != buffers_.end(); ++it) {
        if (std::get<0>(it->second) == tablename && std::get<1>(it->second) == position) {
            auto new_usage = it->first + usage;
            auto buffer_data = it->second;
            buffers_.erase(it);
            buffers_.insert(std::make_pair(new_usage, buffer_data));
            break;
        }
    }
}

void BuffersManager::dropTable(const std::string& tablename) {
    for (auto it = buffers_.begin(); it != buffers_.end();) {
        if (std::get<0>(it->second) == tablename) {
            delete [] std::get<2>(buffers_.begin()->second);
            it = buffers_.erase(it);
        } else {
            ++it;
        }
    }
}

void BuffersManager::clear() {
    while (!buffers_.empty()) {
        dropTable(std::get<0>(buffers_.begin()->second));
    }
}

