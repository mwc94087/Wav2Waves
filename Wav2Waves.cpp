// Wav2Waves.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

import WavReader;
import std;

using namespace std;

string filePath;
ifstream inputFile;
ofstream outputFile;

int main()
{
	// get file, open it
	while (!inputFile.is_open()) {
		// get file
		println(cout, "Put a file in or quit");
		getline(cin, filePath);

		if (filePath.compare("quit") == 0 || filePath.size() < 2) {
			return 0;
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
	bool success = signal.LoadFromFile(inputFile);

	if (!success) {
		println(cout, "i can't read this!");
		return 0;
	}

	// make it mono (or break the whole thing? whatever you want i guess)
	signal.data.pop_back();

	// make it fast
	signal.sampleRate *= 2;

	// write output
	outputFile.open("C:\\Users\\mwc94087\\Desktop\\out.wav", ios::binary);
	signal.WriteToFile(outputFile);
	outputFile.close();

	return 0;
}