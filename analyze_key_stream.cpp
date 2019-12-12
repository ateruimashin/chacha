#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

int main(int argc, char const *argv[]) {
  string filename = "key_stream_result.txt";
  ifstream reading_file;
  string key_stream;

    //配列の宣言と初期化
  int counter[64][256][1];
  for(int i = 0; i < 64; i++){
    for(int j = 0; j < 256; j++){
        counter[i][j][0] = 0;
    }
  }

  //ファイル入力と各byteごとに出てくる数値のカウント
  int i = 0;
  reading_file.open(filename, ios::in);
  while(i < 65536){
    getline(reading_file,key_stream);
    for(int j = 0; j < key_stream.size(); j += 2){
      string tmp;
      tmp.push_back(key_stream[j]);
      tmp.push_back(key_stream[j+1]);
      long b = stol(tmp , NULL, 16);
      counter[j/2][b][0]++;
    }
    i++;
  }

  //確率の計算とファイル出力
  string filename1 = "result.txt";
  ofstream writing_file;
  writing_file.open(filename1, ios::app);

  double max_max_p=0;
  int max_position=0;
  int max_max_value=0;

  for(int i = 0; i < 64; i++){
    double max_p = 0;
    int max_value = 0;
    for(int j = 0; j < 256; j++){
      double tmp_p;
      tmp_p = (double) counter[i][j][0] / 65536;
      if(max_p < tmp_p) {
        max_p = tmp_p;
        max_value = j;
        if(max_max_p < max_p) {
          max_max_p = max_p;
          max_max_value = j;
          max_position = i;
        }
      }
    }
    writing_file<<"byte_position:"<< i+1 <<endl;
    writing_file<<"max_value:"<< max_value<<endl;
    writing_file<<"probability:"<< max_p<<endl;
    writing_file<<endl;
  }
  writing_file<<"MAX"<<endl;
  writing_file<<"max_position:"<<max_position+1<<endl;
  writing_file<<"max_value:"<<max_max_value<<endl;
  writing_file<<"max_probability"<<max_max_p<<endl;
  writing_file<<endl;
  writing_file<<"END"<<endl;
  return 0;
}
