#include <iostream>
#include <list>
#include <string>
#include <cstring>
#include <vector>
#include <ctime>
#include <dirent.h>
#include <map>
#include <thread>
#include <experimental/filesystem>

#include "lame_interface.h"

#include <pthread.h>

#ifdef WIN32
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif
bool file_extension(const std::string &fullString, const std::string &subString)
{
    if (subString.length() > fullString.length()) return false;
    else {
        // lowercase conversion
        std::string fullString_l = fullString;
        for (unsigned int i = 0; i < fullString_l.length(); i++) {
            if ('A' <= fullString_l[i] && fullString_l[i] <= 'Z')
                fullString_l[i] = fullString_l[i] - ('Z' - 'z');
        }
        std::string subString_l = subString;
        for (unsigned int i = 0; i < subString_l.length(); i++) {
            if ('A' <= subString_l[i] && subString_l[i] <= 'Z')
                subString_l[i] = subString_l[i] - ('Z' - 'z');
        }
        return (fullString_l.compare(fullString_l.length() - subString_l.length(), subString_l.length(),
            subString_l) == 0);
    }
}
//c++17
std::list<std::string> parse_directory(const char *dirname)
{
    DIR *dir;
    dirent *ent;
    std::list<std::string> dirEntries;
    for (const auto& entry : std::experimental::filesystem::directory_iterator(dirname)) {
        const auto filenameStr = entry.path().filename().string();
        dirEntries.push_back(filenameStr);
    }
    return dirEntries;
}

int main(int argc, char *argv[])
{
    //open as dir
    unsigned concurentThreadsSupported = std::thread::hardware_concurrency();
    int NUM_THREADS = concurentThreadsSupported;
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " PATH " << std::endl;
            std::cerr << "   PATH   required. E.g F:\MyWavCollection" << std::endl;
            return EXIT_FAILURE;
        }        

        // parse directory
        std::list<std::string> files = parse_directory(argv[1]);
        std::vector<std::string> wavFiles;
        //auto
        for (auto file : files) {
            // check if it's a wave file and add path
            if (file_extension(file, std::string(".wav"))) {
                wavFiles.push_back(std::string(argv[1]) + std::string(PATHSEP) + file);
            }
        }
        int numFiles = wavFiles.size();

        std::cout << "Found " << numFiles << " .wav file(s) in directory." << std::endl;
        if (!(numFiles>0)) return EXIT_SUCCESS;

        // initialize pbFilesFinished array which contains true for all files which are currently already converted
        bool *pbFilesFinished = new bool[numFiles];
        for (int i = 0; i < numFiles; i++) pbFilesFinished[i] = false;


        // initialize threads array and argument arrays
        pthread_t *threads = new pthread_t[NUM_THREADS];
        ENC_WRK_ARGS *threadArgs = (ENC_WRK_ARGS*)malloc(NUM_THREADS * sizeof(ENC_WRK_ARGS));
        for (int i = 0; i < NUM_THREADS; i++) {
            threadArgs[i].iNumFiles = numFiles;
            threadArgs[i].pFilenames = &wavFiles;
            threadArgs[i].pbFilesFinished = pbFilesFinished;
            threadArgs[i].iThreadId = i;
            threadArgs[i].iProcessedFiles = 0;
        }

        // timestamp
        std::clock_t tBegin = clock();

        // create worker threads
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_create(&threads[i], NULL, complete_encode_worker, (void*)&threadArgs[i]);
        }

        // synchronize / join threads
        for (int i = 0; i < NUM_THREADS; i++) {
            int ret = pthread_join(threads[i], NULL);
            if (ret != 0) {
                std::cerr << "A POSIX thread error occured." << std::endl;
            }
        }

        // timestamp
        std::clock_t tEnd = clock();

        // write statistics
        int iProcessedTotal = 0;
        for (int i = 0; i < NUM_THREADS; i++) {
            std::cout << "Thread " << i << " processed " << threadArgs[i].iProcessedFiles << " files." << std::endl;
            iProcessedTotal += threadArgs[i].iProcessedFiles;
        }

        std::cout << "Converted " << iProcessedTotal << " out of " << numFiles << " files in total in " <<
            double(tEnd-tBegin) / CLOCKS_PER_SEC << "s." << std::endl;

        delete[] threads;
        delete[] threadArgs;

        std::cout << "Done." << std::endl;
        if (iProcessedTotal > numFiles)
            return EXIT_FAILURE;
        return EXIT_SUCCESS;

    return 0;
}