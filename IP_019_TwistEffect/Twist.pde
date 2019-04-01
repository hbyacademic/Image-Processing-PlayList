PImage input, output;
void setup(){
  size(480,320);
  input=loadImage("face.bmp");
  input.resize(480,320); 
  frameRate(20); background(125);
}

PImage twist(PImage input,float factor){
  PImage output=createImage(input.width,input.height,RGB);
  input.loadPixels();
  int w=input.width, h=input.height;
  for(int y=0;y<h;y++){
    for(int x=0;x<w;x++){
      //apply sine wave on both x and y directions (also can apply cosine wave)
      float dx=(factor*sin(TWO_PI*y/64.0)), dy=(factor*cos(TWO_PI*x/64.0));
      float newX=(x+dx), newY=(y+dy);
      if(newX>=0 && newX<w && newY>=0 && newY<h)
        output.pixels[y*w+x]=input.pixels[int(newY)*w+int(newX)]; }}
  return output;
}

boolean turn=false;
void draw(){
  if(turn) output=twist(input,abs(19-frameCount%20));
  else output=twist(input,frameCount%20);
  if(frameCount%20==19) turn=!turn;
  image(output,0,0);
}
