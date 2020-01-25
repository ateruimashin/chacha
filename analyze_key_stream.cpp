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

  //ファイル入出力準備
  ifstream reading_file;
	ofstream writing_file;

  for(int i = 0; i < 256; i++){
    //入力ファイル名作成と読み込み準備
    string filename = make_filename(i);
    reading_file.open(filename, ios::in);

    //あとで必要になるやつ
    int byte_position = 0;

    //読み込みとカウンタへ入力
    while(!reading_file.eof()){
      //1行目はbyte_positionなので読み込み、破棄
      string waste;
      getline(reading_file, waste);

      //必要な部分だけ読み出す
      for(int line = 0; line < 16; line++){
        string tmp;
        getline(reading_file, tmp);
        ll count = count_split(tmp, line);

        //カウンタに代入
        counter[byte_position][line][0] += count;
      }

      //次のbyteへ代入するため
      byte_position++;

    }//while終わり

  }//256ループ終わり

	//確率計算のため、パターンの数を計算しておく。
	ll total = 0;
	for(int i = 0; i < 16; i++){
		total += counter[0][i][0];
	}

	//最頻値と確率計算
	for(int i = 0; i < 128; i++){
		ll max=0;	//各byteの最頻値のカウント数
		int value = 0;	//各byteの最頻値
		for(int j = 0; j < 16; j++){
			if(counter[i][j][0] > max){
				max = counter[i][j][0];
				value = j;
			}
		}
		//出力
		writing_file.open("mode of key stream", ios::app);
		writing_file << "byte_position:" << i <<endl;
		writing_file << value << " " << (double)(max/total) <<endl;	//long longの割り算って少数表示できる？
	}
  return 0;
}
