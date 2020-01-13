#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
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
	string s = "result of key stream";
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


int main(int argc, char const *argv[]) {
  string key, nonce, key_stream;

  cout<<"keyの初期値(0を入れると0~0になります)"<<endl;
  cin>>key;
  if(key == "0")  key = "0000000000000000000000000000000000000000000000000000000000000000";
  cout<<"nonceの初期値(0を入れると0~0になります)"<<endl;
  cin>>nonce;
  if(nonce == "0")  nonce = "0000000000000000";

	cout<<"Writing...Please wait..."<<endl;	//実行中何も表示されないと寂しいので

//key stream生成個数を設定
 ll max_size = pow(2, 5);

	//keyを作成して、chacha関数からkey_streamを受け取り、ファイルに出力する。
	for(int q = 0; q < 256; q++){

		//出力ファイル名を指定
		string filename = make_filename(q);

		//key streamのbyteごとの出力をカウントする配列を作成し初期化
		ll counter[128][16][1];
	  for(int i = 0; i < 128; i++){
	    for(int j = 0; j < 16; j++){
	        counter[i][j][0] = 0;
	    }
	  }

		for(ll i = 0; i < max_size; i++){
			key_stream = chacha(key, nonce);	//key streamの生成

			//ファイル出力
			// ofstream	writing_file;
			// writing_file.open(filename, ios::app);
			// writing_file << key_stream << endl;

			//key streamの各byteごとの出力をカウントする
			for(int position = 0; position < 128; position++){
				char v = key_stream[position];
				int value;
				switch(v){
					case 'a':
						value = 10;
						break;
					case 'b':
						value = 11;
						break;
					case 'c':
						value = 12;
						break;
					case 'd':
						value =13;
						break;
					case 'e':
						value =14;
						break;
					case 'f':
						value = 15;
						break;
					default:
						value = v - '0';
						break;
				}
				counter[position][value][0]++;
			}

			//次のkeyとnonceを作成
			string second_key = next_key(key_stream);
			string second_nonce = next_nonce(key_stream);
			key = second_key;
			nonce = second_nonce;

			if(i % 10000 == 0)	cout<<"Now,count is "<< i <<endl;	//暇つぶし
		}

		//key streamの偏りの書き出し
		for(int w = 0; w < 128; w++){
			ofstream	writing_file;
			writing_file.open(filename, ios::app);
			writing_file << "byte_position:" << w << endl;
			for(int v = 0; v < 16; v++){
				writing_file << "value:" << v << " count:" << counter[w][v][0]<< endl;
			}
		}
		cout << "End of analyzing" << (q+1) << "th key stream!" << endl;
	}
	//やはりミクサはかわいい！
  cout << "End of  All" << endl;
	puts("｡оО(｡´•ㅅ•｡)Оо｡おつかれ");
  return 0;
}
