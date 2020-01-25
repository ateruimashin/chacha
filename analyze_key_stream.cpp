#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
using namespace std;
using ll = long long int;

//入出力ファイル名を生成する
string make_filename(int a){
	string s = "result of key stream";
	string num = to_string(a+1);
	s = s + num + ".txt";
	return s;
}

// int value_split(string s, int i){
//   int value = 0;
//     if(i < 10){
//       char num = s[6];
//       value = num - '0';
//     }else{
//       string num;
//       num.push_back(s[6]);
//       num.push_back(s[7]);
//       value = stoi(num);
//     }
//     return value;
// }

ll count_split(string s, int i){
  ll count = 0;
  string cou;
  if(i < 10){
    for(int j = 0; j < 7; j++){
      cou.push_back(s[j+14]);
    }
    count = stoi(cou);
  }else{
    for(int j = 0; j < 7; j++){
      cou.push_back(s[j+15]);
    }
    count = stoi(cou);
  }
  return count;
}

int main(int argc, char const *argv[]) {

  //結果出力用配列を初期化
  ll counter[128][16][1];
  for(int i = 0; i < 128; i++){
    for(int j = 0; j < 16; j++){
        counter[i][j][0] = 0;
    }
  }

  //ファイル入力準備
  ifstream reading_file;

  for(int i = 0; i < 256; i++){
    //入力ファイル名作成と読み込み準備
    string filename = make_filename(i);
    reading_file.open(filename, ios::in);

    //あとで必要になるやつ
    ll byte_positions = 0;

    //読み込みとカウンタへ入力
    while(!reading_file.eof()){
      //1行目はbyte_positionなので読み込み
      string byte_position;
      getline(reading_file, byte_position);

      //必要な部分だけ読み出す
      for(ll line = 0; line < 16; line++){
        string tmp;
        getline(reading_file, tmp);
        ll count = count_split(tmp, i);

        //カウンタに代入
        count[byte_positions][line][0] += count;
      }

      //次のbyteへ代入するため
      byte_positions++;

    }//while終わり

  }//256ループ終わり

  return 0;
}
