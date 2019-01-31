PImage input,output;
void setup(){
  size(720,480);
  input=loadImage("face.bmp"); input.resize(720,480);  
  background(192);
}

PImage fishEye(PImage input){
  PImage output=createImage(input.width,input.height,RGB);
  input.loadPixels();
  int w=input.width, h=input.height, midx=width/2, midy=height/2;
  for(int y=0;y<h;y++){
    for(int x=0;x<w;x++){
      int dx=x-midx,dy=y-midy;
      float theta=atan2(dy,dx), radius=sqrt(dx*dx+dy*dy);
      float Nradius=radius*radius/(max(midx,midy));
      float newX=midx+(Nradius*cos(theta)), newY=midy+(Nradius*sin(theta));
      if(newX>0 && newX<w && newY>0 && newY<h)
        output.pixels[y*w+x]=input.pixels[int(newY)*w+int(newX)];
    }
  }
  return output;
}

void draw(){
  output=fishEye(input);
  image(output,0,0);
}
