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

map<int,map<int,int> > color;

//RGB to gray level
void gray(Image input)
{
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
		{
			//gray = 0.114*B + 0.587*G + 0.299*R 
			color[y][x/3]
			=input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]
			=int(0.114*input.term[y*input.rowsize+x]+0.587*input.term[y*input.rowsize+x+1]+0.299*input.term[y*input.rowsize+x+2]);	
				
		}	
}

//Gaussian 3*3 filter
int Gaussian3[3][3]=	//16
{
	1,2,1,
	2,4,2,
	1,2,1,
};

//noise removal
void filter(Image input,int n)
{
	//convolution 
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
		{
			int sum=0;
			for(int dy=-(n/2),i=0;dy<=n/2;dy++,i++)
				for(int dx=-(n/2),j=0;dx<=n/2;dx++,j++)
				{
						// Image * filter 
						sum+=color[y+dy][x/3+dx]*Gaussian3[i][j];
				}
			color[y][x/3]=input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]=int(sum/16);
		}
}

int sobel_dx[3][3]=
{
	-1,0,1,
	-2,0,2,
	-1,0,1,
};

int sobel_dy[3][3]=
{
	-1,-2,-1,
	0,0,0,
	1,2,1,
};

struct G
{
	int magnitude;	//gradient magnitude
	int part;		//partition of gradient
};

G g[10000][10000];

//edge detection
void sobel(Image input)
{
	double pi=3.1415926;
	//gradient
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
		{
			double gx=0,gy=0;
			for(int dx=-1,i=0;dx<=1;dx++,i++)
				for(int dy=-1,j=0;dy<=1;dy++,j++)
				{
					gx+=color[y+dy][x/3+dx]*sobel_dx[i][j];
					gy+=color[y+dy][x/3+dx]*sobel_dy[i][j];
				}
			//magnitude = sqrt(gx^2+gy^2)
			input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]
			=g[y][x/3].magnitude=int(sqrt(gx*gx+gy*gy));
			
			//arctan
			int theta=atan(gy/gx)*180/pi;
			theta=theta%180;
			if(theta<=22.5 || theta>157.5) 		g[y][x/3].part=0;	//0 degree
			else if(theta<=67.5)				g[y][x/3].part=1;	//45 degree
			else if(theta<=112.5)				g[y][x/3].part=2;	//90 degree
			else if(theta<=157.5)				g[y][x/3].part=3;	//135 degree		
		}	
}

//non-maximal supression
void supression(Image input)
{
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
		{
			//if it's not the maximal between the positive and negative pixels 
			//goal: keep the maximal pixels
			switch(g[y][x/3].part)
			{
				case 0:	//0 degree
					if(g[y][x/3].magnitude<g[y][x/3-1].magnitude || g[y][x/3].magnitude<g[y][x/3+1].magnitude)	
						g[y][x/3].magnitude=0;
					break;
				case 1:	//45 degree
					if(g[y][x/3].magnitude<g[y-1][x/3-1].magnitude || g[y][x/3].magnitude<g[y+1][x/3+1].magnitude)	
						g[y][x/3].magnitude=0;
					break;
				case 2:	//90 degree
					if(g[y][x/3].magnitude<g[y-1][x/3].magnitude || g[y][x/3].magnitude<g[y+1][x/3].magnitude)	
						g[y][x/3].magnitude=0;
					break;
				case 3:	//135 degree
					if(g[y][x/3].magnitude<g[y-1][x/3+1].magnitude || g[y][x/3].magnitude<g[y+1][x/3-1].magnitude)	
						g[y][x/3].magnitude=0;
					break;
			}
			input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]
			=g[y][x/3].magnitude;
		}
} 

//double threshold
void db_threshold(Image input,int high,int low)
{
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
		{
			//if "magnitude<low" OR "low<magnitude<high && his nieghbors are all < high", then it's not edge (should remove them)
			if(g[y][x/3].magnitude<low)	g[y][x/3].magnitude=0;
			else if(g[y][x/3].magnitude<high && g[y][x/3].magnitude>low)
			{
				bool edge=false;
				for(int dx=-1;dx<=1;dx++)
					for(int dy=-1;dy<=1;dy++)
						if(g[y+dy][x/3+dx].magnitude>high)
						{
							edge=true;
							break;
						}
				if(!edge)	g[y][x/3].magnitude=0;
			}
			input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]
			=g[y][x/3].magnitude;			
		}
}

//reverse the black and white
void reverse(Image input)
{
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
		{
			if(input.term[y*input.rowsize+x]==0)
				input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]=255;
			else
				input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]=0;
		}		
}

int main()
{ 
	system("mode con: lines=10 cols=50");
	Image input;
	int high,low;
	char image[80];
	printf("Input an image : ");
	scanf("%s",image);
	input.load(image);
	printf("high, low threshold: ");
	scanf("%d,%d",&high,&low);
	//canny: gray -> Gaussian -> sobel -> supression -> db_threshold
	gray(input);					input.save("1_gray.bmp");
	filter(input,3);				input.save("2_noise_removal.bmp");
	sobel(input);					input.save("3_sobel.bmp");
	supression(input);				input.save("4_supression.bmp");
	db_threshold(input,high,low);	input.save("5_db_threshold.bmp");
	reverse(input);					input.save("reverse.bmp");
	
	system("Pause");
	return 0;
}

