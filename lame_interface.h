#ifndef __LAME_INTERFACE_H_
#define __LAME_INTERFACE_H_

#include <vector>
#include <sstream>
#include "../lame-3.100/include/lame.h"
#include "wave.h"
#include "pthread.h"

/*
 * POSIX-conforming argument struct for worker routine 'complete_encode_worker'.
 */
typedef struct {
    std::vector<std::string> *pFilenames;
    bool *pbFilesFinished;
    int iNumFiles;
    int iThreadId;
    int iProcessedFiles;
} ENC_WRK_ARGS;

/////////////////////
// function prototypes
/////////////////////

/* encode_to_file
 *  Call lame_encode_buffer, lame_encode_flush, and lame_mp3_tags_fid internally for a complete conversion
 *  process.
 */
int encode_to_file(lame_global_flags *gfp, const FMT_DATA *hdr, const short *leftPcm, const short *rightPcm,
    const int iDataSize, const char *filename);

/////////////////////
// threading worker routines conforming to POSIX interface
/////////////////////

/*  encode_worker
 *  Main worker thread routine which is supplied with a list of filenames, a status array indicating which files
 *  are already worked upon, and some additional info via a ENC_WRK_ARGS struct.
 *  As long as there are still unprocessed filenames, this routine will fetch the next free filename, mark it as
 *  processed, and execute the complete conversion from reading .wav to writing .mp3.
 */
void *encode_worker(void* arg);


#endif // __LAME_INTERFACE_H_
