#pragma once
#include <fstream>
#include <string>
#include <ios>
#include <vector>
#include <cstdint>
#include <iostream>

namespace WavFile {
	enum ckId {
		ckId_RIFF,
		ckId_FMT,
		ckId_NONE
	};

	struct Chunk {
		ckId chunkId;
		uint32_t chunkSize;

		void* chunkData;

		std::ifstream* file;

		std::vector<Chunk> subchunks;
	};

	struct RiffData {

	};

	Chunk* readChunk(std::ifstream& input, int pos) {
		std::string buf;

		Chunk* ck = new Chunk;
		ckId& chunkId = ck->chunkId;
		uint32_t& chunkSize = ck->chunkSize;

		// read Chunk ID
		input.seekg(pos);
		buf.resize(4, 0);
		input.read(buf.data(), 4);

		if (buf.compare("RIFF") == 0) {
			chunkId = ckId_RIFF;
		}
		else if (buf.compare("fmt ") == 0) {
			chunkId = ckId_FMT;
		}
		else {
			delete ck;
			return nullptr;
		}

		// read Chunk Size
		input.read((char*)(&chunkSize), 4);

		std::println(std::cout, "{}", chunkSize);

		__debugbreak();
	}

	Chunk* readFile(std::ifstream& input) {
		input.seekg(0, std::ios_base::end);
		if (input.tellg() < 12) {
			return nullptr;
		}
		else {
			return readChunk(input, 0);
		}
	}
}