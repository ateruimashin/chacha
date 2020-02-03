#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <random>
#include <chrono>
#include <bitset>
using namespace std;
using ll = long long;

//出典:https://boringssl.googlesource.com/boringssl/+/master/crypto/chacha/chacha.c
#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d) (			\
	a += b,  d ^= a,  d = ROTL(d,16),	\
	c += d,  b ^= c,  b = ROTL(b,12),	\
	a += b,  d ^= a,  d = ROTL(d, 8),	\
	c += d,  b ^= c,  b = ROTL(b, 7))
#define ROUNDS 20

uint32_t plus32(uint32_t x,uint32_t y){
  return (x+y) & 0xffffffff;
}

array<uint32_t,32> make_array_key(string s){
  array<uint32_t,32> key;
  int count = 0;
  for(int i=0;i < s.size();i+=2){
    string tmp;
    tmp += s[i];
    tmp += s[i+1];
    key[count] = stoi(tmp, nullptr, 16);
    count++;
    tmp.clear();
  }
  return key;
}

array<uint32_t,8> make_array_nonce(string s){
  array<uint32_t,8> nonce;
  int count = 0;
  for(int i=0;i < s.size();i+=2){
    string tmp;
    tmp += s[i];
    tmp += s[i+1];
   nonce[count] = stoi(tmp, nullptr, 16);
    count++;
    tmp.clear();
  }
  return nonce;
}

array<uint32_t, 8> make_block_count(int count){
  array<uint32_t, 8> block_count = {};
  for(int i = 0; i < 8; i++){
    block_count[i] = count / pow(16, 7-i);
    count = count - block_count[i] * pow(16,7-i);
  }
  return block_count;
}

string key_generate(int a, int b, int c, int d){
	string sub_key;
	int bc[4] = {a, b, c, d};
	for(int i = 0; i < 4; i++){
		char c;
		if(bc[i] < 10){
			c = bc[i] + '0';
		}else{
			if(bc[i] == 10)	c = 'a';
			if(bc[i] == 11)	c = 'b';
			if(bc[i] == 12)	c = 'b';
			if(bc[i] == 13)	c = 'd';
			if(bc[i] == 14)	c = 'e';
			if(bc[i] == 15)	c = 'f';
		}
		sub_key.push_back(c);
	}
	return sub_key;
}

string chacha(string key, string nonce, int count) {
  //Initial Stateを作成する。
  array<uint32_t,64>  in = {101, 120, 112, 97,
                                            110, 100, 32 , 51,
                                            50, 45, 98 , 121,
                                            116, 101, 32, 107};
  //↑はconstを2進数にしたものをInital Stateに代入している
  //次の3つはkey,block_count,nonceの配列を作成している。
  array<uint32_t,32> k(make_array_key(key));
  array<uint32_t, 8> block_count(make_block_count(count));
  array<uint32_t, 8> n(make_array_nonce(nonce));

//keyやnonceなどをInital Stateに代入する
  for(int i = 16; i < 64; i++){
    if(i >= 16 && i < 48){
      in[i] = k[i - 16];
    }else if(i >= 48 && i < 56){
      in[i] = block_count[i - 48];
    }else{
      in[i] = n[i - 56];
    }
  }

/*4*4のInital Stateに変換する。64要素あるInital Stateを4要素ずつ取り出し、リトルエンディアンに変換して4*4行列に代入する。
この時、4*4行列ではなく、QRの計算のために1*16行列にする。つまり、このあと計算するのはIn[]ではなく、x[]である。*/
  uint32_t x[16]={};
  for(int i = 0; i < 64; i+=4){
    x[i / 4] = in[i] | (in[i+1] << 8) | (in[i+2] << 16) | (in[i+3] << 24);
  }

  //QR前のx[]をコピーする。
  uint32_t cp[16]={};

  for(int i = 0; i < 16; i++){
		cp[i] = x[i];
	}

  //x[]をQRに通す
  for(int i = 0; i < ROUNDS; i += 2){
		// Odd round
			QR(x[0], x[4], x[ 8], x[12]); // column 0
			QR(x[1], x[5], x[ 9], x[13]); // column 1
			QR(x[2], x[6], x[10], x[14]); // column 2
			QR(x[3], x[7], x[11], x[15]); // column 3
		// Even round
		QR(x[0], x[5], x[10], x[15]); // diagonal 1 (main diagonal)
		QR(x[1], x[6], x[11], x[12]); // diagonal 2
		QR(x[2], x[7], x[ 8], x[13]); // diagonal 3
		QR(x[3], x[4], x[ 9], x[14]); // diagonal 4
  }

  //QR前x[]とQR後x[]を2^32を法とする加算を行う。
  for(int i = 0; i < 16; i++){
    x[i] = plus32(x[i], cp[i]);
  }

  //リトルエンディアンを逆変換する
  uint32_t result[64]={};
  for(int i = 0; i < 64; i +=4){
    result[i]		= (x[i/4] & 0xff);
    result[i+1] = ((x[i/4] >>  8) & 0xff);
    result[i+2] = ((x[i/4] >> 16) & 0xff);
    result[i+3] = ((x[i/4] >> 24) & 0xff);
  }

//16進数のstring型に変換する。これがkey_steam。
	string key_stream;
  for(int i = 0; i < 64; i++){
    stringstream ss;
		ss << hex << result[i];

		if(ss.str().size() != 2 ){
			string s = '0'+ss.str();
			key_stream += s;
		}else{
			key_stream += ss.str();
		}
  }
  return key_stream;
}


//初期keyとnonceをランダムに作成
string make_key(int moji){
	string key;
	random_device rnd;
	mt19937 mt(rnd()+moji);
	uniform_int_distribution<> rand16(0, 15);
	for(int i = 0; i < moji; i++){
		int tmp = rand16(mt);
		char word;
		switch (tmp) {
			case 10:
				word = 'a';
				break;
			case 11:
				word = 'b';
				break;
			case 12:
				word = 'c';
				break;
			case 13:
				word = 'd';
				break;
			case 14:
				word = 'd';
				break;
			case 15:
				word = 'e';
				break;
			default:
				word = tmp + '0';
				break;
		}
		key.push_back(word);
	}
	return key;
}

//16進数string→2進数string
string convert (string stream){
  string k = stream;
  string k_2;
  for(int i = 0; i <128; i++){
    char tmp = k[i];
    switch(tmp){
      case '0':
        k_2 += "0000";
        break;
      case '1':
        k_2 += "0001";
        break;
      case '2':
        k_2 += "0010";
        break;
      case '3':
        k_2 += "0011";
        break;
      case '4':
        k_2 += "0100";
        break;
      case '5':
        k_2 += "0101";
        break;
      case '6':
        k_2 += "0110";
        break;
      case '7':
        k_2 += "0111";
        break;
      case '8':
        k_2 += "1000";
        break;
      case '9':
        k_2 += "1001";
        break;
      case 'a':
        k_2 += "1010";
        break;
      case 'b':
        k_2 += "1011";
        break;
      case 'c':
        k_2 += "1100";
        break;
      case 'd':
        k_2 += "1101";
        break;
      case 'e':
        k_2 += "1110";
        break;
      case 'f':
        k_2 += "1111";
        break;
      default:
        break;
    }
  }
  return k_2;
}

//出力ファイル名を生成する
string make_filename(int a){
	string s = "result";
	string num = to_string(a+1);
	s = s + num + ".txt";
	return s;
}

int main(int argc, char const *argv[]) {
  string key, nonce, key_stream;

  //時刻計測に必要なもの
  chrono::system_clock::time_point	start, end;


  //ループ回数
  int loop_max = pow(10, 1);

  //1000回結果を求める
  for(int loop = 0; loop < loop_max; loop++){

    //時間計測開始
    start = chrono::system_clock::now();

    key = make_key(64);
    nonce = make_key(16);

    //初期ブロックカウント
    int block_count = 0;

    //解析するkey_stream
    string f_key_stream;

    //10^6bit以上のkey streamを作成
    for(int i = 0; i < 2000; i++){
     key_stream = chacha(key, nonce, block_count);
     string binary_key_stream = convert(key_stream);
     f_key_stream += binary_key_stream;
     block_count++;
    }

    //key_streamを500通りに切り出し
    vector<string> split_string;
    for(int i = 0; i < f_key_stream.size(); i += 2048){
     string tmp;
     tmp = f_key_stream.substr(i, 2048);
     split_string.push_back(tmp);
    }

    //テンプレートを作成
    string temp[1024] ={};
    for(int i = 0; i < 1024; i++){
     stringstream ss;
     ss << bitset<10>(i);
     temp[i] = ss.str();
    }

    cout << "Ready" << endl;

    //解析
    int result[1024][6]={{},{}};
    for(int i = 0; i < 1024; i++){                    //テンプレート数
     for(int j = 0; j < 500; j++){                    //分割数
       int count = 0;
       for(int k = 0; k < 2038; k++){           //分割中の探索開始位置
         string tmp;
         tmp = split_string[j].substr(k, 10);
         if(temp[i] == tmp) count++;
       }
       if(count >= 5){
         result[i][5]++;
       }else{
         result[i][count]++;
       }
     }
    }

    string filename = make_filename(loop);

    ofstream writing_file;
    writing_file.open(filename, ios::app);

    for(int i = 0; i < 1024; i++){
     writing_file << temp[i] << endl;
     for(int j = 0; j < 6; j++){
       writing_file << result[i][j] <<" " << endl;
     }
    }
    cout << loop <<"th loop was finished!" << endl;
    end = chrono::system_clock::now();
    auto time = chrono::duration_cast<chrono::seconds>(end - start).count();
    cout << "time is " <<time << "s" <<endl;
    cout << "left" << loop_max - loop - 1 << endl;
  }
 return 0;
}
