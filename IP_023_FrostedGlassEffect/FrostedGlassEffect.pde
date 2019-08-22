PImage img;
void setup(){
  size(480,320);
  img=loadImage("face.bmp");
  img.resize(480,320);
  frameRate(1000);
}

void draw(){
  for(int i=0;i<50;i++){
    //randomly select a pixel
    int x=int(random(0,width)), y=int(random(0,height));
    //set the corresponding color (with 31% transparent)
    //(80/255)=31%
    color c=color(img.get(x,y),80);
    stroke(c);
    //draw a line segment started from the selected pixel
    line(x,y,x+random(width*-0.03,width*0.03),y+random(width*-0.03,width*0.03));
  }
}
