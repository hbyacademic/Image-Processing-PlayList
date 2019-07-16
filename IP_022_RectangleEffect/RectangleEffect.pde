PImage img;
class RECT{
  float x;  //x position
  float y;  //y position
  float r;  //radius
  color c;  //color
  
  RECT(float bx,float by,float br,color bc){
    x=bx;  y=by;  r=br;  c=bc;
  }
 
  //show the rectangle
  void show(){
    fill(c);
    noStroke();
    rectMode(RADIUS);
    rect(x,y,r,r);
  }
}

//put all rectangles into RECTs
ArrayList<RECT> RECTs = new ArrayList<RECT>();

void setup(){
  size(480,320);
  img=loadImage("face.bmp");
  img.resize(480,320);
  
  //initial rectangle
  color c=img.get(width/2,height/2);
  c=color(c,32);
  RECTs.add(new RECT(width/2, height/2, width/2,c)); 
}

void draw(){
  
  if(RECTs.size()<2){
    image(img,0,0);
  } 

  //show all the rentangles in RECTs
  for(int i=0;i<RECTs.size();i++){
    RECT r=RECTs.get(i);
    r.show();
  }
 
  for(int i=0;i<RECTs.size();i++){
    RECT b=RECTs.get(i);
    
    if(dist(mouseX,mouseY,b.x,b.y)<b.r){
      //remove the current mouse-pointed rectangle
      RECTs.remove(i);
      
      //current rectangle was spilt into four sub rectangles
      for(int dx=0;dx<2;dx++){
         for(int dy=0;dy<2;dy++){
           color c=img.get(int(pow(-1,dx)*b.r/2+b.x),int(pow(-1,dy)*b.r/2+b.y));
           RECT nc=new RECT(pow(-1,dx)*b.r/2+b.x,pow(-1,dy)*b.r/2+b.y,b.r/2,c);
           RECTs.add(nc);
         }
      }
    }
  }
}
