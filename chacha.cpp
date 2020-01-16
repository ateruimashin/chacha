#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <random>
#include <omp.h>
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

//平文をランダムに作成
string make_planetext(){
	string planetext;
	random_device rnd;
	mt19937 mt(rnd());
	uniform_int_distribution<> rand16(0, 15);
	for(int i = 0; i < 128; i++){
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
		planetext.push_back(word);
	}
	return planetext;
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

//10進数int→16進数char変換
int conversion_10int_to_16char(int n){
char c;
	switch (n) {
		case 10:
			c = 'a';
			break;
		case 11:
			c = 'b';
			break;
		case 12:
			c = 'c';
			break;
		case 13:
			c = 'd';
			break;
		case 14:
			c = 'e';
			break;
		case 15:
			c = 'f';
			break;
		default:
			c = n + '0';
			break;
	}
	return c;
}

//暗号文作成
string make_cryptogram(string p, string k){
	string crypto;
	int num_p=0,num_k=0;
	for(int i = 0; i < 128; i++){
		num_p = conversion_16char_to_10int(p[i]);
		num_k = conversion_16char_to_10int(k[i]);
		int num = num_p ^ num_k;
		char c = conversion_10int_to_16char(num);
		crypto.push_back(c);
	}
	return crypto;
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
 ll max_size = pow(2, 32);

	//key streamと暗号文を作成し、256個のファイルを出力する
	for(int q = 0; q < 256; q++){

		//出力ファイル名を指定
		string filename = make_filename(q);

		//key streamと暗号文のbyteごとの出力をカウントする配列を作成し初期化
		//counterがkey stream用、c_counterが暗号文用
		ll counter[128][16][1],c_counter[128][16][1];
		for(int i = 0; i < 128; i++){
			for(int j = 0; j < 16; j++){
					counter[i][j][0] = 0;
					c_counter[i][j][0] = 0;
			}
		}

		string plane_text = make_planetext();

		omp_set_num_threads(32);

		//スレッド数
		int n = omp_get_max_threads();
		cout<<"threads:"<<n<<endl;

		ofstream	writing_file;
		writing_file.open("cry.txt", ios::app);
		writing_file << "planetext:" << plane_text << endl;

		#pragma omp parallel for  private(key_stream)
		for(ll i = 0; i < max_size; i++){
			key_stream = chacha(key, nonce);	//key streamの生成

			//key streamの各byteごとの出力をカウントする
			for(int position = 0; position < 128; position++){
				char v = key_stream[position];
				int value = conversion_16char_to_10int(v);
				#pragma omp atomic
				counter[position][value][0]++;
			}

			//暗号文を作成
			string cryptogram;
			cryptogram =	make_cryptogram(plane_text, key_stream);

			//暗号文の各byteごとの出力をカウントする
			for(int position = 0; position < 128; position++){
				char v = key_stream[position];
				int value = conversion_16char_to_10int(v);
				#pragma omp atomic
				c_counter[position][value][0]++;
			}

			//次のkeyとnonceを作成
			string second_key = next_key(key_stream);
			string second_nonce = next_nonce(key_stream);
			key = second_key;
			nonce = second_nonce;
		}


		//key_stream解析結果出力
			for(int w = 0; w < 128; w++){
				ofstream	writing_file;
				writing_file.open(filename, ios::app);
				writing_file << "key stream解析結果" << endl;
				writing_file << "byte_position:" << w << endl;
				for(int v = 0; v < 16; v++){
					writing_file << "value:" << v << " count:" << counter[w][v][0]<< endl;
				}
			}

			//暗号文解析結果出力
			for(int w = 0; w < 128; w++){
				ofstream	writing_file;
				writing_file.open(filename, ios::app);
				writing_file << "暗号文解析結果" << endl;
				writing_file << "byte_position:" << w << endl;
				for(int v = 0; v < 16; v++){
					writing_file << "value:" << v << " count:" << c_counter[w][v][0]<< endl;
				}
			}
			cout << "End of analyzing" << (q+1) << "th key stream!" << endl;
		}

	//やはりミクサはかわいい！
  cout << "End of  All" << endl;
	puts("｡оО(｡´•ㅅ•｡)Оо｡おつかれ");
  return 0;
}
