#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;
using ll = long long;

//入出力ファイル名を生成する
string make_filename(int a){
	string s = "result";
	string num = to_string(a+1);
	s = s + num + ".txt";
	return s;
}

//分割
vector<string> split(string s){
	vector<string> v;
	stringstream ss{s};
	while(getline(ss, s, ' ')){
		v.push_back(s);
	}
	return v;
}

int main(int argc, char const *argv[]) {

  ifstream reading_file;
  ofstream writing_file;

  ll max_crypto[128][16] = {{},{}};

  for(int sample = 0; sample < 256; sample++){

    string filename = make_filename(sample);
    reading_file.open(filename, ios::in);

    for(int n = 0; n < 128; n++){
      for(int line = 0; line < 18; line++){
        int value;
        ll count;
        if(line < 2){
          string waste;
          getline(reading_file, waste);
        }else{
          string tmp;
          getline(reading_file, tmp);
          vector<string> v = split(tmp);
          try{
            value = stoi(v[1]);
            count = stoll(v[3]);
          }
          catch(const invalid_argument& e){
            cout << "illegal string Error" << endl;
          }
          catch(const out_of_range & e){
            cout<< "Out of range" << endl;
          }
          max_crypto[n][value] += count;
        }
      }
    }
    reading_file.close();
  }

  writing_file.open("counter.txt", ios::app);
  for(int i = 0; i < 128; i++){
    writing_file << i << endl;
    for(int j = 0; j < 16; j++){
      writing_file << j << " " << max_crypto[i][j] << endl;
    }
  }
  return 0;
}
