#include <iostream>
#include <fstream>  // file processing  
#include <stdlib.h>
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

typedef unsigned char   BYTE;   //  1 byte (0~255) 
typedef unsigned short  WORD;   //  2 bytes (0~65536) 
typedef unsigned long   DWORD;  //  4 bytes (0~2^32 -1) 

// int -> 4 bytes
#pragma pack(push) 
#pragma pack(2)
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
#pragma pack(pop)        

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

void gray2bin(Image input)
{
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
		{
			if(input.term[y*input.rowsize+x]>128 && input.term[y*input.rowsize+x+1]>128 && input.term[y*input.rowsize+x+2]>128)
				input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]=255;
			else
				input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]=0;
		}
}

/*
 morphological processing
 1. It is related to the shape or morphology of features in an image
 2. to enhance features or textures of the image according to their requirements
 3. 4 major operations (dilation, erosion, opening, closing)
 4. dilation -> to enhance the detected edges
 5. erosion ->  to reduce some noises in images
 6. opening = erosion then dilation	=> example: analysis of fingerprint
 7. closig = dilation then erosion	=> example: painting restoration 
*/   

void dilation(Image input)
{
	//initialize to white image
	Image output(input.height,input.width);
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
			output.term[y*output.rowsize+x]=output.term[y*output.rowsize+x+1]=output.term[y*output.rowsize+x+2]=255;
	
	//dilation		
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
			for(int dy=-1;dy<=1;dy++)
				for(int dx=-3;dx<=3;dx+=3)
				{
					if(y+dy<0 || y+dy>input.height || x+dx<0 || x+dx>input.rowsize) continue;
					/*****************************************************************************************
						xxx
						x x   =>  eight neighboring pixels
						xxx				
					
						if one of the above pixels is "black", then we set the central pixel to "black"
					*****************************************************************************************/
					if(input.term[(y+dy)*input.rowsize+(x+dx)]==0)
					{
						output.term[y*output.rowsize+x]=output.term[y*output.rowsize+x+1]=output.term[y*output.rowsize+x+2]=0;
						break;
					}
				}
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
			input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]=output.term[y*output.rowsize+x];
}

void erosion(Image input)
{
	//initialize to black image
	Image output(input.height,input.width);
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
			output.term[y*output.rowsize+x]=output.term[y*output.rowsize+x+1]=output.term[y*output.rowsize+x+2]=0;
	
	//erosion		
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
			for(int dy=-1;dy<=1;dy++)
				for(int dx=-3;dx<=3;dx+=3)
				{
					if(y+dy<0 || y+dy>input.height || x+dx<0 || x+dx>input.rowsize) continue;
					/*****************************************************************************************
						xxx
						x x   =>  eight neighboring pixels
						xxx				
					
						if one of the above pixels is "white", then we set the central pixel to "white"
					*****************************************************************************************/
					if(input.term[(y+dy)*input.rowsize+(x+dx)]==255)
					{
						output.term[y*output.rowsize+x]=output.term[y*output.rowsize+x+1]=output.term[y*output.rowsize+x+2]=255;
						break;
					}
				}
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
			input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]=output.term[y*output.rowsize+x];
}

int main()
{ 
	Image Erosion, Dilation, Opening, Closing;
	Erosion.load("finger.bmp");
	Dilation.load("finger.bmp");
	Opening.load("finger.bmp");
	Closing.load("painting.bmp");
	
	//example of erosion
	erosion(Erosion);
	Erosion.save("_erosion.bmp");
	
	//example of dilation
	dilation(Dilation);
	Dilation.save("_dilation.bmp");	
	
	//example of opening
	erosion(Opening);
	dilation(Opening);
	Opening.save("_opening.bmp");	
	
	//example of closing
	for(int i=0;i<4;i++)	dilation(Closing);
	for(int i=0;i<4;i++)	erosion(Closing);
	Closing.save("_closing.bmp");
	
	system("Pause");
	return 0;
}

