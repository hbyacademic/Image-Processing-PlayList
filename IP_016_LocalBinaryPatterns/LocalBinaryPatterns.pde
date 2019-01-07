float [] oeye=new float[256];
float [] omouse=new float[256];
float [] eye=new float[256];
float [] mouse=new float[256];
PImage img,output,binary;
PFont myFont;

void setup(){
  size(780,600);
  img=loadImage("head.bmp"); 
  img.resize(360,360);
  myFont = createFont("Georgia", 25);
  textFont(myFont);
  textAlign(CENTER,CENTER);
  strokeWeight(2);
}

//If the input image is RGB, then it should transfer to grayscale first.
//////////////////////////////////////////////////
//simple transformation from RGB to grayscale   //
//formula: 0.299*red + 0.587*green + 0.114*blue //
//////////////////////////////////////////////////
PImage rgb2gray(PImage img){
  output=createImage(img.width,img.height,ALPHA);
  for(int i=0;i<img.pixels.length;i++){
    color C=img.pixels[i];
    output.pixels[i]=color(0.299*red(C)+0.587*green(C)+0.114*blue(C));
  }
  return output;
}

//////////////////////////////////////
//  return the color of pixel(x,y)  //
//////////////////////////////////////
color Color(PImage input,int x,int y){
  return input.pixels[y*input.width+x];
}

///////////////////////////////////////
//pixels[] naotation                 //
//x increases from left to right     //
//y increases from top to bottom     //
//LBP: Local Binary Patterns         //
///////////////////////////////////////
PImage localBinaryPatterns(PImage input){
  output=createImage(input.width,input.height,ALPHA);
  input.loadPixels();
 
  for(int y=0;y<output.height;y++){
    for(int x=0;x<output.width;x++){
      int sum=0,flag;
      color threshold=Color(input,x,y);
      
      //not to modify the pixel value x, if it doesn't have eight neighbors 
      if(y==0 || y==output.height-1 || x==0 || x==output.width-1){
        output.pixels[y*output.width+x]=input.pixels[y*output.width+x];
      }
       
      // a b c 
      // h x d   
      // g f e
      
      //a,b,c,d,e,f,g,h should be either 0 or 1
      //if the current pixel value is greater than x, then curret pixel is set to be 1
      //otherwise, set to be 0
      //binary string: abcdefg
      //the resultant binary string is the new pixel value of x   
      else{
        flag=(Color(input,x-1,y-1)>threshold)?1:0;
        sum+=(flag*128);
        flag=(Color(input,x,y-1)>threshold)?1:0;
        sum+=(flag*64);
        flag=(Color(input,x+1,y-1)>threshold)?1:0;
        sum+=(flag*32);
        flag=(Color(input,x+1,y)>threshold)?1:0;
        sum+=(flag*16);
        flag=(Color(input,x+1,y+1)>threshold)?1:0;
        sum+=(flag*8);
        flag=(Color(input,x,y+1)>threshold)?1:0;
        sum+=(flag*4);
        flag=(Color(input,x+1,y-1)>threshold)?1:0;
        sum+=(flag*2);
        flag=(Color(input,x,y-1)>threshold)?1:0;
        sum+=(flag*1);
        output.pixels[y*output.width+x]=color(sum);  
     }
   }      
 }
 return output;
}

/////////////////////////////////////////////////////////////
//make the histogram of selected regions                   //
//input: input image                                       //
//eye: statistical results of specific region (eye)        //
//ex,ey: the top left point of speicifc region (eye)       //
//ew,eh: the width/height of specific region (eye)         //
//erange: upper bound of the value's target range          //
//                                                         //
//mouse: statistical results of specific region (mouse)    //
//mx,my: the top left point of speicifc region (mouse)     //
//mw,mh: the width/height of specific region (mouse)       //
//mrange: upper bound of the value's target range          //
/////////////////////////////////////////////////////////////
void histogram(PImage input,float [] eye,int ex,int ey,int ew,int eh,int erange,float [] mouse,int mx,int my,int mw,int mh,int mrange){
  
  input.loadPixels();
  
  //for 256 grayscale
  int [] tmpEye=new int[256];
  int [] tmpMouse=new int[256];
  
  //calculate the number of each grayscale
  for(int y=ey;y<ey+eh;y++){
    for(int x=ex;x<ex+ew;x++){
      tmpEye[int(red(input.pixels[y*input.width+x]))]++;
      }
  }
  for(int y=my;y<my+mh;y++){
    for(int x=mx;x<mx+mw;x++){
      tmpMouse[int(red(input.pixels[y*input.width+x]))]++;
    }
  }
  
  //re-maps a number from one range to another
  for(int i=0;i<256;i++){
    eye[i]=map(tmpEye[i],0,ew*eh,0,erange);
    mouse[i]=map(tmpMouse[i],0,mw*mh,0,mrange);
  }
}

////////////////////////////////////////////////
//show the histogram of selected region       //
//statistics: statistical result              //
//posx: first x-coordinate of histogram       //
//c: histogram color                           //
////////////////////////////////////////////////
void showHistogram(float [] statistics,int posx,color c){
   for(int i=0;i<256;i++){
     stroke(c);
     rect(posx+i*0.5,590-statistics[i],0.5,statistics[i]);
   }
}

PImage visual(PImage input,int i){
  img.loadPixels();
  input.loadPixels();
  output=createImage(img.width,img.height,RGB);
  for(int ptr=0;ptr<i;ptr++)
    output.pixels[ptr]=color(input.pixels[ptr]);
  for(int ptr=i;ptr<img.pixels.length;ptr++)
    output.pixels[ptr]=color(img.pixels[ptr]);
  return output;
}

PImage show;
int i=0;


void draw(){
  
  //RGBtoBinary
  text("Greyscale Input Image (GII)",200,20);
  text("Histogram of GII",200,460);
  fill(0);
  binary=rgb2gray(img);
  image(binary,20,40);
  histogram(binary,oeye,207,282,72,42,800,omouse,24,174,66,96,800);
  showHistogram(omouse,72,#ff0000);
  showHistogram(oeye,72+128+10,#00ff00);
  //mouse
  noFill();
  stroke(0,255,0);
  rect(227,322,72,42);
  //eye
  stroke(255,0,0);
  rect(44,214,66,96);
  
  //--------------------------------------------------------
  //LBP
  text("Local Binary Patterns (LBP)",580,20);
  text("(feature vector)",580,430);
  text("Histogram of LBP",580,460);
  fill(0);
  output=localBinaryPatterns(binary);
  image(output,400,40);
  histogram(output,eye,207,282,72,42,800,mouse,24,174,66,96,1000);
  showHistogram(mouse,452,#ff0000);
  showHistogram(eye,452+128+10,#00ff00);
  //mouse
  noFill();
  stroke(0,255,0);
  rect(607,322,72,42);
  //eye
  stroke(255,0,0);
  rect(424,214,66,96);
}
