#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;
using ll = long long int;

//入出力ファイル名を生成する
string make_filename(int a){
	string s = "result";
	string num = to_string(a+1);
	s = s + num + ".txt";
	return s;
}


int main(int argc, char const *argv[]) {
  for(int sample_no = 0; sample_no < 256; sample_no++){
    //結果用の配列の初期化
    ll k_counter[128][16][1];
    for(int i = 0; i < 128; i++){
      for(int j = 0; j < 16; j++){
        k_counter[i][j][0] = 0;
      }
    }
    //入力ファイル名
    string filename = make_filename(sample_no);




  }
  return 0;
}
