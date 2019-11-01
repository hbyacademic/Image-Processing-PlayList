PImage img,imgHE;
final int WIDTH = 480;
final int HEIGHT = 320;
final int offset = (WIDTH-256)/2;
final float OneThirdHeight = HEIGHT *1.0 / 3.0;

void setup(){
  size(960,640);
  noLoop();
  //input image
  img=loadImage("face.bmp");
  img.resize(WIDTH, HEIGHT);
}

void draw(){
  image(img,0,0);
  image(HistogramEqualization(img),480,0);
  
  //histogram
  for(int i=0;i<256;i++){
    //red component
    stroke(255,0,0,128);
    line(offset+i      , 2*HEIGHT, offset+i      , 2*HEIGHT-map(rhist[i],0,histMax(rhist),0,100));
    line(offset+i+WIDTH, 2*HEIGHT, offset+i+WIDTH, 2*HEIGHT-map(rnewhist[i],0,histMax(rnewhist),0,100));
    
    //green component
    stroke(0,255,0,128);
    line(offset+i      , 2*HEIGHT-OneThirdHeight, offset+i      , 2*HEIGHT-OneThirdHeight-map(rhist[i],0,histMax(rhist),0,100));
    line(offset+i+WIDTH, 2*HEIGHT-OneThirdHeight, offset+i+WIDTH, 2*HEIGHT-OneThirdHeight-map(rnewhist[i],0,histMax(rnewhist),0,100));
    
    //blue component
    stroke(0,0,255,128);
    line(offset+i      , 2*HEIGHT-212, offset+i      , 2*HEIGHT-212-map(rhist[i],0,histMax(rhist),0,100));
    line(offset+i+WIDTH, 2*HEIGHT-212, offset+i+WIDTH, 2*HEIGHT-212-map(rnewhist[i],0,histMax(rnewhist),0,100));
  }
  
  //cumulative histogram
  stroke(0,0,0,128);
  strokeWeight(2);
  for(int i=0;i<256-1;i++){
    //red component
    line(offset+i      , 2*HEIGHT-map(rcdf[i],0,histMax(rcdf),0,100)      , offset+i+1      , 2*HEIGHT-map(rcdf[i+1],0,histMax(rcdf),0,100));
    line(offset+i+WIDTH, 2*HEIGHT-map(rnewcdf[i],0,histMax(rnewcdf),0,100), offset+i+1+WIDTH, 2*HEIGHT-map(rnewcdf[i+1],0,histMax(rnewcdf),0,100));
    
    //green component
    line(offset+i      , 2*HEIGHT-OneThirdHeight-map(gcdf[i],0,histMax(gcdf),0,100)      , offset+i+1      , 2*HEIGHT-OneThirdHeight-map(gcdf[i+1],0,histMax(gcdf),0,100));
    line(offset+i+WIDTH, 2*HEIGHT-OneThirdHeight-map(gnewcdf[i],0,histMax(gnewcdf),0,100), offset+i+1+WIDTH, 2*HEIGHT-OneThirdHeight-map(gnewcdf[i+1],0,histMax(gnewcdf),0,100));
    
    //blue component
    line(offset+i      , 2*HEIGHT-2*OneThirdHeight-map(bcdf[i],0,histMax(bcdf),0,100)      , offset+i+1      , 2*HEIGHT-2*OneThirdHeight-map(bcdf[i+1],0,histMax(bcdf),0,100));
    line(offset+i+WIDTH, 2*HEIGHT-2*OneThirdHeight-map(bnewcdf[i],0,histMax(bnewcdf),0,100), offset+i+1+WIDTH, 2*HEIGHT-2*OneThirdHeight-map(bnewcdf[i+1],0,histMax(bnewcdf),0,100));
  }
}

//histograms
float [] rhist = new float[256];
float [] ghist = new float[256];
float [] bhist = new float[256];

//cdfs
float [] rcdf = new float[256];
float [] gcdf = new float[256];
float [] bcdf = new float[256];

//histograms after H.E.
float [] rnewhist = new float[256];
float [] gnewhist = new float[256];
float [] bnewhist = new float[256];

//cdfs after H.E.
float [] rnewcdf = new float[256];
float [] gnewcdf = new float[256];
float [] bnewcdf = new float[256];

//find the maximum value from array 
float histMax(float [] input){
  float max=-1;
  for(int i=0;i<256;i++){
    if(input[i]>max){
      max=input[i];
    }
  }
  return max;
}

//perform H.E.
PImage HistogramEqualization(PImage img){
  PImage output=createImage(img.width,img.height,RGB);
  
  //initial histogram
  for(int i=0;i<256;i++){
    rhist[i]=0;
    ghist[i]=0;
    bhist[i]=0;
  }
  
  //count for each color compoenets (red, green, blue)
  for(int i=0;i<img.pixels.length;i++){
    rhist[int(red(color(img.pixels[i])))]++;
    ghist[int(green(color(img.pixels[i])))]++;
    bhist[int(blue(color(img.pixels[i])))]++;
  }
  
  //establish cdfs
  for(int i=0;i<256;i++){
    if(i==0){
      rcdf[i]=rhist[0];
      gcdf[i]=ghist[0];
      bcdf[i]=bhist[0];
    }
    else{
      rcdf[i]+=(rhist[i]+rcdf[i-1]);
      gcdf[i]+=(ghist[i]+gcdf[i-1]);
      bcdf[i]+=(bhist[i]+bcdf[i-1]);
    }   
  }
  
  //find the minimum in cdf (excluding 0)
  float rcdfMin=0, gcdfMin=0, bcdfMin=0;
  for(int i=0;i<256;i++){
    if(rcdf[i]!=0){
      rcdfMin=rcdf[i];
      break;
    }
  }
  
  for(int i=0;i<256;i++){
    if(gcdf[i]!=0){
      gcdfMin=gcdf[i];
      break;
    }
  }

  for(int i=0;i<256;i++){
    if(bcdf[i]!=0){
      bcdfMin=bcdf[i];
      break;
    }
  }
  
  output.loadPixels();
  img.loadPixels();
  for(int i=0;i<img.pixels.length;i++){
    //new_color = (cdf(graylevel)-cdfMin)/((# pixel in input image)-cdfMin)*255
    int r=int((rcdf[int(red(color(img.pixels[i])))]-rcdfMin)*1.0/(img.width*img.height-rcdfMin)*255.0);
    int g=int((gcdf[int(green(color(img.pixels[i])))]-gcdfMin)*1.0/(img.width*img.height-gcdfMin)*255.0);
    int b=int((bcdf[int(blue(color(img.pixels[i])))]-bcdfMin)*1.0/(img.width*img.height-bcdfMin)*255.0);
    color c=(r<<16 | g<<8 | b);
    output.pixels[i]=color(c);
    
    rnewhist[int((rcdf[int(red(color(img.pixels[i])))]-rcdfMin)*1.0/(img.width*img.height-rcdfMin)*255.0)]++;
    gnewhist[int((gcdf[int(green(color(img.pixels[i])))]-gcdfMin)*1.0/(img.width*img.height-gcdfMin)*255.0)]++;
    bnewhist[int((bcdf[int(blue(color(img.pixels[i])))]-bcdfMin)*1.0/(img.width*img.height-bcdfMin)*255.0)]++;   
  }
  output.updatePixels();

  //for visualizing the cdfs after H.E.
  for(int i=0;i<256;i++){
    if(i==0){
      rnewcdf[i]=rnewhist[0];
      gnewcdf[i]=gnewhist[0];
      bnewcdf[i]=bnewhist[0];
    }
    else{
      rnewcdf[i]+=(rnewhist[i]+rnewcdf[i-1]);
      gnewcdf[i]+=(gnewhist[i]+gnewcdf[i-1]);
      bnewcdf[i]+=(bnewhist[i]+bnewcdf[i-1]);
    }
  }
 
  return output;
}
