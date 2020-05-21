#include "lame_interface.h"
#include <mutex>
#include <fstream>
#include <iostream>

std::mutex mutFilesFinished;

int encode_to_file(lame_global_flags *gfp, const FMT_DATA *hdr, const short *leftPcm, const short *rightPcm,
    const int iDataSize, const char *filename)
{
    int numSamples = iDataSize / hdr->wBlockAlign; // divide by 4
    int mp3BufferSize = numSamples * 5 / 4 + 7200; // worst case estimate
    unsigned char *mp3Buffer = new unsigned char[mp3BufferSize];

    // call to lame_encode_buffer
    int mp3size = lame_encode_buffer(gfp, (short*)leftPcm, (short*)rightPcm, numSamples, mp3Buffer, mp3BufferSize);
    if (!(mp3size > 0)) {
        delete[] mp3Buffer;
        std::cerr << "No data was encoded by lame_encode_buffer. Return code: " << mp3size << std::endl;
        return EXIT_FAILURE;
    }

    // write to file
    //
    FILE *out = fopen(filename, "wb+");
    std::ofstream ofs;

    fwrite((void*)mp3Buffer, sizeof(unsigned char), mp3size, out);

    // call to lame_encode_flush
    int flushSize = lame_encode_flush(gfp, mp3Buffer, mp3BufferSize);

    // write flushed buffers to file
    fwrite((void*)mp3Buffer, sizeof(unsigned char), flushSize, out);

    // call to lame_mp3_tags_fid (might be omitted)
    lame_mp3_tags_fid(gfp, out);

    fclose(out);
    delete[] mp3Buffer;

    return EXIT_SUCCESS;
}

void *encode_worker(void* arg)
{
    int ret;
    ENC_WRK_ARGS *args = reinterpret_cast<ENC_WRK_ARGS*>(arg); // parse argument struct

    while (true) {
        // determine which file to process next
        bool bFoundWork = false;
        int iFileIdx = -1;

        //pthread_mutex_lock(&mutFilesFinished);
        mutFilesFinished.lock();

        for (int i = 0; i < args->iNumFiles; i++) {
            if (!args->pbFilesFinished[i]) {
                args->pbFilesFinished[i] = true; // mark as being worked on
                iFileIdx = i;
                bFoundWork = true;
                break;
            }
        }
        //pthread_mutex_unlock(&mutFilesFinished);
        mutFilesFinished.unlock();

        if (!bFoundWork) {// done yet?
            return NULL; // break
        }
        std::string sMyFile = args->pFilenames->at(iFileIdx);
        std::string sMyFileOut = sMyFile.substr(0, sMyFile.length() - 3) + "mp3";

        // start working
        FMT_DATA *hdr = NULL;
        short *leftPcm = NULL, *rightPcm = NULL;
        // init encoding params
        lame_global_flags *gfp = lame_init();
        lame_set_brate(gfp, 320);
        // parse wave file
        int iDataSize = -1;

        ret = read_wave(sMyFile.c_str(), hdr, leftPcm, rightPcm, iDataSize);

        if (ret != EXIT_SUCCESS) {
            std::cout << "Error in file " << sMyFile.c_str() << ". Skipping" << std::endl;
            continue; // see if there's more to do
        }
        //check wave sample frequency

        lame_set_in_samplerate(gfp,hdr->dwSamplesPerSec);
        lame_set_quality(gfp, 2); // increase quality level
        lame_set_num_channels(gfp, hdr->wChannels);
        lame_set_num_samples(gfp, iDataSize / hdr->wBlockAlign);
        // check params
        ret = lame_init_params(gfp);
        if (ret != 0) {
            std::cerr << "Invalid encoding parameters! Skipping file." << std::endl;
            continue;
        }

        // encode to mp3
        ret = encode_to_file(gfp, hdr, leftPcm, rightPcm, iDataSize, sMyFileOut.c_str());
        if (ret != EXIT_SUCCESS) {
            std::cerr << "Unable to encode mp3: " << sMyFileOut.c_str() << std::endl;
            continue;
        }

        std::cout << "File [" << sMyFile.c_str() << "] converted succesfully!" << std::endl;
        ++args->iProcessedFiles;

        lame_close(gfp);
        if (leftPcm != NULL) delete[] leftPcm;
        if (rightPcm != NULL) delete[] rightPcm;
        if (hdr != NULL) delete hdr;
    }
}
