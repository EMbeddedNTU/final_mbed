#pragma once

#include "mbed.h"
#include <functional>
#include <string>

#include "BlockDevice.h"
#include "FATFileSystem.h"


namespace GSH {

    class FileSystem
    {
    public:
        static constexpr int BUFFER_MAX_LEN = 10;
        static constexpr bool FORCE_REFORMAT = true;
    public:
        FileSystem()
            :m_FS("fs") 
        {}

        ~FileSystem() {}

        bool init();

        FILE* openFile(const std::string& filename);

        bool closeFile(FILE* f);

        void printFile(FILE* f);


        DIR* openDir(const std::string& dirPath);

        bool closeDir(DIR* dir);

        const string& changeDir(const std::string& dirPath);

        void printDir(DIR* dir);

    private:
        BlockDevice *m_BD = BlockDevice::get_default_instance();
        FATFileSystem m_FS;
        std::string m_CurrentDirPath;
    };

}