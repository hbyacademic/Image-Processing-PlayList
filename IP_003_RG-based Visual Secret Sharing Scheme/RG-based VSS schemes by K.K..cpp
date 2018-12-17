#include <iostream>
#include <fstream>    
#include <time.h>  
#include<stdlib.h>   
#include<cmath>
using namespace std;

typedef unsigned char   BYTE;   //  1 byte  
typedef unsigned short  WORD;   //  2 bytes 
typedef unsigned long   DWORD;  //  4 bytes 
                                

#pragma pack(push)  //在改變為2前先儲存原來的設定 
#pragma pack(2)    //以 2 bytes 對齊 
struct INFO{
	// BITMAPFILEHEADER (14 bytes)   
	WORD bfType;         
	DWORD BfSize;            
	WORD bfReserved1;     
    WORD bfReserved2;        
    DWORD bfOffBits; 
	// BITMAPINFOHEADER(40 bytes)  
    DWORD biSize;             
    int biWidth;
    int biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;     
    DWORD biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;    
};
#pragma pack(pop)        //恢復原來的設定 

void print(INFO h){         //informations of the image 
	cout<<"bfType: "<<h.bfType<<endl;
	cout<<"bfSize: "<<h.BfSize<<endl;
	cout<<"bfReserved1: "<<h.bfReserved1<<endl;
	cout<<"bfReserved2: "<<h.bfReserved2<<endl;
	cout<<"bfOffBits: "<<h.bfOffBits<<endl;
	cout<<"biSize: "<<h.biSize<<endl;
	cout<<"biWidth: "<<h.biWidth<<endl;
	cout<<"biHeight: "<<h.biHeight<<endl;
	cout<<"biPlanes: "<<h.biPlanes<<endl;
	cout<<"biBitCount: "<<h.biBitCount<<endl;
	cout<<"biCompression: "<<h.biCompression<<endl;
	cout<<"biSizeImage: "<<h.biSizeImage<<endl;
	cout<<"biXPelsPerMater: "<<h.biXPelsPerMeter<<endl;
	cout<<"biYPelsPerMeter: "<<h.biYPelsPerMeter<<endl;
	cout<<"biClrUsed: "<<h.biClrUsed<<endl;
	cout<<"biClrImportant: "<<h.biClrImportant<<endl;
}

class ImageMatrix{
	
	public:
		
		int height;
		int width;
		int rowsize;    // rgb佔 3 bytes 
		BYTE* term;
		
		ImageMatrix()   //儲存由"下而上"由左而右  
		{
			height=0;
			width=0;
		}
		
		ImageMatrix(int height,int width)
		{
			this->height=height;
			this->width=width;
			rowsize=(3*width+3)/4*4;
			term=new BYTE[height*rowsize];  
		}
		
		void load(char *filename)
		{
			INFO h;  
			ifstream f;
			f.open(filename,ios::in|ios::binary);
			f.seekg(0,f.end);
			cout<<"圖檔大小： "<<f.tellg()<<"bytes"<<endl;
			f.seekg(0,f.beg);
			f.read((char*)&h,sizeof(h));
						
			width=h.biWidth;
			height=h.biHeight;
			cout<<"reading from "<<filename<<"..."<<endl;
			print(h);
			*this=ImageMatrix(height,width);
			f.read((char*)term,height*rowsize);
			f.close();
		}
		
		void save(char* filename)
		{
			INFO h=
			{		
				19778,   //0x4d42
				54+rowsize*height,   
				0,
				0,
				54,
				40,
				width,
				height,
				1,
				24,   //1->24  就成功ㄌ 
				0,
				rowsize*height,
				0,   //3780
				0,   //3780
				0,
				0				
			};
			cout<<"writing into "<<filename<<"..."<<endl;
			ofstream f;
			f.open(filename,ios::out|ios::binary);
			f.write((char*)&h,sizeof(h));
			f.write((char*)term,rowsize*height);
			f.close();	
		}	
};

int color(ImageMatrix m,int x,int y)  // 0 -> white ; 1 -> black 
{
	if(m.term[y*m.rowsize+x]>BYTE(127) && m.term[y*m.rowsize+x+1]>BYTE(127) && m.term[y*m.rowsize+x+2]>BYTE(127)) 
		return 0;
	else           
		return 1;
}
 
void setcolor(ImageMatrix m,int x,int y,int bow)  // 0 -> white ; 1 -> black
{
	if(bow==0)
		m.term[y*m.rowsize+x]=m.term[y*m.rowsize+x+1]=m.term[y*m.rowsize+x+2]=BYTE(255);
	else
		m.term[y*m.rowsize+x]=m.term[y*m.rowsize+x+1]=m.term[y*m.rowsize+x+2]=BYTE(0);
}
 
int flag;
ImageMatrix rand_grid(ImageMatrix m)  // generate a full random grids
{	
	srand((unsigned) time(NULL));   // B G R
	
	for(int y=0;y<m.height;y++)
	{
		for(int x=0;x<m.rowsize;x+=3)
		{
			flag=rand()%2;
			setcolor(m,x,y,flag);		
		}
	}
	return m;	
}

ImageMatrix algo_1(ImageMatrix input,ImageMatrix grid)   //random grid algorithm 1 
{
	ImageMatrix mm=ImageMatrix(grid.height,grid.width);	  
	for(int y=0;y<grid.height;y++)   
	{
		for(int x=0;x<grid.rowsize;x+=3)
		{	
			flag=color(input,x,y)?abs(color(grid,x,y)-1):color(grid,x,y);
			setcolor(mm,x,y,flag);
		}
	}
	return mm;
} 

ImageMatrix algo_2(ImageMatrix input,ImageMatrix grid)   //random grid algorithm 2
{
	ImageMatrix mm=ImageMatrix(grid.height,grid.width);	  
	for(int y=0;y<grid.height;y++)   
	{
		for(int x=0;x<grid.rowsize;x+=3)
		{	
			flag=color(input,x,y)?rand()%2:color(grid,x,y);
			setcolor(mm,x,y,flag);
		}
	}
	return mm;
} 

ImageMatrix algo_3(ImageMatrix input,ImageMatrix grid)   //random grid algorithm 3 
{
	ImageMatrix mm=ImageMatrix(grid.height,grid.width);	  
	for(int y=0;y<grid.height;y++)   
	{
		for(int x=0;x<grid.rowsize;x+=3)
		{	
			flag=color(input,x,y)?abs(color(grid,x,y)-1):rand()%2;
			setcolor(mm,x,y,flag);
		}
	}
	return mm;
} 

ImageMatrix useless(ImageMatrix input,ImageMatrix grid)   //random grid algorithm useless 
{
	ImageMatrix mm=ImageMatrix(grid.height,grid.width);	  
	for(int y=0;y<grid.height;y++)   
	{
		for(int x=0;x<grid.rowsize;x+=3)
		{	
			setcolor(mm,x,y,rand()%2);
		}
	}
	return mm;
} 

ImageMatrix restored(ImageMatrix grid,ImageMatrix grid2)
{
	ImageMatrix original=ImageMatrix(grid.height,grid.width);
	
	for(int y=0;y<grid.height;y++)
		for(int x=0;x<grid.rowsize;x+=3)
			(color(grid,x,y)==0 && color(grid2,x,y)==0)?setcolor(original,x,y,0):setcolor(original,x,y,1);
	
	return original;
}


int main()
{
	ImageMatrix grid2_1,grid2_2,grid2_3,grid2_4,input,output;
	char filename[80];
	cout<<"Input an image: ";
	cin.getline(filename,80);
	input.load(filename);
	int width=input.width;  
	int height=input.height;  
	ImageMatrix grid(height,width);
	
	srand((unsigned)time(NULL));
	
	//generate random grids (grid1)
	rand_grid(grid);
//	grid.save("grid.bmp"); 
	
	//generate second random grids according to grid1	
	grid2_1=algo_1(input,grid);
//	grid2_1.save("algo.1_grid2.bmp");
	
	grid2_2=algo_2(input,grid);
//	grid2_2.save("algo.2_grid2.bmp");
	
	grid2_3=algo_3(input,grid);
//	grid2_3.save("algo.3_grid2.bmp");

	grid2_4=useless(input,grid);
	
	// superimpose two random grid to recover the original pics
	output=restored(grid,grid2_1);
	output.save("algo.1_output.bmp");
	
	output=restored(grid,grid2_2);
	output.save("algo.2_output.bmp");
	
	output=restored(grid,grid2_3);
	output.save("algo.3_output.bmp");
	
	output=restored(grid,grid2_4);
	output.save("algo.useless_output.bmp");
	
		
	system("Pause");
	return 0;
}
