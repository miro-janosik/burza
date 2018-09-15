// Program generuje zoznam dvojpismenovych kodov a k nim trojpismenove hesla.
// Ak sa prida ako parameter subor s kodmi (za pomlckou mozu byt hesla)
// tak tieto sa pouziju ako stare kody, pre ktore netreba generovat hesla

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <ctime>

using namespace std;

int main(int argc, char** argv)
{
	// random seed
	srand( (unsigned int)time(0) );
	set<string> oldCodes;

	if (argc >= 2)
	{
		string oldCodesFileName = argv[1];
		ifstream oldCodesFile(oldCodesFileName);
		if (!oldCodesFile.good())
		{
			cout << "Error opening old code file from parameter " << oldCodesFileName << std::endl;
			return 1;
		}

		cout << "Old codes: ";
		for ( string line; getline(oldCodesFile, line ); )
		{
			// strip away password after minus
			size_t pos = line.find_first_of("-");
			if (string::npos != pos)
				line = line.substr(0, pos);

			// change letters to uppercase			
			for (auto& c : line ) c = toupper(c);
			oldCodes.insert(line);
			cout << line << " ";
		}

		cout << endl;
	}

	// char validLetters[] = "ABCDEHIKLMPRSTVYZ";
	string validLetters = "ABCDEHLIKMPRSTVZ";

	ofstream listFile("codes.csv");

	for ( char first : validLetters )
		for ( char second : validLetters )
		{
			string code;
			code += first;
			code += second;

			// see if it is old or new
			bool old = (oldCodes.end() != oldCodes.find(code));

			// for new codes generate passwords
			string codeWithPassword;
			if (!old)
			{
				string pwd;
				static const unsigned char lettersCount = 'Z' - 'A';
				for (int i=0; i<3; ++i)
				{
					char l = 'A' + (rand() % lettersCount );
					pwd += l;
				}

				codeWithPassword = code + "-" + pwd;
			}

			listFile << code << ", " << (old ? 1 : 0) << ", " << codeWithPassword << endl;
		}

	cout << "Generated." << endl;
	return 0;
}
