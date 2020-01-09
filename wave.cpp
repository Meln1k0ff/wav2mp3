#include "wave.h"

// function implementations
int read_wave_header(std::ifstream &file, FMT_DATA *&hdr, int &iDataSize, int &iDataOffset)
{
	if (!file.is_open()) return EXIT_FAILURE; // check if file is open
    file.seekg(0, std::ios::beg); // rewind file

	ANY_CHUNK_HDR chunkHdr;

    // validate RIFF header
	RIFF_HDR rHdr;
	file.read((char*)&rHdr, sizeof(RIFF_HDR));
	if (EXIT_SUCCESS != check_riff_header(&rHdr))
		return EXIT_FAILURE;

	// then continue parsing file until we find the 'fmt ' chunk
	bool bFoundFmt = false;
	while (!bFoundFmt && !file.eof()) {
		file.read((char*)&chunkHdr, sizeof(ANY_CHUNK_HDR));
		if (0 == strncmp(chunkHdr.ID, "fmt ", 4)) {
			// rewind and parse the complete chunk
			file.seekg((int)file.tellg()-sizeof(ANY_CHUNK_HDR));

			hdr = new FMT_DATA;
			file.read((char*)hdr, sizeof(FMT_DATA));
			bFoundFmt = true;
			break;
		} else {
			// skip this chunk (i.e. the next chunkSize bytes)
            file.seekg(chunkHdr.chunkSize, std::ios::cur);
		}
	}
	if (!bFoundFmt) { // found 'fmt ' at all?
        std::cerr << "FATAL: Found no 'fmt ' chunk in file." << std::endl;
	} else if (EXIT_SUCCESS != check_format_data(hdr)) { // if so, check settings
		delete hdr;
		return EXIT_FAILURE;
	}

	// finally, look for 'data' chunk
	bool bFoundData = false;
	while (!bFoundData && !file.eof()) {
		//printf("Reading chunk at 0x%X:", (int)file.tellg());
		file.read((char*)&chunkHdr, sizeof(ANY_CHUNK_HDR));
		//printf("%s\n", string(chunkHdr.ID, 4).c_str());
		if (0 == strncmp(chunkHdr.ID, "data", 4)) {
			bFoundData = true;
			iDataSize = chunkHdr.chunkSize;
			iDataOffset = file.tellg();
			break;
		} else { // skip chunk
            file.seekg(chunkHdr.chunkSize, std::ios::cur);
		}
	}
	if (!bFoundData) { // found 'data' at all?
        std::cerr << "FATAL: Found no 'data' chunk in file." << std::endl;
		delete hdr;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int check_format_data(const FMT_DATA *hdr)
{
	if (hdr->wFmtTag != 0x01) {
        std::cerr << "Bad non-PCM format:" << hdr->wFmtTag << std::endl;
		return EXIT_FAILURE;
	}
	if (hdr->wChannels != 1 && hdr->wChannels != 2) {
        std::cerr << "Bad number of channels (only mono or stereo supported)." << std::endl;
		return EXIT_FAILURE;
	}
	if (hdr->chunkSize != 16) {
        std::cerr << "WARNING: 'fmt ' chunk size seems to be off." << std::endl;
	}
	if (hdr->wBlockAlign != hdr->wBitsPerSample * hdr->wChannels / 8) {
        std::cerr << "WARNING: 'fmt ' has strange bytes/bits/channels configuration." << std::endl;
	}

	return EXIT_SUCCESS;
}

int check_riff_header(const RIFF_HDR *rHdr)
{
	if (0 == strncmp(rHdr->rID, "RIFF", 4) && 0 == strncmp(rHdr->wID, "WAVE", 4) && rHdr->fileLen > 0)
		return EXIT_SUCCESS;

    std::cerr << "Bad RIFF header!" << std::endl;
	return EXIT_FAILURE;
}

void get_pcm_channels_from_wave(std::ifstream &file, const FMT_DATA* hdr, short* &leftPcm, short* &rightPcm, const int iDataSize,
	const int iDataOffset)
{
	int idx;
	int numSamples = iDataSize / hdr->wBlockAlign;

	leftPcm = NULL;
	rightPcm = NULL;

	// allocate PCM arrays
	leftPcm = new short[iDataSize / hdr->wChannels / sizeof(short)];
	if (hdr->wChannels > 1)
		rightPcm = new short[iDataSize / hdr->wChannels / sizeof(short)];

	// capture each sample
	file.seekg(iDataOffset);// set file pointer to beginning of data array

	if (hdr->wChannels == 1) {
		file.read((char*)leftPcm, hdr->wBlockAlign * numSamples);
	} else {
		for (idx = 0; idx < numSamples; idx++) {
			file.read((char*)&leftPcm[idx], hdr->wBlockAlign / hdr->wChannels);
			if (hdr->wChannels>1)
				file.read((char*)&rightPcm[idx], hdr->wBlockAlign / hdr->wChannels);
		}
	}

	assert(rightPcm == NULL || hdr->wChannels != 1);
}

int read_wave(const char *filename, FMT_DATA* &hdr, short* &leftPcm, short* &rightPcm, int &iDataSize)
{
    std::ifstream inFile(filename, std::ios::in | std::ios::binary);
	if (inFile.is_open()) {
		// determine size and allocate buffer
        inFile.seekg(0, std::ios::end);

		// parse file
		int iDataOffset = 0;
        if (read_wave_header(inFile, hdr, iDataSize, iDataOffset) != EXIT_SUCCESS) {
			return EXIT_FAILURE;
		}
		get_pcm_channels_from_wave(inFile, hdr, leftPcm, rightPcm, iDataSize, iDataOffset);
		inFile.close();

		// cleanup and return
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
