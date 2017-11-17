#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
using namespace std;

int main() {
	ifstream fin("./data/LAUR060803MX20NODES0TO255EVENTS.csv");
	ofstream fout("./test.txt");
	string s;
	while (getline(fin, s)) {
		fout << s << endl;
		this_thread::sleep_for(chrono::seconds(1));
		cout << "done" << endl;
	}
	fout.close();
	fin.close();
}
