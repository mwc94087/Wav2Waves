// Wav2Waves.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

import WavSignal;
import std;

using namespace std;

string filePath;
string outFilePath = "C:\\Users\\mwc94087\\Desktop\\out.wav";
ifstream inputFile;
ofstream outputFile;

void readAndWriteFile() {
	// get file, open it
	while (!inputFile.is_open()) {
		// get file
		println(cout, "Put a file in or quit");
		getline(cin, filePath);

		if (filePath.compare("quit") == 0 || filePath.size() < 2) {
			return;
		}

		//take off any quotes and open file
		if (filePath.at(0) == '"') {
			filePath.erase(0, 1);
			filePath.pop_back();
		}

		inputFile.open(filePath, ios::binary);
	}

	// Fill the file with data
	WavSignal signal;
	bool success = signal.ReadFromFile(inputFile);

	if (!success) {
		println(cout, "i can't read this!");
		return;
	}

	// make it mono
	signal.setChannels(1);

	// make it fast
	signal.sampleRate *= 1.2;

	// write output
	outputFile.open(outFilePath, ios::binary);
	signal.WriteToFile(outputFile);
	outputFile.close();
}

void writeSineWave() {
	WavSignal signal;
	CosineWave wave;

	signal.setChannels(2);
	signal.setSamples(44100 * 2);
	signal.sampleRate = 44100;

	wave.amplitude = 0.2;
	wave.frequency = 440;
	wave.phase = 0;
	
	signal.SetCosine(wave, 0);
	wave.phase = numbers::pi;
	signal.SetCosine(wave, 1);

	outputFile.open(outFilePath, ios::binary);
	signal.WriteToFile(outputFile);
	outputFile.close();
}

int main()
{
	writeSineWave();

	return 0;
}