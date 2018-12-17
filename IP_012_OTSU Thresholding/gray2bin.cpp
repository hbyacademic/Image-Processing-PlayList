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

void intro_of_the_variance()
{
/*
1. total variance VT = VW + VB

2. within class variance VW= pL(t)*sL(t)+pH(t)*sH(t)

	t is the threshold value so that pxiels can be classified into two classes (Low,High)
	
	uL is the mean of class_Low
	uH is the mean of class_High
	 
	pL(t) = (# of pixels in class_Low)/(total pixels)
	the sum of the probaility of the pixel where the grayvalue <= threshold "t"
	the ratio of (# of pixels in class_Low) to (total pixels)
		  
	pH(t) = (# of pixels in class_High)/(total pixels)
	the sum of the probaility of the pixel where the grayvalue > threshold "t"
	the ratio of (# of pixels in class_High) to (total pixels)
	
	sL(t) = [(uL-0)^2 + (uL-1)^2 + ... + (uL-t)]/(# of pixels in class_Low)
	the variacne of class_Low
	
	sH(t) = [(uH-(t+1))^2 + (uH-(t+2))^2 + ... + (uH-255)^2]/(# of pixels in class_High) 
	the variance of class_High	

3. between class variance VB = total variance - within class variance
						 	 = VT - VW
						 	 = pL(t)*[(uL-u)^2] + pH(t)*[(uH-u)^2] ,where u = pL(t)*uL+pH(t)+uH
						 	 = pl(t)*pH(t)*[(uL-uH)^2]
*/
}

void intro_of_between_class_variance()
{
/* 
----OTSUn2 & OTSUn----	//between class variance

#goal: find the maximum of pL(t)*pH(t)*{[uL(t)-uH(t)]^2}, 
			where t is the threshold value
			  pL(t) is the ratio of (# of pxiels in class_Low) to (total pixels)
			  pH(t) is the ratio of (# of pxiels in class_High) to (total pixels)
			  uL(t) is the mean of class_Low
		   	  uH(t) is the mean of class_High
	
	h(t) is the histogram of the pixel where the grayvalue is t
	Nh(t) is the probaility of the pixel where the grayvalue is t
	pL(t) = Nh(0)+Nh(1)+...+Nh(t) = [h(0)+h(1)+...+h(t)]/(total_pixel)
	pH(t) = Nh(t+1)+Nh(t+2)+...+Nh(255) = 1-pL(t) 
	uL(t) = [0*Nh(0)+1*Nh(1)+...+t*Nh(t)]/pL(t)
	uH(t) = [(t+1)*Nh(t+1)+(t+2)*Nh(t+2)+...+255*Nh(255)]/pH(t)
*/
}

void intro_of_within_class_variance()
{
/*
----OTSU2----	//within class vairance

#goal: find the minimum of pL(t)sL(t)+pH(t)sH(t),
			where pL(t) is the ratio of (# of pxiels in class_pH) to (total pixels)
			  	  pH(t) is the ratio of (# of pxiels in class_pL) to (total pixels)
				  sL(t) is the variance of class_pL
				  sH(t) is the variance of class_pH

	h(t) is the histogram of the pixel where the grayvalue is t
	uL(t) = [0*h(0)+1*h(1)+...+t*h(t)]/[h(0)+h(1)+...+h(t)]
	uH(t) = [(t+1)*h(t+1)+(t+2)*h(t+2)+...+255*h(255)]/[h(t+1)+h(t+2)+...+h(255)]
	sL(t) = {[(uL-0)^2]*h(0)+[(uL-1)^2]*h(1)+...+[(uL-t)^2]*h(t)}/pl(t)
	sH(t) = {[(uH-(t+1))^2]*h(t+1)+[(uH-(t+2))^2]*h(t+2)+...+[(uH-255)^2]*h(255)]}/pH(t) 
*/
}

//gray to binary
void gray2bin(Image input,unsigned char threshold)
{
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
		{
			//"white" if > threshold, "black" if <= threshold 
			if(input.term[y*input.rowsize+x]>threshold && input.term[y*input.rowsize+x+1]>threshold && input.term[y*input.rowsize+x+2]>threshold)
				input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]=255;
			else
				input.term[y*input.rowsize+x]=input.term[y*input.rowsize+x+1]=input.term[y*input.rowsize+x+2]=0;
		}
}

//the maximum of Vb = pL*pH*((uL-uH)^2)
//time complexity: O(n^2)
int OTSU_b_n2(Image input)
{
	float max=-1;
	int cnt[256]={0};
	unsigned char threshold=0;
	int total_pixel=input.height*input.width;
	
	//histogram
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
			cnt[input.term[y*input.rowsize+x]]++;

	//compute pL, pH, uL, uH
	for(int t=0;t<256;t++)
	{
		float pL=0,pH=0,uL=0,uH=0;
		for(int i=0;i<256;i++)
		{
			if(i<=t)
			{
				pL+=cnt[i];
				uL+=(cnt[i]*i*1.0/total_pixel);
			}
			else
			{
				pH+=cnt[i];
				uH+=(cnt[i]*i*1.0/total_pixel);
			}
		}
		pL/=total_pixel;
		uL/=pL;
		pH/=total_pixel;
		uH/=pH;
		
		//find the maximum of pL*pH*((uL-uH)^2)
		if(pL*pH*(uL-uH)*(uL-uH)>max)
		{
			max=pL*pH*(uL-uH)*(uL-uH);	
			threshold=t;
		}
	}
	return threshold;
}

//the maximum of Vb = pL*pH*((uL-uH)^2)
//time complexity: O(n)
int OTSU_b_n(Image input)
{
	float max=-1;
	int cnt[256]={0},Tsum=0;
	unsigned char threshold=0;
	
	//histogram
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
			cnt[input.term[y*input.rowsize+x]]++;

	//Tsum = total sum of i*cnt[i]
	for(int i=0;i<256;i++)
		Tsum+=(i*cnt[i]);

	//nL is # of pixels in class_L
	//hH is # of pixels in class_H
	float nL=0,nH=0,Psum=0,pL=0,pH=0,uL=0,uH=0;
	int total_pixel=input.height*input.width;
	for(int t=0;t<256;t++)
	{	
		nL+=cnt[t];
		nH=total_pixel-pL;
		//Psum = partial sum of t*cnt[t]
		Psum+=(t*cnt[t]);
		if(nL==0 || nH==0) continue;
		//Rsum = rest sum of t*cnt[t], which is equal to Tsum-Psum
		int Rsum=Tsum-Psum;

		pL=nL*1.0/total_pixel;
		pH=1-pL;
		
		uL=Psum/pL;
		uH=Rsum/pH;
		//find the maximum of pL*pH*((uL-uH)^2), where uH=Psum/p1n & u2=Rsum/nHn
		if((pL*pH*(uL-uH)*(uL-uH))>max)
		{
			max=(pL*pH*(uL-uH)*(uL-uH));
			threshold=t;
		}
	}
	return threshold;
}

//the minimum of Vw = pL*sL+pH*sH
//time complexity: O(n^2) 
int OTSU_w_n2(Image input)
{
	float min=INT_MAX;
	int cnt[256]={0};
	unsigned char threshold=0;
	
	//histogram
	for(int y=0;y<input.height;y++)
		for(int x=0;x<input.rowsize;x+=3)
			cnt[input.term[y*input.rowsize+x]]++;

	int total_pixel=input.height*input.width;
	for(int t=0;t<256;t++)
	{
		float nL=0,nH=0,uL=0,uH=0,sL=0,sH=0,pL=0,pH=0;
		for(int i=0;i<256;i++)
		{
			if(i<=t)
			{
				nL+=cnt[i];
				uL+=(cnt[i]*i);
			}
			else
			{
				nH+=cnt[i];
				uH+=(cnt[i]*i);
			}	
		}	
		uL/=nL;
		uH/=nH;	
		for(int i=0;i<256;i++)
		{
			if(i<=t)
				sL+=((uL-i)*(uL-i)*cnt[i]);	
			else
				sH+=((uH-i)*(uH-i)*cnt[i]);
		}
		
		sL/=nL;
		sH/=nH;
		pL=nL/total_pixel;
		pH=nH/total_pixel;
		
		if(pL*sL+pH*sH<min)
		{
			min=pL*sL+pH*sH;
			threshold=t;
		}		
	}
	return threshold;
}

int main()
{ 
	system("mode con: lines=20 cols=60");
	Image input,input2;
	input.load("face.bmp");
	input2.load("face.bmp");
	int t;	//threshold value
	
	//find maximum of between_class_variance
	t=OTSU_b_n2(input);
	cout<<"(Vb_O(n^2)) threshold: "<<t<<endl;
	t=OTSU_b_n(input);
	cout<<"(Vb_O(n))   threshold: "<<t<<endl;
	
	//find minimum of within_class_variance
	t=OTSU_w_n2(input);
	cout<<"(Vw_O(n^2)) threshold: "<<t<<endl;
	
	//Otsu thresholding v.s. fixed thresholding
	gray2bin(input,t);
	input.save("Otsu.bmp");
	gray2bin(input2,128);
	input2.save("fixed.bmp");

	return 0;
}

