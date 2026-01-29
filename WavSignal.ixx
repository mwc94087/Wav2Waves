export module WavSignal;
import std;

using namespace std;

#define clamp(x, low, high) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

constexpr double tau = numbers::pi * 2;

// cosine class to represent a cosine
export class CosineWave {
public:
	double frequency = 0; // cycles per second
	double amplitude = 0; // [-1, 1]
	double phase = 0; // [0, tau)

	double getValueAtTime(double t);
private:
};

double [[nodiscard]] CosineWave::getValueAtTime(double t) {
	return cos(t * tau * frequency + phase) * amplitude;
}

// 16 bit pcm class
export class WavSignal {
public:
	void SetCosine(CosineWave wave, int channel);
	bool ReadFromFile(ifstream& input);
	void WriteToFile(ofstream& output);

	void copyChannel(uint32_t from, uint32_t to);

	void setChannels(uint32_t count);
	void setSamples(uint32_t count);
	void clear();

	uint32_t sampleRate = 0;
private:
	uint32_t channels = 0;
	uint32_t samples = 0;

	vector<vector<double>> data;
};

void WavSignal::SetCosine(CosineWave wave, int channel) {
	if (channel >= data.size())
		throw out_of_range("Index out of range");

	for (uint32_t i = 0; i < samples; ++i) {
		double time = (double)i / (double)sampleRate;

		data.at(channel).at(i) = wave.getValueAtTime(time);
	}
}

bool WavSignal::ReadFromFile(ifstream& input) {
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
	for (uint32_t i = 0; i < channels; ++i) data[i].resize(samples);

	input.seekg(dataPos);

	int16_t sample = 0;

	for (uint32_t i = 0; i < dataSize / bytesPerBlock; ++i) {
		for (uint32_t j = 0; j < channels; j++) {
			input.read((char*)&sample, 2);

			// Convert 16 bit int into double [-1, 1]
			data[j][i] = ((double)sample + 0.5f) / 32767.5f; 
		}
	}

	return true;
}

void WavSignal::WriteToFile(ofstream& output) {
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
	num4 = channels * 2 * sampleRate; // Average Bytes Per Second (channels * sample size * sample rate)
	output.write((char*)&num4, 4);
	num2 = channels * 2; // Block size (bytes)
	output.write((char*)&num2, 2);
	num2 = 16; // Sample size (bits)
	output.write((char*)&num2, 2);

	// Data chunk
	output.write("data", 4);
	num4 = samples * channels * 2; // samples * channels * sample size
	output.write((char*) & num4, 4);

	int16_t sample;

	println(cout, "Array size: {}, {}", data.size(), data.at(data.size() - 1).size());

	for (uint32_t i = 0; i < samples; ++i) {
		for (uint32_t j = 0; j < channels; j++) {
			// Convert double [-1, 1] into 16 bit int
			sample = (int16_t) (data.at(j).at(i) * 32767.5 - 0.5);
			output.write((char*)&sample, 2);
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

void WavSignal::copyChannel(uint32_t from, uint32_t to) {
	if (to >= data.size() || from >= data.size())
		throw out_of_range("Index out of range");

	data.at(to) = data.at(from);
}

void WavSignal::setChannels(uint32_t count) {
	data.resize(count);

	if (channels < data.size()) {
		for (uint32_t i = channels; i < data.size(); ++i) {
			data.at(i).resize(samples);
		}
	}

	channels = count;
}

void WavSignal::setSamples(uint32_t count) {
	for (auto& a : data) {
		a.resize(count);
	}

	samples = count;
}

void WavSignal::clear() {
	data.clear();
	samples = 0;
	channels = 0;
	sampleRate = 0;
}

// Float class 