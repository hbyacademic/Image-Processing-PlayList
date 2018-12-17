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

map<int,map<int,int> >R;
map<int,map<int,int> >G;
map<int,map<int,int> >B;

int Gaussian[7][7]=	//198
{
	1,1,2,2,2,1,1,
	1,3,4,5,4,3,1,
	2,4,7,8,7,4,2,
	2,5,8,10,8,5,2,
	2,4,7,8,7,4,2,
	1,31,4,5,4,3,1,
	1,1,2,2,2,1,1
};
/*
int Gaussian[3][3]=	//16
{
	1,2,1,
	2,4,2,
	1,2,1
};
*/

//lowpass filter
int lpf(int n,int x,int y)
{
	return 1;
}


void median(Image input,int n,int x,int y)
{
	vector<int>seqB,seqG,seqR;
	seqB.clear();
	seqG.clear();
	seqR.clear();
	for(int dy=-(n/2);dy<=n/2;dy++)
	{
		for(int dx=-(n/2);dx<=n/2;dx++)
		{
			seqB.push_back(B[y+dy][x+3*dx]);
			seqG.push_back(G[y+dy][x+3*dx]);
			seqR.push_back(R[y+dy][x+3*dx]);
		}
	}
	sort(seqB.begin(),seqB.end());
	B[y][x]=seqB[n*n/2];
	sort(seqG.begin(),seqG.end());
	G[y][x]=seqG[n*n/2];
	sort(seqR.begin(),seqR.end());
	R[y][x]=seqR[n*n/2];
}

void scan(Image input) 
{
	int h=input.height;
	int row=input.rowsize;
	for(int y=0;y<h;y++)
	{
		for(int x=0;x<row;x+=3)
		{
			B[y][x]=input.term[y*row+x+0];
			G[y][x]=input.term[y*row+x+1];
			R[y][x]=input.term[y*row+x+2];	
		}
	}
}

int nxn(Image input,int n,int x,int y,int bgr,int option)
{
	int sum=0,i=0,j=0;
	if(option==1)
	{
		median(input,n,x,y);
		if(bgr==0)
			sum=(n*n)*B[y][x];
		else if(bgr==1)
			sum=(n*n)*G[y][x];
		else if(bgr==2)
			sum=(n*n)*R[y][x];	
	}
	
	else
	{
		for(int dy=-(n/2);dy<=n/2;dy++,i++)
		{
			for(int dx=-(n/2);dx<=n/2;dx++,j++)
			{
				if(bgr==0)
				{	
					if(option==0)
						sum+=lpf(n,i,j)*B[y+dy][x+3*dx];
					else
						sum+=Gaussian[i][j]*B[y+dy][x+3*dx];
				}
				
				else if(bgr==1)
				{
					if(option==0)
						sum+=lpf(n,i,j)*G[y+dy][x+3*dx];
					else
						sum+=Gaussian[i][j]*G[y+dy][x+3*dx];
				}
				
				else if(bgr==2)
				{
					if(option==0)
						sum+=lpf(n,i,j)*R[y+dy][x+3*dx];
					else
						sum+=Gaussian[i][j]*R[y+dy][x+3*dx];
				}
			} 
		j=0;
		}
	}
	if(option==2)
		return sum/16;
		
	return sum/(n*n);
}

void smooth(Image input,int h,int row,int n,int option)
{
	h=input.height;
	row=input.rowsize;
	for(int y=0;y<h;y++)
	{
		for(int x=0;x<row;x+=3)
		{	
			input.term[y*row+x+0]=BYTE((int(nxn(input,n,x,y,0,option)))%256);
			input.term[y*row+x+1]=BYTE((int(nxn(input,n,x,y,1,option)))%256);
			input.term[y*row+x+2]=BYTE((int(nxn(input,n,x,y,2,option)))%256);
		}
	}
}

int main()
{ 
	Image input,output;
	int n,option;
	char image[80];
	printf("Input an image : ");
	scanf("%s",image);
	input.load(image);
	/* 
	printf("n(mask region) : ");
	scanf("%d",&n);*/ 
	
	printf("option(0:lpf, 1:median, 2:Gaussian) : ");
	scanf("%d",&option);
	for(int i=0;i<10000;i++)
		for(int j=0;j<10000;j++);
/*	
	scan(input);
	smooth(input,input.height,input.rowsize,n,option);
	input.save("output.bmp");*/ 
		
	system("Pause");
	return 0;
}

