PImage img,bg,bg2,out,result;
int mksw=1; //mask side width
float v=16.0;
float [][] gf = {{1,2,1}, {2,4,2}, {1,2,1}}; //3x3 gaussian filter

void setup(){
  size(480,320);
  img=loadImage("face.bmp");
  bg=loadImage("old.jpg");
  bg2=loadImage("old2.jpg");
  img.resize(480,320);
  bg.resize(480,320);
  bg2.resize(480,320);
}

PImage OldEffect(PImage img, PImage bg, float ratio){
  //1. load input image and background image
  img.loadPixels();
  bg.loadPixels();
  for(int i=0;i<img.pixels.length;i++){
    color c=img.pixels[i];
    color bc=bg.pixels[i];
    //2. convert the input image into sepia image with special formula
    //new_red  = 0.393*old_red + 0.769*old_green + 0.189*old_blue;
    //new_green= 0.349*old_red + 0.686*old_green + 0.168*old_blue;
    //new_blue = 0.272*old_red + 0.534*old_green + 0.131*old_blue;
    
    //3. blend sepia image with background image via alpha blending
    //output = sepia*ratio + background*(1-ratio), where 0<=ratio<=1
    float nr=constrain(0.393*red(c)+0.769*green(c)+0.189*blue(c),0,255)*ratio+red(bc)*(1-ratio);
    float ng=constrain(0.349*red(c)+0.686*green(c)+0.168*blue(c),0,255)*ratio+green(bc)*(1-ratio);
    float nb=constrain(0.272*red(c)+0.534*green(c)+0.131*blue(c),0,255)*ratio+blue(bc)*(1-ratio);
    img.pixels[i]=color(nr,ng,nb);
  }
  //4. update the pixels in input image
  img.updatePixels();
  
  //5. apply guaussian filter to blur image
  //since old image is sometimes under lower resolution
  out=createImage(img.width,img.height,RGB); 
  out=img;
  for(int y=mksw;y<img.height-mksw;y++){
    for(int x=mksw;x<img.width-mksw;x++){
      float r=0.0,g=0.0,b=0.0;
      for(int ky=-mksw;ky<=mksw;ky++){
        for(int kx=-mksw;kx<=mksw;kx++){
          color c=img.pixels[(y+ky)*img.width+(x+kx)];
          //convolve the image with gaussian filter
          r+=red(c)*gf[ky+mksw][kx+mksw];
          g+=green(c)*gf[ky+mksw][kx+mksw];
          b+=blue(c)*gf[ky+mksw][kx+mksw];
        }
      }
      out.pixels[y*img.width+x]=color(r/v,g/v,b/v);
    }
  }
  out.updatePixels();
  return out;
}

void draw(){
  noLoop();
  result=OldEffect(img,bg,0.8);
  //result=OldEffect(img,bg2,0.9);
  result.save("output.jpg");
  image(result,0,0);
}
