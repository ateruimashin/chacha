#include <iostream>
#include <string>
#include <array>
#include <cstring>

using namespace std;

//出典:https://boringssl.googlesource.com/boringssl/+/master/crypto/chacha/chacha.c
#define ROTL(a,b) ((a) << (b)) | ((a) >> (32-(b) ))
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
  for(int i=0;i<s.size();i+=2){
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
  array<uint32_t,8> key;
  int count = 0;
  for(int i=0;i<s.size();i+=2){
    string tmp;
    tmp += s[i];
    tmp += s[i+1];
    key[count] = stoi(tmp, nullptr, 16);
    count++;
    tmp.clear();
  }
  return key;
}


void chacha(string key, string nonce) {

//debug
cout<<"key="<<key<<"&nonce="<<nonce<<endl;

  //Initial Stateを作成する。
  array<uint32_t,32>  in = {101, 120, 112, 97,
                                            110, 100, 32 , 51,
                                            50, 45, 98 , 121,
                                            116, 101, 32, 107};
  //↑はconstを2進数にしたものをInital Stateに代入している
  //次の3つはkey,block_count,nonceの配列を作成している。
  array<uint32_t,32> k(make_array_key(key));
  array<uint32_t, 8> block_count = {0, 0, 0, 0, 0, 0, 0, 0};
  array<uint32_t, 8> n(make_array_nonce(nonce));

  //debug
  cout<<"key作成終了"<<endl;

  //上で作成したkey,block_count,nonceをInital Stateに代入。
  //この時、Initial Stateは64要素ある。
  for(int i = 16; i < 64; i++){
		//debug
		cout<<"i="<<i<<endl;
    if(i >= 16 && i < 48){
      in[i] = k[i - 16];
      //debug
      cout<<1<<endl;
    }else if(i >= 48 && i < 56){
      in[i] = block_count[i - 48];
      //debug
      cout<<2<<endl;
    }else{
      in[i] = n[i - 56];
      //debug
      cout<<3<<endl;
    }
  }

//debug
cout<<"1*64のInital State完成"<<endl;

/*4*4のInital Stateに変換する。64要素あるInital Stateを4要素ずつ取り出し、リトルエンディアンに変換して4*4行列に代入する。
この時、4*4行列ではなく、QRの計算のために1*16行列にする。つまり、このあと計算するのはIn[]ではなく、x[]である。
また、これはリトルエンディアンにしている。*/
  uint32_t x[16]={};
  for(int i = 0; i < 64; i++){
    x[i / 4] = in[i] | (in[i+1] << 8) | (in[i+2] << 16) | (in[i+3] << 24);
  }

  //debug
  cout<<"4*4のInital State完成"<<endl;

  //QR前のx[]をコピーする。
  uint32_t cp[16]={};
  memcpy(cp, x, sizeof(x));

  //debug
  cout<<"コピー完了"<<endl;

  //x[]をQRに通す
  for(int i = 0; i < ROUNDS; i += 2){
    // Odd round
		QR(x[ 0], x[ 4], x[ 8], x[12]);	// column 1
		QR(x[ 5], x[ 9], x[13], x[ 1]);	// column 2
		QR(x[10], x[14], x[ 2], x[ 6]);	// column 3
		QR(x[15], x[ 3], x[ 7], x[11]);	// column 4
		// Even round
		QR(x[ 0], x[ 1], x[ 2], x[ 3]);	// row 1
		QR(x[ 5], x[ 6], x[ 7], x[ 4]);	// row 2
		QR(x[10], x[11], x[ 8], x[ 9]);	// row 3
		QR(x[15], x[12], x[13], x[14]);	// row 4
  }

  //debug
  cout<<"QR終了"<<endl;

  //QR前x[]とQR後x[]を2^32を法とする加算を行う。
  for(int i = 0; i < 16; i++){
    x[i] = plus32(x[i], cp[i]);
  }

  //debug
  cout<<"2^32加算終了"<<endl;

  //リトルエンディアンを逆変換する
  uint32_t result[64]={};
  for(int i = 0; i < 64; i ++){
    result[i] =  (x[i] & 0xff);
    result[i] = ((x[i] >> 8) & 0xff);
    result[i] = ((x[i] >>16) & 0xff);
    result[i] = ((x[i] >> 24) & 0xff);
  }

  //debug
  cout<<"逆リトルエンディアン終了"<<endl;

//16進数のstring型に変換する。これがkey_steam。
  for(int i = 0; i < 64; i++){
    cout<<hex<<result[i];
  }
  cout<<endl;
}

int main(int argc, char const *argv[]) {
  string key, nonce;
  cout << "key?" << endl;
  key = "0000000000000000000000000000000000000000000000000000000000000000";
  cout << "nonce" << endl;
  nonce = "0000000000000000";
  chacha(key, nonce);
  return 0;
}
