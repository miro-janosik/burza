// Program generuje zoznam dvojpismenovych kodov a k nim trojpismenove hesla.
// Ak sa prida ako parameter subor s kodmi
// tak tieto sa pouziju ako stare kody, pre ktore netreba generovat hesla
// Tento subor obsahuje "AA-BBB<tab>Meno predajcu"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <tuple>
#include <vector>
#include <sstream>
#include <list>

using namespace std;

// removed similar letters, D ~ O, E ~ F, G ~ C, J ~ I, M ~ N, Q ~ O, U ~ V, Y ~ V
string validLetters = "ABCEHIKLMOPRSTVXZ";

int main(int argc, char** argv)
{
	// random seed
	srand( (unsigned int)time(0) );
    vector<tuple<string, string, string>> oldSellers;

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
			// separate into login and name
			size_t pos = line.find_first_of("\t");
			auto login = line.substr(0, pos);
			auto name = line.substr(pos+1);

			// change letters to uppercase			
			for (auto& c : login ) c = toupper(c);
            
			// strip away password after minus
			pos = login.find_first_of("-");
			auto code = login.substr(0, pos);
            
			cout << code << " ";
			oldSellers.push_back(make_tuple(code, login, name));
		}

		cout << endl;
	}
	
	list<string> lines;

	for ( char first : validLetters )
		for ( char second : validLetters )
		{
			string code;
			code += first;
			code += second;

			// see if it is old or new
			auto it = find_if(oldSellers.begin(), oldSellers.end(), [code](const tuple<string,string,string>& i) 
					{return std::get<0>(i) == code;});
			bool old = (it != oldSellers.end());

			// for new codes generate passwords
			string codeWithPassword;
			string name;
			if (old)
			{
				codeWithPassword = std::get<1>(*it);
				name = std::get<2>(*it);
				oldSellers.erase(it);
			}
			else
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

			stringstream s; s << code << ", " << (old ? 1 : 0) << ", " << codeWithPassword << ", " << name << endl;
			lines.push_back(s.str());
		}
	
	// print out remaining sellers that were not written
	// may happen if seller code does contain non-allowed letters
	bool old = true;
	for ( auto seller: oldSellers )
	{
		string code = std::get<0>(seller);
		string codeWithPassword = std::get<1>(seller);
		string name = std::get<2>(seller);
		stringstream s; s << code << ", " << (old ? 1 : 0) << ", " << codeWithPassword << ", " << name << endl;
		lines.push_back(s.str());
	}
	
	// order alphabeticaly including the non-allowed letters sellers
	lines.sort();

	ofstream listFile("codes.csv");
	for ( auto s : lines )
		listFile << s;

	cout << "Generated." << endl;
	return 0;
}
