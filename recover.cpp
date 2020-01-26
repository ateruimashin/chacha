#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

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

//16進数char→10進数int変換
int conversion_16char_to_10int(char c){
	int num;
	switch (c) {
		case 'a':
			num = 10;
			break;
		case 'b':
			num = 11;
			break;
		case 'c':
			num = 12;
			break;
		case 'd':
			num = 13;
			break;
		case 'e':
		 num = 14;
		 break;
		case 'f':
			num = 15;
			break;
		default:
			num = c - '0';
			break;
	}
	return num;
}

int main(int argc, char const *argv[]) {
  //ファイル入出力準備
  ifstream  reading_file;
  ofstream  writing_file;

	//最頻値key stream
	string key_stream = "377c8d9b7f932b8824d93fd5b7ffbf5562d2f42c6dba6ed39ddebf896d6e16ddf7669ba01231382bf1419a9bc128cc5acb461c8f1f029743aae499a4b64e6ace";

	//平文回復できた個数を保存する配列
	double counter[128]={};

	//平文を保存する
	string planetext[256];
	reading_file.open("planetext.txt", ios::in);
	for(int i = 0; i < 256; i++){
		string tmp;
		getline(reading_file, tmp);
		vector<string> v = split(tmp);
		planetext[i] = v[1];
	}
	reading_file.close();

	//暗号文のbyteごとの最頻値を出す
  for(int sample = 0; sample < 256; sample++){
    string filename = make_filename(sample);
    reading_file.open(filename, ios::in);

    //各byteごとの最頻値を保存する配列と平文
    int max_crypto[128] = {};

    //最頻値を出す
      for(int byte = 0; byte < 128; byte++){

				int max = 0;	//最大値出す時に使う

        for(int line = 0; line < 17; line++){
          if(line == 0){
						string waste;
            getline(reading_file, waste); //個別の上1行は破棄
          }else{
            string tmp;
            getline(reading_file, tmp);
						vector<string> v = split(tmp);	//value部分だけを取得

						//stoiは例外処理を書かないとコンパイルエラーを吐くので
						int count,value;
						try{
							value	= stoi(v[0]);
							count = stoi(v[1]);
						}
						catch(const invalid_argument& e){
							cout << "illegal string Error" << endl;
						}
						catch(const out_of_range & e){
							cout<< "Out of range" << endl;
						}

						//最頻値を出す
						if(count > max){
							max_crypto[byte] = value;
							max = count;
						}
        }
      }
    }
		for(int i = 0; i < 128; i++){
			string one_planetext = planetext[sample];
			int p = conversion_16char_to_10int(one_planetext[i]);
			int k =	conversion_16char_to_10int(key_stream[i]);
			int c =max_crypto[i];
			int xor_p = k ^ c;
			if(xor_p == p)	counter[i]++;
		}
		reading_file.close();
  }

	writing_file.open("recover.txt");
	for(int i = 0; i < 128; i++){
		writing_file << "byte_position " << i << " ";
		writing_file << "counter " <<	counter[i] << " ";
		writing_file << "probability " << counter[i] / 256 <<endl;
	}
	
  return 0;
}
