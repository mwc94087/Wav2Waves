#pragma once
#include <fstream>
#include <string>
#include <ios>
#include <vector>
#include <cstdint>
#include <iostream>

#define i32(a) ((uint32_t)(a))
#define shl(a, b) ((a) << (b))
#define combine4(a, b, c, d) ((a) | (b) | (c) | (d))
#define idtoui32(a, b, c, d) combine4(i32(a), shl(i32(b), 8), shl(i32(c), 16), shl(i32(d), 24))

using namespace std;

namespace WavFile {
	enum chunkId : uint32_t {
		ckId_RIFF = idtoui32('R', 'I', 'F', 'F'),
		ckId_FMT  = idtoui32('f','m','t',' '),
		ckId_DATA = idtoui32('d','a','t','a'),
		ckId_WAVE = idtoui32('W','A','V','E'),
		ckId_NONE = 0
	};

	struct Chunk {
		chunkId id = ckId_NONE;
		uint32_t size = 0;

		void* data = nullptr; // point to struct
		vector<Chunk*> subchunks;

		~Chunk() {
			for (Chunk* sc : subchunks) {
				delete sc;
			}

			if (data != nullptr) {
				delete data;
			}
		}
	};

	struct RiffData {
		uint32_t riffType;
	};

	struct FmtData {
		uint16_t wFormatTag;
		uint16_t nChannels;
		uint32_t nSamplesPerSec;
		uint32_t nAvgBytesPerSec;
		uint16_t nBlockAlign;
		uint16_t wBitsPerSample;
	};

	struct RawData {
		size_t dataSize = 0;
		char* data = nullptr;

		~RawData() {
			delete[] data;
		}
	};

	Chunk* readChunk(ifstream& input, size_t& pos) {
		std::string buf;

		Chunk* chunk = new Chunk;
		chunkId& chunkId = chunk->id;
		uint32_t& chunkSize = chunk->size;

		// read Chunk ID
		input.seekg(pos);
		input.read((char*)(&chunkId), 4);
		// read Chunk Size
		input.read((char*)(&chunkSize), 4);

		pos += 8;

		int endPos = pos + chunkSize;

		switch (chunkId) {
		case ckId_RIFF: {
			// Add riff data
			RiffData* data = new RiffData;
			chunk->data = data;

			// Read riff chunk type
			input.read((char*)(&data->riffType), 4);
			pos += 4;

			// dont read chunk members if its not wave
			if (data->riffType != ckId_WAVE) {
				println(cout, "this is not a wav file");
				break;
			}

			while (pos < endPos) {
				chunk->subchunks.push_back(readChunk(input, pos));
			}

			break;
		}
		case ckId_FMT: {
			FmtData* data = new FmtData;
			chunk->data = data;

			input.read((char*)(&data->wFormatTag), 2);
			input.read((char*)(&data->nChannels), 2);

			input.read((char*)(&data->nSamplesPerSec), 4);
			input.read((char*)(&data->nAvgBytesPerSec), 4);

			input.read((char*)(&data->nBlockAlign), 2);
			input.read((char*)(&data->wBitsPerSample), 2);

			if (chunkSize != 16) {
				println(cout, "Warning: wav file might be compressed");
			}

			pos = endPos;

			break;
		}
		case ckId_DATA: {
			RawData* data = new RawData;
			chunk->data = data;

			data->data = new char[chunkSize];
			data->dataSize = chunkSize;

			input.read(data->data, chunkSize);
			pos = endPos;

			break;
		}
		default:
			println(cout, "Skipped: Unknown chunk \"{}\"", std::string((char*)(&chunkId), 4));
			pos = endPos;
			chunkId = ckId_NONE;
			break;
		}

		return chunk;
	}

	Chunk* readFile(std::ifstream& input) {
		input.seekg(0, std::ios_base::end);
		size_t fSize = input.tellg();
		println(cout, "File is {} bytes", fSize);

		size_t pos = 0;

		if (input.tellg() < 8) {
			println(cout, "Stopped: File is too small to include any chunks!");
			return nullptr;
		}
		else {
			Chunk* out = readChunk(input, pos);
			println(cout, "Read {} bytes", pos);
		}
	}
}