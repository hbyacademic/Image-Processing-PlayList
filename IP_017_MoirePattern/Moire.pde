void setup(){
  size(600,600); width=height=600;
  strokeWeight(3); frameRate(5);
}

void pattern(float d,color c){
  stroke(c); int gap=6,HGap=200,VGap=50;
  int MidX=width/2, MidY=height/2;
  pushMatrix();
  //rotate around their relative position to the center point of the canvas
  translate(MidX, MidY); //move to the center point of the canvas
  rotate(radians(d)); //rotate d degrees
  for(int i=0;i<=(width-2*HGap)/gap;i++)
    line(HGap+gap*i-MidX,VGap-MidY,HGap+gap*i-MidX,height-VGap-MidY); //vertical lines
  popMatrix();
}

int c=0,cnt=1;
void draw(){
  background(192);  
  c=c+cnt;
  if( (c%31)-15==15 || (c%31)-15==-15 ) cnt*=-1; 
  pattern(0,#ff0000); //red lines, rotate 0 degree (fixed)
  pattern((c%31)-15,#00ff00); //green lines, rotate -15~0~15 degrees
  pattern(-((c%31)-15),#0000ff); //blue lines, rotate 15~0~-15 degrees
  //pattern(0,#FF0000);
  //pattern(25,#FF7D00);
  //pattern(50,#FFFF00);
  //pattern(75,#00FF00);
  //pattern(100,#0000FF);
  //pattern(125,#00FFFF); 
  //pattern(150,#FF00FF);
}
