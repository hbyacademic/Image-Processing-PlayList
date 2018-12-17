#include <iostream>
#include <fstream>  // file processing  
#include <stdlib.h>
#include <math.h>
using namespace std;

typedef unsigned char   BYTE;   //  1 byte (0~255) 
typedef unsigned short  WORD;   //  2 bytes (0~65536) 
typedef unsigned long   DWORD;  //4 bytes (0~2^32 -1) 

#pragma pack(push)	//store 
#pragma pack(2)		//2-bytes aligned
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
#pragma pack(pop)  	//restore

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
			for(int y=0; y<height; y++)
				for(int x=0; x<width; x++)
					term[y*rowsize+3*x]=term[y*rowsize+3*x+1]=term[y*rowsize+3*x+2]= 255;
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

Image FSdither(Image original,int colorAmt)
{
	Image input=original;
	//for each y from top to bottom
   	//for each x from left to right
	for(int y=input.height-1; y>0; y--)
		for(int x=1; x<input.width-1; x++)
		{
			//oldpixel  := pixel[x][y]
      		//newpixel  := find_closest_palette_color(oldpixel)
      		
      		#define B(X,Y) input.term[(Y)*input.rowsize+3*(X)]
      		#define G(X,Y) input.term[(Y)*input.rowsize+3*(X)+1]
      		#define R(X,Y) input.term[(Y)*input.rowsize+3*(X)+2]
      		
      		int newB,newG,newR;
      		newB=int(round(colorAmt*B(x,y)*1.0/255)*(255/colorAmt));
      		newG=int(round(colorAmt*G(x,y)*1.0/255)*(255/colorAmt));
      		newR=int(round(colorAmt*R(x,y)*1.0/255)*(255/colorAmt));
			
			//quant_error  := oldpixel - newpixel
			int quantB,quantG,quantR;
			quantB=B(x,y)-newB;
			quantG=G(x,y)-newG;
			quantR=R(x,y)-newR;
		
			//pixel[x][y]  := newpixel
			B(x,y)=newB;
			G(x,y)=newG;
			R(x,y)=newR;
			
			#define pixel_add_quanterror(X,Y,numerator,denominator) \
			B(X,Y)+=(quantB*numerator/denominator); \
			G(X,Y)+=(quantG*numerator/denominator); \
			R(X,Y)+=(quantR*numerator/denominator);
			
			//error diffusion
			pixel_add_quanterror(x+1,y,7,16);
      		pixel_add_quanterror(x-1,y-1,3,16);
      		pixel_add_quanterror(x,y-1,5,16);
     		pixel_add_quanterror(x+1,y-1,1,16);		
		}
		return input;
}

int main()
{
	Image input,output;
	input.load("face.bmp");
	
	int colorAmt[6];
	cout<<"Input the colorAmt n (allowed n*n*n colors): ";
	for(int i=0; i<6 ;i++)	cin>>colorAmt[i];
	output=FSdither(input,colorAmt[0]-1);
	//output.save("output.bmp");
	return 0;
}
