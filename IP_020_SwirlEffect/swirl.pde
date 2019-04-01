PImage input, output;
void setup(){
  size(480,320);
  input=loadImage("face.bmp");
  input.resize(480,320); 
  background(192); frameRate(10);
}

PImage Swirl(PImage input,float factor){
  PImage output=createImage(input.width,input.height,RGB);
  input.loadPixels();
  int w=input.width, h=input.height, midx=width/2, midy=height/2;
  for(int y=0;y<h;y++){
    for(int x=0;x<w;x++){
      int dx=x-midx,dy=y-midy;
      //increasing "radius" also increases "the degree of the roatation"
      float theta=atan2(dy,dx), radius=sqrt(dx*dx+dy*dy);    
      float newX=midx+(radius*cos(theta+radians(factor*radius)));    
      float newY=midy+(radius*sin(theta+radians(factor*radius)));
      if(newX>=0 && newX<w && newY>=0 && newY<h)
        output.pixels[y*w+x]=input.pixels[int(newY)*w+int(newX)]; }}
  return output;
}

boolean turn=false; static float cnt=0;
void draw(){
  if(turn) output=Swirl(input,cnt-=0.1);
  else output=Swirl(input,cnt+=0.1);
  if(cnt>=1.5 || cnt<=0) turn=!turn;
  image(output,0,0);
}
