PImage img;
float time = 0.0, timestep = 50;
int rIdx = 0, gIdx = 0, bIdx = 0;  //index
float initrIdx, initgIdx, initbIdx;  //init index
float rProb, gProb, bProb;  //probability
float inverter;

void setup(){
  size(480,320);
  img = loadImage("face.bmp");
  //img = loadImage("input2.png");
  img.resize(480,320);
}

PImage glitch(PImage img, float nbsplit, float speed){  
  
  //with different time for noise input
  rProb = exp(-10/speed) * noise(time + timestep * 1);
  gProb = exp(-10/speed) * noise(time + timestep * 2);
  bProb = exp(-10/speed) * noise(time + timestep * 3);   
  
  PImage output = createImage(img.width, img.height, RGB);
  output.loadPixels();
  img.loadPixels();
  
  for(int i=0;i<img.pixels.length;i++){   
    
    inverter = (random(1) <= 0.5)? 1 : -1;
    //move toward upper left direction or lower right direction
    if(rProb > random(1))
      rIdx += (img.width) * inverter;    
    
    if(gProb > random(1)) 
      gIdx += (img.width) * inverter;
    
    if(bProb > random(1)) 
      bIdx += (img.width) * inverter;
    
    rIdx += nbsplit;
    gIdx += nbsplit;
    bIdx += nbsplit;
    
    //out of boundary handling
    if(rIdx < 0) rIdx = 0;
    if(gIdx < 0) gIdx = 0;
    if(bIdx < 0) bIdx = 0;
    
    //for cycling the index
    //not to use "constrain(rIdx, 0, img.pixels.length-1)"
    rIdx %= img.pixels.length;
    gIdx %= img.pixels.length;
    bIdx %= img.pixels.length;
    
    //assign color
    //take the similar concept
    //here, we use only one image instead of three images
    //take red, green, blue components of pixels in different positions 
    //and apply those values to one pixel
    output.pixels[i] = color(red(img.pixels[rIdx]), 
                             green(img.pixels[gIdx]), 
                             blue(img.pixels[bIdx]));
  }
  output.updatePixels();
  img.updatePixels();  
  return output;
}

PImage out;
float randomizedSpeed;
void draw(){
  randomizedSpeed = random(1, 1.1);
  out = glitch(img, 1, randomizedSpeed);
  
  //not to let Idx be far from initial value
  if(abs(initrIdx-rIdx) > img.width*2) rIdx = 0;
  if(abs(initgIdx-gIdx) > img.width*2) gIdx = 0;
  if(abs(initbIdx-bIdx) > img.width*2) bIdx = 0;
  
  image(out, 0, 0); 
  time += 1/30.0;
}
