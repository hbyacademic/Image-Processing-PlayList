PImage img,output;
void setup(){
  size(720,480);
  img=loadImage("face.bmp");
  img.resize(720,480);
}

PImage oil(PImage input,int radius, int IntensityLevel){
  
  //a new image where the size is the same as input one
  output=createImage(img.width,img.height,RGB);
  for(int y=radius;y<input.height-radius;y++){
    for(int x=radius;x<input.width-radius;x++){
      int [] Intensity=new int [256];
      float [] SumR=new float[256];
      float [] SumG=new float[256];
      float [] SumB=new float[256];
      
      //find the intensities of the neighboring (radius*radius) pixels
      // x x x
      // x c x    =>  x is neighboring pixel, and c is the current pixel
      // x x x
      for(int dy=-radius;dy<=radius;dy++){
        for(int dx=-radius;dx<=radius;dx++){
          
          //R,G,B of the current pixel
          float R=red(input.pixels[(x+dx)+(y+dy)*input.width]);
          float G=green(input.pixels[(x+dx)+(y+dy)*input.width]);
          float B=blue(input.pixels[(x+dx)+(y+dy)*input.width]);
          
          //intensity of the current pixel
          int CurrentIntensity = int((((R+G+B)/3.0)*IntensityLevel)/255.0);
          if(CurrentIntensity>255) CurrentIntensity = 255;
          Intensity[CurrentIntensity]++;
          SumR[CurrentIntensity]+=R;
          SumG[CurrentIntensity]+=G;
          SumB[CurrentIntensity]+=B;
        }
      }
      
      //find the highest freqency intensity
      int MaxIntensityFreq=0;
      int MaxIntensityIndex=0;
      for(int i=0;i<256;i++){
        if(Intensity[i]>MaxIntensityFreq){
          MaxIntensityFreq=Intensity[i];
          MaxIntensityIndex=i;
        }
      }
      
      //assign the new color to current pixel
      float newR=SumR[MaxIntensityIndex]*1.0/MaxIntensityFreq;
      float newG=SumG[MaxIntensityIndex]*1.0/MaxIntensityFreq;
      float newB=SumB[MaxIntensityIndex]*1.0/MaxIntensityFreq;
      output.pixels[x+y*input.width]=color(newR,newG,newB);
    }
  } 
  return output;
}

void draw(){
  output=oil(img,4,8);
  image(output,0,0);
}
