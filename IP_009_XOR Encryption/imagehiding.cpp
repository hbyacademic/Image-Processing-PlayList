#include <iostream>
#include <fstream>  // file processing  
#include <stdlib.h>
#include <map>
#include <vector>
#include <math.h>
using namespace std;

typedef unsigned char   BYTE;   //  1 byte (0~255) 
typedef unsigned short  WORD;   //  2 bytes (0~65536) 
typedef unsigned long   DWORD;  //  4 bytes (0~2^32 -1) 

#pragma pack(push)  //在改變為2前先儲存原來的設定 
#pragma pack(2)    //以 2 bytes 對齊 
struct INFO
{
	// BITMAPFILEHEADER (14 bytes) from 16 reducing to 14
	WORD bfType;          //BM -> 0x4d42 (19778)     
	DWORD BfSize;         //總圖檔大小        
	WORD bfReserved1;     //bfReserved1 須為0  
    WORD bfReserved2;     //bfReserved2 須為0        
    DWORD bfOffBits;      //偏移量 
	// BITMAPINFOHEADER(40 bytes)    
    DWORD biSize;         //info header大小     
    int biWidth;
    int biHeight;
    WORD biPlanes;        //位元圖層數=1 
    WORD biBitCount;      //每個pixel需要多少bits 
    DWORD biCompression;  //0為不壓縮 
    DWORD biSizeImage;    //點陣圖資料大小  
    int biXPelsPerMeter;  //水平解析度 
    int biYPelsPerMeter;  //垂直解析度 
    DWORD biClrUsed;      //0為使用所有調色盤顏色 
    DWORD biClrImportant; //重要的顏色數(0為所有顏色皆一樣重要) 
};
#pragma pack(pop)        //恢復原來的設定 

class Image
{	
	public:
		
		int height;
		int width;
		int rowsize;    // bgr -> 3 bytes(24 bits) 
		BYTE* term;
		
		Image()   //storage is bottom-up,from left to right 
		{
			height=0;
			width=0;
		}
		
		Image(int height,int width)
		{
			this->height=height;
			this->width=width;
			rowsize=(3*width+3)/4*4;   //set to be a multiple of "4" 
			term=new BYTE[height*rowsize];  
		}
		
		void load(const char *filename)
		{
			INFO h;  
			ifstream f;
			f.open(filename,ios::in|ios::binary);
			f.seekg(0,f.end);
			//cout<<"圖檔大小： "<<f.tellg()<<"bytes"<<endl;
			f.seekg(0,f.beg);
			f.read((char*)&h,sizeof(h));
						
			width=h.biWidth;
			height=h.biHeight;
			//cout<<"reading from "<<filename<<"..."<<endl;
			//print(h);
			*this=Image(height,width);
			f.read((char*)term,height*rowsize);
			f.close();
		}
		
		void save(const char* filename)
		{
			INFO h=
			{		
				19778,   //0x4d42
				DWORD(54+rowsize*height),   
				0,
				0,
				54,
				40,
				width,
				height,
				1,
				24,   
				0,
				DWORD(rowsize*height),
				3780,   //3780
				3780,   //3780
				0,
				0				
			};
			//cout<<"writing into "<<filename<<"..."<<endl;
			ofstream f;
			f.open(filename,ios::out|ios::binary);
			f.write((char*)&h,sizeof(h));
			f.write((char*)term,rowsize*height);
			f.close();	
		}	
};

Image encode(Image key,Image secret)
{
	Image encoded(key.height,key.width);
	for(int y=0;y<key.height;y++)
		for(int x=0;x<key.rowsize;x+=3)
		{
			// >>5 is equal to /32
			encoded.term[y*key.rowsize+x]=(secret.term[y*key.rowsize+x]>>5)^key.term[y*key.rowsize+x];
			encoded.term[y*key.rowsize+x+1]=(secret.term[y*key.rowsize+x+1]>>5)^key.term[y*key.rowsize+x+1];
			encoded.term[y*key.rowsize+x+2]=(secret.term[y*key.rowsize+x+2]>>5)^key.term[y*key.rowsize+x+2];	
		} 
	return encoded;
}

Image decode(Image key,Image encoded)
{
	Image decoded(key.height,key.width);
	for(int y=0;y<key.height;y++)
		for(int x=0;x<key.rowsize;x+=3)
		{ 
			// <<5 is equal to *32
			decoded.term[y*key.rowsize+x]=(encoded.term[y*key.rowsize+x]^key.term[y*key.rowsize+x])<<5;
			decoded.term[y*key.rowsize+x+1]=(encoded.term[y*key.rowsize+x+1]^key.term[y*key.rowsize+x+1])<<5;
			decoded.term[y*key.rowsize+x+2]=(encoded.term[y*key.rowsize+x+2]^key.term[y*key.rowsize+x+2])<<5;	
		}	
	return decoded;
}

int main()
{
	Image input,secret,encoded,decoded;
	char in[80],s[80];
	
	cout<<"Input a key: ";
	scanf("%s",in);
	input.load(in);
	
	cout<<"Input a secret: ";
	scanf("%s",s);
	secret.load(s);
	
	encoded=encode(input,secret);
	encoded.save("encoded.bmp");
	decoded=decode(input,encoded);
	decoded.save("decoded.bmp");
	return 0;
}
