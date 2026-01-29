export module WavReader;

import std;

using namespace std;

// 16 bit pcm class
export class WavSignal {
public:
	bool LoadFromFile(ifstream& input);
	void WriteToFile(ofstream& output);

	vector<vector<int16_t>> data;
	uint32_t sampleRate = 44100;
};

bool WavSignal::LoadFromFile(ifstream& input) {
	char chunkId[8];
	uint32_t chunkSize = 0;

	input.seekg(0);

	// Read riff and wave
	input.read(chunkId, 4);
	input.ignore(4);
	input.read(chunkId + 4, 4);

	if (strncmp(chunkId, "RIFFWAVE", 8) != 0) {
		println("not a wav file");
		return false;
	};

	// Read chunks until format and data are found
	bool fmtFound = false;
	uint32_t samples = 0;
	uint16_t channels = 0;

	bool dataFound = false;
	uint32_t dataPos = 0;
	uint32_t dataSize = 0;
	uint16_t bytesPerBlock = 0;


	while (!(fmtFound && dataFound)) {
		input.read(chunkId, 4); // Chunk Id
		input.read((char*) &chunkSize, 4); // Chunk size in bytes

		if (input.eof()) break;

		if (strncmp(chunkId, "fmt ", 4) == 0) {
			uint32_t tagAndBits = 0;

			input.read((char*)&tagAndBits, 2); // Format tag (must be 1 for pcm)
			input.read((char*)&channels, 2); // Number of channels
			input.read((char*)&sampleRate, 4); // Samples per second
			input.ignore(4); // Average bytes per second
			input.read((char*)&bytesPerBlock, 2); // Bytes per block
			input.read((char*)&tagAndBits + 2, 2); // Bits per sample (must be 16)

			if (tagAndBits != 0x00100001) {
				println("your file must be 16 bit pcm");
				return false;
			};

			fmtFound = true;
		}
		else if (strncmp(chunkId, "data ", 4) == 0) {
			dataPos = input.tellg();
			input.ignore(chunkSize);

			dataSize = chunkSize;
			dataFound = true;
		}
		else {
			input.ignore(chunkSize);
		}
	}

	if (!(fmtFound && dataFound)) return false;

	samples = dataSize / bytesPerBlock;

	// Split data into channels
	data.clear();
	data.resize(channels);
	for (int i = 0; i < channels; ++i) data[i].resize(samples);

	input.seekg(dataPos);

	int16_t sample = 0;

	for (int i = 0; i < dataSize / bytesPerBlock; ++i) {
		for (int j = 0; j < channels; j++) {
			input.read((char*)&sample, 2);
			data[j][i] = sample;
		}
	}

	println(cout, "{} {}", samples, channels);

	return true;
}

void WavSignal::WriteToFile(ofstream& output) {
	uint16_t channels = data.size();
	uint32_t samples = data.at(0).size();

	uint16_t num2;
	uint32_t num4;

	// Format chunk (and the file header)
	output.write("RIFF____WAVEfmt ", 16);
	num4 = 16; // Data chunk size (16 bytes)
	output.write((char*)&num4, 4);
	num2 = 1; // Format tag (PCM)
	output.write((char*)&num2, 2);
	num2 = channels; // Number of Channels
	output.write((char*)&num2, 2);
	num4 = sampleRate; // Sample rate
	output.write((char*)&num4, 4);
	num4 = sampleRate * channels * 2; // Average Bytes Per Second (channels * sample size * sample rate)
	output.write((char*)&num4, 4);
	num2 = channels * 2; // Block size (bytes)
	output.write((char*)&num2, 2);
	num2 = 16; // Sample size (bits)
	output.write((char*)&num2, 2);

	// Data chunk
	output.write("data", 4);
	num4 = samples * channels * 2; // samples * channels * sample size
	output.write((char*) & num4, 4);

	println(cout, "{} {}", samples, channels);

	for (int i = 0; i < samples; ++i) {
		for (int j = 0; j < channels; j++) {
			num2 = data.at(j).at(i);
			output.write((char*)&num2, 2);
		}
	}

	// Fill size
	uint32_t size = output.tellp();
	println(cout, "{} ? num4 {}", size - 8, num4);
	output.seekp(4);
	num4 = size - 8;
	output.write((char*)&num4, 4);
	output.seekp(0, ios_base::end);
}