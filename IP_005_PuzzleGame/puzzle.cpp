#include <iostream>
#include <fstream> 
#include <stdlib.h>
#include <vector>
#include <algorithm> 
using namespace std;

typedef unsigned char   BYTE;   //  1 byte (0~255) 
typedef unsigned short  WORD;   //  2 bytes (0~65536) 
typedef unsigned long   DWORD;  //  4 bytes (0~2^32 -1) 

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

void setcolor(Image m,int x,int y,int bow)  // 0 -> white ; 1 -> black
{
	if(bow==0)
		m.term[y*m.rowsize+x]=m.term[y*m.rowsize+x+1]=m.term[y*m.rowsize+x+2]=BYTE(255);
	else
		m.term[y*m.rowsize+x]=m.term[y*m.rowsize+x+1]=m.term[y*m.rowsize+x+2]=BYTE(0);
}

int row[9]={0,1,2,0,1,2,0,1,2};
int col[9]={2,2,2,1,1,1,0,0,0};

int game_over(Image input,Image org)
{
	int flag=1;
	//check if 2nd~9th subpuzzles all match 
	for(int i=2;i<10;i++)
		for(int y=col[i-1]*input.height/3;y<(col[i-1]+1)*input.height/3;y++)
			for(int x=row[i-1]*input.rowsize/3;x<(row[i-1]+1)*input.rowsize/3;x+=3)
				if(	input.term[y*input.rowsize+x]!=org.term[y*input.rowsize+x]
					&&input.term[y*input.rowsize+x+1]!=org.term[y*input.rowsize+x+1]
					&&input.term[y*input.rowsize+x+2]!=org.term[y*input.rowsize+x+2]
				) { flag=0; break;	}
				
	return (flag)? 1 : 0; 				
}

Image rand(Image input)
{
	Image output(input.height,input.width);
	
	//set the top left corner to be the white subpuzzle
	for(int y=col[0]*input.height/3;y<(col[0]+1)*input.height/3;y++)
		for(int x=row[0]*input.rowsize/3;x<(row[0]+1)*input.rowsize/3;x+=3)
			setcolor(output,x,y,0);
	
	//generate the random number from 2~9
	vector<int>idx;
	for(int i=2;i<10;i++) idx.push_back(i);
	random_shuffle(idx.begin(),idx.end());
	
	//shuffle eight subpuzzles for game start
	int seq=2;
	for(int j=0;j<idx.size();j++)
	{
		for(int yy=col[idx[j]-1]*input.height/3,y=col[seq-1]*input.height/3;yy<(col[idx[j]-1]+1)*input.height/3 && y<(col[seq-1]+1)*input.height/3;yy++,y++)
			for(int xx=row[idx[j]-1]*input.rowsize/3,x=row[seq-1]*input.rowsize/3;xx<(row[idx[j]-1]+1)*input.rowsize/3 && x<(row[seq-1]+1)*input.rowsize/3;xx+=3,x+=3)
			{
				output.term[yy*input.rowsize+xx]=BYTE(input.term[y*input.rowsize+x]);
				output.term[yy*input.rowsize+xx+1]=BYTE(input.term[y*input.rowsize+x+1]);
				output.term[yy*input.rowsize+xx+2]=BYTE(input.term[y*input.rowsize+x+2]);
			}
		seq++;
	}
	idx.clear();
	return output;
}

void swipe(Image input,int num)
{	
	//record the location of the white subpuzzle
	static int org=1;

	//invalid steps
	if(	 (num<1)
	   ||((org==1) && (num!=2 && num!=4))
	   ||((org==2) && (num!=1 && num!=3 && num!=5))
	   ||((org==3) && (num!=2 && num!=6))
	   ||((org==4) && (num!=1 && num!=5 && num!=7))
	   ||((org==5) && (num!=2 && num!=4 && num!=6 && num!=8))
	   ||((org==6) && (num!=3 && num!=5 && num!=9))
	   ||((org==7) && (num!=4 && num!=8))
	   ||((org==8) && (num!=7 && num!=5 && num!=9))
	   ||((org==9) && (num!=6 && num!=8))	   
	)
	cout<<" invaild step!"<<endl;	

	else
	{
		//move the chosen one to location of white subpuzzle
		for(int yy=col[org-1]*input.height/3,y=col[num-1]*input.height/3;yy<(col[org-1]+1)*input.height/3 && y<(col[num-1]+1)*input.height/3;yy++,y++)
			for(int xx=row[org-1]*input.rowsize/3,x=row[num-1]*input.rowsize/3;xx<(row[org-1]+1)*input.rowsize/3 && x<(row[num-1]+1)*input.rowsize/3;xx+=3,x+=3)
			{
				input.term[yy*input.rowsize+xx]=BYTE(input.term[y*input.rowsize+x]);
				input.term[yy*input.rowsize+xx+1]=BYTE(input.term[y*input.rowsize+x+1]);
				input.term[yy*input.rowsize+xx+2]=BYTE(input.term[y*input.rowsize+x+2]);
			}
		
		//set the chosen region to white
		for(int y=col[num-1]*input.height/3;y<(col[num-1]+1)*input.height/3;y++)
			for(int x=row[num-1]*input.rowsize/3;x<(row[num-1]+1)*input.rowsize/3;x+=3)
				setcolor(input,x,y,0);
		
		//update the location of the white subpuzzle
		org=num;
	}
}

int main()
{ 
	system("mode con: lines=80 cols=55");
	Image input,output,org;
	char filename[80];
	int num;
	
	//select an image
	cout<<"Input an image: ";
	cin>>filename;
	input.load(filename);
	
	//copy an original one for comparison
	org=input;
	
	cout<<"Moving Puzzle (Q/q: quit)"<<endl; 
	//remove the left top corner & randomly set the locations of eight subpuzzles
	output=rand(input);
	input=output;
	input.save("current.bmp");
	cout<<"num: ";
	while(cin>>num)
	{
		if(num==81||num==113) break;
		//swap the chosen subpuzzle and the white subpuzzle
		swipe(input,num);
		input.save("current.bmp");
		//check if eight subpuzzles all match the original one
		if(game_over(input,org)) break;
		cout<<"num: ";
	}
	
	return 0;
}

