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

//次のkeyとnonceを作成する。
string next_key(string key){
  string n_key;
  for(int i = 0; i< 64; i++){
    n_key.push_back(key[i]);
  }
  return n_key;
}

string next_nonce(string key){
  string n_nonce;
  for(int i = 0; i < 16; i++){
    n_nonce.push_back(key[i+110]);
  }
  return n_nonce;
}

//出力ファイル名を生成する
string make_filename(int a){
	string s = "result";
	string num = to_string(a+1);
	s = s + num + ".txt";
	return s;
}

string chacha(string key, string nonce) {
  //Initial Stateを作成する。
  array<uint32_t,64>  in = {101, 120, 112, 97,
                                            110, 100, 32 , 51,
                                            50, 45, 98 , 121,
                                            116, 101, 32, 107};
  //↑はconstを2進数にしたものをInital Stateに代入している
  //次の3つはkey,block_count,nonceの配列を作成している。
  array<uint32_t,32> k(make_array_key(key));
  array<uint32_t, 8> block_count = {0, 0, 0, 0, 0, 0, 0, 0};
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

int main(int argc, char const *argv[]) {
  string key, nonce, key_stream;

  cout<<"keyの初期値(0を入れると0~0になります。1を入れるとランダムなkeyを生成します。)"<<endl;
  cin>>key;
  if(key == "0")  key = "0000000000000000000000000000000000000000000000000000000000000000";
	if(key == "1")	key = make_key(64);
  cout<<"nonceの初期値(0を入れると0~0になります。1を入れるとランダムなnonceを生成します。)"<<endl;
  cin>>nonce;
  if(nonce == "0")  nonce = "0000000000000000";
	if(nonce == "1")	nonce = make_key(16);

	cout<<key<<endl;
	cout<<nonce<<endl;

//key stream生成個数を設定
 ll max_size = pow(10, 3);

  key_stream = chacha(key, nonce);
  string binary_key_stream = convert(key_stream);


  return 0;
}
