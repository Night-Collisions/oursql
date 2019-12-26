#ifndef OURSQL_SERVER_ENGINE_BUFFERSMANAGER_H_
#define OURSQL_SERVER_ENGINE_BUFFERSMANAGER_H_

#include <cstring>
#include <tuple>
#include <map>
#include <fstream>

class BuffersManager {
   public:
    static char* getOrCreateBuffer(const std::string& tablename, int position);
    static char* getBuffer(const std::string& tablename, int position);
    static void updateBufferIfExists(const std::string& tablename, int position, const char* new_buff);
    static void increaseUsageIfExists(const std::string& tablename, int position, int usage);
    static void dropTable(const std::string& tablename);
    static void clear();

    static int kBufferSize_;
    static int getLoadedFromFile() { return loadedFromFile; }
    static int getLoadedFromFileWithoutOptimisation() { return loadedFromFileWithoutOptimisation; }

   private:
    using BufferData = std::tuple<std::string, int, char*>;

    static void insertNewBuffer(const std::string& tablename, int position);

    static int kMaxBuffersCount_;
    static int loadedFromFile;
    static int loadedFromFileWithoutOptimisation;
    static std::multimap<int, BufferData> buffers_;
};

#endif
