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
#include <iomanip>
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

array<uint32_t,12> make_array_nonce(string s){
  array<uint32_t,12> nonce;
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

array<uint32_t, 4> make_block_count(int count){
  array<uint32_t, 4> block_count;
	stringstream ss;
	ss << hex << count;
	string block = ss.str();

	string b;

	if(block.size() != 8){
		string zero = {};
		for(int z = 0; z < 8 - block.size(); z++){
			zero.push_back('0');
		}
		cout<<"z " << zero << endl;
		block = zero + block;
	}

cout << "block=" << block << endl;

	for(int i=0;i < block.size();i+=2){
    string tmp;
    tmp += block[i];
    tmp += block[i+1];
   block_count[i/2] = stoi(tmp, nullptr, 16);
    count++;
    tmp.clear();
  }
  return block_count;
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
  array<uint32_t, 4> block_count(make_block_count(count));
  array<uint32_t, 12> n(make_array_nonce(nonce));

//keyやnonceなどをInital Stateに代入する
  for(int i = 16; i < 64; i++){
    if(i >= 16 && i < 48){
      in[i] = k[i - 16];
    }else if(i >= 48 && i < 52){
      in[i] = block_count[i - 48];
    }else{
      in[i] = n[i - 52];
    }
  }

/*4*4のInital Stateに変換する。64要素あるInital Stateを4要素ずつ取り出し、リトルエンディアンに変換して4*4行列に代入する。
この時、4*4行列ではなく、QRの計算のために1*16行列にする。つまり、このあと計算するのはIn[]ではなく、x[]である。*/
  uint32_t x[16]={};
  for(int i = 0; i < 64; i+=4){
	   x[i / 4] = in[i] | (in[i+1] << 8) | (in[i+2] << 16) | (in[i+3] << 24);
  }

	x[12] = count;	//何故かblock countだけリトルエンディアンに変換しないので、あとで上書きする。

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
	for(int i = 0; i < moji; i++){
		rand();	rand();	rand();	rand();	rand();	//rand関数を5回空回しする
		int tmp = rand() % 16;
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


//出力ファイル名を生成する
string make_filename(int a){
	string s = "test";
	string num = to_string(a+1);
	s = s + num + ".txt";
	return s;
}

int main(int argc, char const *argv[]) {
  string key, nonce, key_stream;

  //ループ回数
  int loop_max = pow(10, 0);

  //1000回結果を求める
  for(int loop = 0; loop < loop_max; loop++){
    //初期ブロックカウントは1
    int block_count = 1;

    //解析するkey_stream
    string f_key_stream;

    //1028016bits以上のkey streamを作成
    for(int i = 0; i < 2024; i++){
		key = "23AD52B15FA7EBDC4672D72289253D95DC9A4324FC369F593FDCC7733AD77617";
		nonce = "5A5F6C13C1F12653";
     key_stream = chacha(key, nonce, block_count);
     f_key_stream += key_stream;
     block_count++;
    }

		string filename = "test1.txt";

    ofstream writing_file;
    writing_file.open(filename, ios::app);
    writing_file << f_key_stream << endl;

     }
  return 0;
}
