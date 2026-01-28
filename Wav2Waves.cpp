// Wav2Waves.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>


using namespace std;

string filePath;
ifstream inputFile;


int main()
{
	while (!inputFile.is_open()) {
		// get file
		println(cout, "Put a file in or quit");
		getline(cin, filePath);

		if (filePath.compare("quit") == 0 || filePath.size() < 2) {
			exit(0);
		}

		// format path (take off quotes) and open file
		filePath.erase(0, 1);
		filePath.pop_back();

		inputFile.open(filePath, ios_base::binary);
	}



	return 0;
}