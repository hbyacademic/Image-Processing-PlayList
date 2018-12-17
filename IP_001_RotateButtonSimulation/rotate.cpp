#include <iostream>
#include <fstream>  // file processing  
#include<vector>
using namespace std;

typedef unsigned char   BYTE;   //  1 byte (0~255) 
typedef unsigned short  WORD;   //  2 bytes (0~65536) 
typedef unsigned long   DWORD;  //  4 bytes (0~2^32 -1) 

// int -> 4 bytes
 
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

Image newstore(Image input,char ctrl) 
{

	Image output=(ctrl=='R' || ctrl=='L')?Image(input.width,input.height):Image(input.height,input.width);
		
	if(ctrl=='H')	//Y-mirror
	{
		for(int y=0;y<input.height;y++)
			for(int x=0;x<input.rowsize;x+=3)
			{ 
				output.term[y*input.rowsize+input.rowsize-3-x]=BYTE(input.term[y*input.rowsize+x]);
				output.term[y*input.rowsize+input.rowsize-3-x+1]=BYTE(input.term[y*input.rowsize+x+1]);
				output.term[y*input.rowsize+input.rowsize-3-x+2]=BYTE(input.term[y*input.rowsize+x+2]);		
			}
	}
		
	else if(ctrl=='V')	//X-mirror
	{
		for(int y=0;y<input.height;y++)
			for(int x=0;x<input.rowsize;x+=3)
			{ 
				output.term[(input.height-1-y)*input.rowsize+x]=BYTE(input.term[y*input.rowsize+x]);
				output.term[(input.height-1-y)*input.rowsize+x+1]=BYTE(input.term[y*input.rowsize+x+1]);
				output.term[(input.height-1-y)*input.rowsize+x+2]=BYTE(input.term[y*input.rowsize+x+2]);		
			}
	}
		
	else if(ctrl=='T')	//L180
	{
		for(int y=0;y<input.height;y++)
			for(int x=0;x<input.rowsize;x++)
				output.term[(input.height-1-y)*input.rowsize+(input.width-1-x/3)*3+x%3]=BYTE(input.term[y*input.rowsize+x]);
	}
		
	else if(ctrl=='R')	//R90
	{
		for(int y=0;y<input.height;y++)
			for(int x=0;x<input.rowsize;x++)
				output.term[3*input.height*(input.width-x/3-1)+(3*y+x%3)]=BYTE(input.term[y*input.rowsize+x]);
	
	}
		
	else if(ctrl=='L')	//L90
	{
		for(int y=0;y<input.height;y++)
			for(int x=0;x<input.rowsize;x++)
				output.term[3*input.height*(x/3)+(input.height-y-1)*3+x%3]=BYTE(input.term[y*input.rowsize+x]);
	}
	
	return output;
}

int main()
{ 
	Image input,output;
	char filename[80];	
	cout<<"Input a file: ";
	cin>>filename;
	input.load(filename);	
	
	char str;
	cout<<"L(l90deg), R(r90deg), T(r180deg), V(vertical), H(horizontal), Q(quit)\n";
	cout<<"control: ";
	while(cin>>str)
	{
		if(str=='Q') break; 
		cout<<"control: ";
		output=(str=='R' || str=='L')?Image(input.width,input.height):Image(input.height,input.width);
		output=newstore(input,str);
		output.save("output.bmp");
		input=output;
	}

	system("Pause");
	return 0;
}

