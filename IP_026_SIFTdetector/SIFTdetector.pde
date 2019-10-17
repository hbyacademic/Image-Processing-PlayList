////////////////////////////////////////////////////////////////////////////////////////////
// Gaussian space     DoG space      DoG check space                                      //
//    5 ----                                                                              //
//          |--->        4                                                                //
//    4 ----                                                                              //
//          |--->        3                 2                                              //
//    3 ----                                                                              //                     
//          |--->        2 -----           1                                              //
//    2 ----                    | \                                                       //
//          |--->        1 -----   ==>     0 (three DoG layers => one DoG check layer)    //
//    1 ----                    | /                                                       //
//          |--->        0 -----                                                          //
//    0 ----                                                                              //
////////////////////////////////////////////////////////////////////////////////////////////

class PotentialExtrema{
  boolean IsExtrema=false;
  int octaveIdx;
  int dogIdx;
  
  PotentialExtrema(int _octaveIdx, int _dogIdx){
    octaveIdx=_octaveIdx;
    dogIdx=_dogIdx;
  }
}
PotentialExtrema [][] p;

int mksize=7;  //Gaussian mask size
int mkside=(mksize-1)/2;  //mask side
PImage img, gray, grayCopy;  //input image, input grayscale image, a copy of input grayscale image

void setup(){
  noLoop();
  size(600,600);
  ellipseMode(CENTER);
  fill(255,0,0);  //red
  stroke(255,0,0);  //red
  strokeWeight(2);
  
  //load input image
  img=loadImage("face.bmp");
  img.resize(img.width/2,img.height/2);
  
  //initialization
  p = new PotentialExtrema[img.width][img.height];
  for(int j=0;j<img.height;j++){
    for(int i=0;i<img.width;i++){
      p[i][j]= new PotentialExtrema(0,0);
    }
  }

  //RGB to grayscale
  gray=createImage(img.width,img.height,RGB);
  for(int j=0;j<img.height;j++){
    for(int i=0;i<img.width;i++){
      color c=img.get(i,j);
      float C=red(c)*0.3+green(c)*0.59+blue(c)*0.11;
      gray.pixels[j*img.width+i]=color(C,C,C);
    }
  }
  gray.updatePixels();
  
  //a copy for grayscale image
  grayCopy=createImage(img.width,img.height,RGB);
  for(int j=0;j<img.height;j++){
    for(int i=0;i<img.width;i++){
      color c=gray.get(i,j);
      grayCopy.pixels[j*img.width+i]=color(red(c),green(c),blue(c));
    }
  }
  grayCopy.updatePixels();
}

/////////////////////////////////////////////////////////////////////////////////////
////////// create Gaussian blur image with parameter ((k^power)*segma)   ////////////
/////////////////////////////////////////////////////////////////////////////////////
PImage createGauImg(PImage gray, float k, float power, float octaveIdx, float sigma){
  
  //prepare output Gaussian blur image
  PImage output=createImage(gray.width,gray.height,RGB);
  
  //prepare 2D Gaussian mask G
  float [][] Gau = new float[mksize][mksize];
  float K = pow(k,power) * octaveIdx;
  for(int y=-mkside;y<=mkside;y++){
    for(int x=-mkside;x<=mkside;x++){
      Gau[y+mkside][x+mkside]=
      (1.0 /(2 * PI * pow(K*sigma,2))) 
      * exp(-(pow(x,2)+pow(y,2)) * 1.0 / (2*pow(K*sigma,2)));
    }
  }
  
  //convolve input grayscale image with G
  for(int j=mkside;j<gray.height-mkside;j++){
    for(int i=mkside;i<gray.width-mkside;i++){
      float r=0.0,g=0.0,b=0.0;
      for(int dy=-mkside;dy<=mkside;dy++){
        for(int dx=-mkside;dx<=mkside;dx++){
          r+=red(gray.get(i-dx,j-dy)) * Gau[dy+mkside][dx+mkside];
          g+=green(gray.get(i-dx,j-dy)) * Gau[dy+mkside][dx+mkside];
          b+=blue(gray.get(i-dx,j-dy)) * Gau[dy+mkside][dx+mkside];
        }
      }
      r=constrain(r,0,255);
      g=constrain(g,0,255);
      b=constrain(b,0,255);
      output.pixels[j*gray.width+i]=color(r,g,b);
    }
  }
  output.updatePixels();
  return output;
}

/////////////////////////////////////////////////////////////////////////////////////
////////// create one octave consisting of "s+3" Gaussian blur images ///////////////
//////////         s is the number of layer in DoG check space        ///////////////
/////////////////////////////////////////////////////////////////////////////////////
int s=3;
PImage [] createOctave(PImage gray, float octaveIdx){
  
  //prepare "s+3" output images
  int nblayer=s+3;
  PImage [] output= new PImage [nblayer];
  
  //initialization for each output image
  for(int i=0;i<output.length;i++)
    output[i]=createImage(gray.width,gray.height,RGB);
  
  //create Gaussian blur image
  ////////////////////////////////////////////////////////////////////////////////////////////
  // Gaussian space (first octave 0th)         Gaussian space (second octave 1th)  ...      //                              
  //    5    (sigma= (2^0)*[k^(5/s)]*sigma)       5    (SIGMA= (2^1)*[k^(5/s)]*sigma)       // 
  //                                                                                        //
  //    4    (SIGMA= (2^0)*[k^(4/s)]*sigma)       4    (SIGMA= (2^1)*[k^(4/s)]*sigma)       //
  //                                                                                        //                                                                                                
  //    3    (SIGMA= (2^0)*[k^(3/s)]*sigma)       3    (SIGMA= (2^1)*[k^(3/s)]*sigma)       //
  //                                                                                        //
  //    2    (SIGMA= (2^0)*[k^(2/s)]*sigma)       2    (SIGMA= (2^1)*[k^(2/s)]*sigma)       //
  //                                                                                        //
  //    1    (SIGMA= (2^0)*[k^(1/s)]*sigma)       1    (SIGMA= (2^1)*[k^(1/s)]*sigma)       //
  //                                                                                        //
  //    0    (SIGMA= (2^0)*sigma)                 0    (SIGMA= (2^1)*sigma)                 //
  ////////////////////////////////////////////////////////////////////////////////////////////
  //octaveIdx, for example, in first octave, octaveIdx=(2^0)
  for(int i=0;i<nblayer;i++){
    output[i]=createGauImg(gray, 2.0, i*1.0/(s*1.0), octaveIdx, 1.6);
  }
  return output;
}

/////////////////////////////////////////////////////////////////////////////////////
////////// create DoG space consisting of "s+2" Gaussian blur images  ///////////////
//////////         s is the number of layer in DoG check space        ///////////////
/////////////////////////////////////////////////////////////////////////////////////
PImage [] createDoGSpace(PImage [] GauImgs){
  
  //prepare "s+2" output images
  int nbdog=s+2;
  PImage [] output= new PImage [nbdog];
  
  //initialization for each output image
  for(int i=0;i<output.length;i++)
    output[i]=createImage(GauImgs[0].width,GauImgs[0].height,RGB);
  
  //create difference of Gaussian (DoG) images
  for(int idx=0;idx<nbdog;idx++){
    for(int j=0;j<GauImgs[0].height;j++){
      for(int i=0;i<GauImgs[0].width;i++){
        color curr=GauImgs[idx].get(i,j);
        color next=GauImgs[idx+1].get(i,j);
        output[idx].pixels[j*GauImgs[0].width+i]=
        color(red(curr)-red(next),green(curr)-green(next),blue(curr)-blue(next));
      }
    }
  }
  return output;
}

/////////////////////////////////////////////////////////////////////////////////////
////////// find potential extrema by comparing 26 neighbors in DoG space  ///////////
/////////////////////////////////////////////////////////////////////////////////////
void findPotentialExtrema(PImage [] Dog, PotentialExtrema [][] p, int scale){  
  
  //find from 1st DoG image to sth DoG image
  for(int idx=1;idx<=s;idx++){
    for(int j=mkside;j<Dog[0].height-mkside;j++){
      for(int i=mkside;i<Dog[0].width-mkside;i++){     
        //compare with 26 neighbors 
        //if it is maximal
        if(largerValue(Dog[idx],i,j,Dog[idx-1],i-1,j-1) && largerValue(Dog[idx],i,j,Dog[idx-1],i  ,j-1) && largerValue(Dog[idx],i,j,Dog[idx-1],i+1,j-1) &&
           largerValue(Dog[idx],i,j,Dog[idx-1],i-1,j  ) && largerValue(Dog[idx],i,j,Dog[idx-1],i  ,j  ) && largerValue(Dog[idx],i,j,Dog[idx-1],i+1,j  ) &&
           largerValue(Dog[idx],i,j,Dog[idx-1],i-1,j+1) && largerValue(Dog[idx],i,j,Dog[idx-1],i  ,j+1) && largerValue(Dog[idx],i,j,Dog[idx-1],i+1,j+1) &&
           
           largerValue(Dog[idx],i,j,Dog[idx+1],i-1,j-1) && largerValue(Dog[idx],i,j,Dog[idx+1],i  ,j-1) && largerValue(Dog[idx],i,j,Dog[idx+1],i+1,j-1) &&
           largerValue(Dog[idx],i,j,Dog[idx+1],i-1,j  ) && largerValue(Dog[idx],i,j,Dog[idx+1],i  ,j  ) && largerValue(Dog[idx],i,j,Dog[idx+1],i+1,j  ) &&
           largerValue(Dog[idx],i,j,Dog[idx+1],i-1,j+1) && largerValue(Dog[idx],i,j,Dog[idx+1],i  ,j+1) && largerValue(Dog[idx],i,j,Dog[idx+1],i+1,j+1) &&
           
           largerValue(Dog[idx],i,j,Dog[idx],i-1,j-1) && largerValue(Dog[idx],i,j,Dog[idx],i  ,j-1) && largerValue(Dog[idx],i,j,Dog[idx],i+1,j-1) &&
           largerValue(Dog[idx],i,j,Dog[idx],i-1,j  ) &&                                               largerValue(Dog[idx],i,j,Dog[idx],i+1,j  ) &&
           largerValue(Dog[idx],i,j,Dog[idx],i-1,j+1) && largerValue(Dog[idx],i,j,Dog[idx],i  ,j+1) && largerValue(Dog[idx],i,j,Dog[idx],i+1,j+1)
        ){
          p[i*scale][j*scale].octaveIdx=scale;  //assign octave index
          p[i*scale][j*scale].IsExtrema=true;  //assign extrema flag
          p[i*scale][j*scale].dogIdx=idx;  //assign DoG space index
        }
        
        //compare with 26 neighbors
        //if it is minimal
        if(smallerValue(Dog[idx],i,j,Dog[idx-1],i-1,j-1) && smallerValue(Dog[idx],i,j,Dog[idx-1],i  ,j-1) && smallerValue(Dog[idx],i,j,Dog[idx-1],i+1,j-1) &&
           smallerValue(Dog[idx],i,j,Dog[idx-1],i-1,j  ) && smallerValue(Dog[idx],i,j,Dog[idx-1],i  ,j  ) && smallerValue(Dog[idx],i,j,Dog[idx-1],i+1,j  ) &&
           smallerValue(Dog[idx],i,j,Dog[idx-1],i-1,j+1) && smallerValue(Dog[idx],i,j,Dog[idx-1],i  ,j+1) && smallerValue(Dog[idx],i,j,Dog[idx-1],i+1,j+1) &&
           
           smallerValue(Dog[idx],i,j,Dog[idx+1],i-1,j-1) && smallerValue(Dog[idx],i,j,Dog[idx+1],i  ,j-1) && smallerValue(Dog[idx],i,j,Dog[idx+1],i+1,j-1) &&
           smallerValue(Dog[idx],i,j,Dog[idx+1],i-1,j  ) && smallerValue(Dog[idx],i,j,Dog[idx+1],i  ,j  ) && smallerValue(Dog[idx],i,j,Dog[idx+1],i+1,j  ) &&
           smallerValue(Dog[idx],i,j,Dog[idx+1],i-1,j+1) && smallerValue(Dog[idx],i,j,Dog[idx+1],i  ,j+1) && smallerValue(Dog[idx],i,j,Dog[idx+1],i+1,j+1) &&
           
           smallerValue(Dog[idx],i,j,Dog[idx],i-1,j-1) && smallerValue(Dog[idx],i,j,Dog[idx],i  ,j-1) && smallerValue(Dog[idx],i,j,Dog[idx],i+1,j-1) &&
           smallerValue(Dog[idx],i,j,Dog[idx],i-1,j  ) &&                                                smallerValue(Dog[idx],i,j,Dog[idx],i+1,j  ) &&
           smallerValue(Dog[idx],i,j,Dog[idx],i-1,j+1) && smallerValue(Dog[idx],i,j,Dog[idx],i  ,j+1) && smallerValue(Dog[idx],i,j,Dog[idx],i+1,j+1)
        ){
          p[i*scale][j*scale].octaveIdx=scale;  //assign octave index
          p[i*scale][j*scale].IsExtrema=true;  //assign extrema flag
          p[i*scale][j*scale].dogIdx=idx;  //assign DoG space index
        }    
      }// end of for idx
    }//end of for i
  }// end of for j
} 

boolean largerValue(PImage current, int ci, int cj, PImage reference, int ri, int rj){
  return red(current.get(ci,cj)) > red(reference.get(ri,rj));
}

boolean smallerValue(PImage current, int ci, int cj, PImage reference, int ri, int rj){
  return red(current.get(ci,cj)) < red(reference.get(ri,rj));
}

/////////////////////////////////////////////////////////////////////////////////////
//////////           remove low contrast potential extrema                ///////////
/////////////////////////////////////////////////////////////////////////////////////
void Taylor(PotentialExtrema [][] p,PImage img,int x,int y){
  
  int prevW=img.width, prevH=img.height;
  int currW=int(prevW/p[x][y].octaveIdx), currH=int(prevH/p[x][y].octaveIdx);
  
  //prepare a copy of input grayscale image
  PImage imgCopy;
  imgCopy=createImage(img.width,img.height,RGB);
  for(int j=0;j<img.height;j++){
    for(int i=0;i<img.width;i++){
      color c=img.get(i,j);
      imgCopy.pixels[j*img.width+i]=color(red(c),green(c),blue(c));
    }
  }
  imgCopy.updatePixels();
  imgCopy.resize(currW,currH);

  //remap the (x,y) in "imgCopy" coordinate system
  int currX=int(x/p[x][y].octaveIdx), currY=int(y/p[x][y].octaveIdx);
  
  //calculate 2-nd partial derivative Dxx, Dxy, Dxs, Dyy, Dys, Dss
  //Dxx Dyx Dsx
  //Dxy Dyy Dsy  => H
  //Dxs Dys Dss

  float k=pow(2,1.0/(s*1.0)*p[x][y].dogIdx)*p[x][y].octaveIdx;  
  float sigma=1.6;
  sigma=sigma*k;
  float Gconst=1.0/(2*PI*pow(sigma,2)); 
  float sigmaNextLayer=sigma*pow(2,1.0/(s*1.0));
  float GconstNextLayer=1.0/(2*PI*pow(sigmaNextLayer,2));
  float dxx, dyy, dxy, dxs, dys, dss;
  float dxxNextLayer, dyyNextLayer, dxyNextLayer, dxsNextLayer, dysNextLayer, dssNextLayer;
  float dx, dy, ds;
  float dxNextLayer, dyNextLayer, dsNextLayer;
  
  float Dxx=0.0, Dyy=0.0, Dxy=0.0, Dxs=0.0, Dys=0.0, Dss=0.0;
  float Dx=0.0, Dy=0.0, Ds=0.0;
  float e, G, eNextLayer, GNextLayer;
  
  for(int my=-mkside;my<=mkside;my++){
    for(int mx=-mkside;mx<=mkside;mx++){
      
      //2-nd partial derivative with respect to xx, yy, xy, xs, ys, ss
      e=exp(-(pow(mx,2)+pow(my,2))/(2*pow(sigma,2)));
      G=Gconst*e;
      dxx=G * (pow(mx,2)-pow(sigma,2)) * 1.0 / pow(sigma,4);
      dyy=G * (pow(my,2)-pow(sigma,2)) * 1.0 / pow(sigma,4);
      dxy=G * ((mx)*(my)) * 1.0 / pow(sigma,4);
      dxs=G * (4*(mx)+pow(mx,2)+pow(my,2)) * 1.0 / pow(sigma,3);
      dys=G * (4*(my)+pow(mx,2)+pow(my,2)) * 1.0 / pow(sigma,3);
      dss=G * (2.0/pow(sigma,2) + (pow(mx,2)+pow(my,2))/pow(sigma,4) + pow((-2*pow(sigma,2)+pow(mx,2)+pow(my,2))*1.0/pow(sigma,3),2) );
      
      //"NextLayer" means one layer above the current layer in the same octave
      eNextLayer=exp(-(pow(mx,2)+pow(my,2))/(2*pow(sigmaNextLayer,2)));
      GNextLayer=GconstNextLayer*eNextLayer;
      dxxNextLayer=GNextLayer * (pow(mx,2)-pow(sigmaNextLayer,2)) * 1.0 / pow(sigmaNextLayer,4);
      dyyNextLayer=GNextLayer * (pow(my,2)-pow(sigmaNextLayer,2)) * 1.0 / pow(sigmaNextLayer,4);
      dxyNextLayer=GNextLayer * ((mx)*(my)) * 1.0 / pow(sigmaNextLayer,4);
      dxsNextLayer=GNextLayer * (4*(mx)+pow(mx,2)+pow(my,2)) * 1.0 / pow(sigmaNextLayer,3);
      dysNextLayer=GNextLayer * (4*(my)+pow(mx,2)+pow(my,2)) * 1.0 / pow(sigmaNextLayer,3);
      dssNextLayer=GNextLayer * (2.0/pow(sigmaNextLayer,2) + (pow(mx,2)+pow(my,2))/pow(sigmaNextLayer,4) + pow((-2*pow(sigmaNextLayer,2)+pow(mx,2)+pow(my,2))*1.0/pow(sigmaNextLayer,3),2) );
  
      //1-st partial derivative with respect to x, y, s
      dx=G * (-(mx)*1.0/pow(sigma,2));
      dy=G * (-(my)*1.0/pow(sigma,2));
      ds=G * ((-2*pow(sigma,2)+pow(mx,2)+pow(my,2))*1.0/pow(sigma,3));
    
      dxNextLayer=GNextLayer * (-(mx)*1.0/pow(sigmaNextLayer,2));
      dyNextLayer=GNextLayer * (-(my)*1.0/pow(sigmaNextLayer,2));
      dsNextLayer=GNextLayer * ((-2*pow(sigmaNextLayer,2)+pow(mx,2)+pow(my,2))*1.0/pow(sigmaNextLayer,3));
    
      //convolve pixel (currX,currY) with 2-nd derivative DoG filter 
      Dxx+=(dxx-dxxNextLayer)*red(imgCopy.get(currX-mx,currY-my));
      Dyy+=(dyy-dyyNextLayer)*red(imgCopy.get(currX-mx,currY-my));
      Dxy+=(dxy-dxyNextLayer)*red(imgCopy.get(currX-mx,currY-my));
      Dxs+=(dxs-dxsNextLayer)*red(imgCopy.get(currX-mx,currY-my));
      Dys+=(dys-dysNextLayer)*red(imgCopy.get(currX-mx,currY-my));
      Dss+=(dss-dssNextLayer)*red(imgCopy.get(currX-mx,currY-my));
      
      //convolve pixel (currX,currY) with 1-st derivative DoG filter 
      Dx+=(dx-dxNextLayer)*red(imgCopy.get(currX-mx,currY-my));
      Dy+=(dy-dyNextLayer)*red(imgCopy.get(currX-mx,currY-my));
      Ds+=(ds-dsNextLayer)*red(imgCopy.get(currX-mx,currY-my));
    }
  }
  
  //fit into 3*3 matrix
  float [][] H = new float [3][3];
  H[0][0]=Dxx;  H[0][1]=Dxy;  H[0][2]=Dxs;
  H[1][0]=Dxy;  H[1][1]=Dyy;  H[1][2]=Dys;
  H[2][0]=Dxs;  H[2][1]=Dys;  H[2][2]=Dss;
  
  //inverse of H
  float [][] invH = new float [3][3];
  invH=inverseMatrix(H);
    
  //calculate (xhead, yhead, shead) 
  // (x, y, s) = X
  //partial derivative with repsect to three parameters as above
  //(xhead, yhead, shead) =  inverse([d^2H/dX^2]) * [dH/dX]
  float xhead=-(invH[0][0]*Dx+invH[0][1]*Dy+invH[0][2]*Ds);
  float yhead=-(invH[1][0]*Dx+invH[1][1]*Dy+invH[1][2]*Ds);
  float shead=-(invH[2][0]*Dx+invH[2][1]*Dy+invH[2][2]*Ds);
  
  //put (xhead, yhead, shead) back to H
  //and check if the point is needed to be eliminated
  float Newe=exp(-(pow(xhead,2)+pow(yhead,2))/(2*pow(shead,2)));
  float NewGconst=1.0/(2*PI*pow(shead,2)); 
  float NewG=NewGconst*Newe;
  
  float Newdx=NewG * (-(xhead)*1.0/pow(shead,2));
  float Newdy=NewG * (-(yhead)*1.0/pow(shead,2));
  float Newds=NewG * ((-2*pow(shead,2)+pow(xhead,2)+pow(yhead,2))*1.0/pow(shead,3));
  
  //H(xhead, yhead, shead) = H + 0.5 * {transport([dH/dX])*(xhead, yhead, shead)} = Dhead + Dsecond
  float Dhead=exp(-(pow(xhead,2)+pow(yhead,2))/(2*pow(shead,2)))/(2*PI*pow(shead,2))*red(imgCopy.get(currX,currY));
  float Dsecond=(Newdx*xhead+Newdy*yhead+Newds*shead)*1.0/2.0;
  float Dextremal=Dhead+Dsecond;
  
  //reject low contrast points
  //should be "abs(Dextremal)<0.03"
  if(abs(Dextremal)!=0){
    fill(0,0,255); 
    stroke(0,0,255);
    ellipse(x,y,1,1);  //label in blue point
  }
}

/////////////////////////////////////////////////////////////////////////////////////
//////////           find the inverse matrix of input matrix H            ///////////
/////////////////////////////////////////////////////////////////////////////////////
float [][] inverseMatrix(float [][] H){
  //H00 H01 H02
  //H10 H11 H12
  //H20 H21 H22
  float [][] output = new float [3][3];  
  float det=H[0][0]*(H[1][1]*H[2][2]-H[1][2]*H[2][1])-H[0][1]*(H[1][0]*H[2][2]-H[1][2]*H[2][0])+H[0][2]*(H[1][0]*H[2][1]-H[1][1]*H[2][0]); 
  output[0][0]=(H[1][1]*H[2][2]-H[1][2]*H[2][1])*1.0/det;
  output[0][1]=-(H[0][1]*H[2][2]-H[0][2]*H[2][1])*1.0/det;
  output[0][2]=(H[0][1]*H[1][2]-H[0][2]*H[1][1])*1.0/det;
  output[1][0]=-(H[1][0]*H[2][2]-H[1][2]*H[2][0])*1.0/det;
  output[1][1]=(H[0][0]*H[2][2]-H[0][2]*H[2][0])*1.0/det;
  output[1][2]=-(H[0][0]*H[1][2]-H[0][2]*H[1][0])*1.0/det;
  output[2][0]=(H[1][0]*H[2][1]-H[1][1]*H[2][0])*1.0/det;
  output[2][1]=-(H[0][0]*H[2][1]-H[0][1]*H[2][0])*1.0/det;
  output[2][2]=(H[0][0]*H[1][1]-H[0][1]*H[1][0])*1.0/det;
  
  return output;
}

/////////////////////////////////////////////////////////////////////////////////////
//////////           remove low contrast potential extrema                ///////////
/////////////////////////////////////////////////////////////////////////////////////
void Hessian(PotentialExtrema [][] p, PImage img, int x, int y){
  
  int prevW=img.width, prevH=img.height;
  int currW=int(prevW/p[x][y].octaveIdx), currH=int(prevH/p[x][y].octaveIdx);
  
  //prepare a copy of input grayscale image
  PImage imgCopy;
  imgCopy=createImage(img.width,img.height,RGB);
  for(int j=0;j<img.height;j++){
    for(int i=0;i<img.width;i++){
      color c=img.get(i,j);
      imgCopy.pixels[j*img.width+i]=color(red(c),green(c),blue(c));
    }
  }
  imgCopy.updatePixels();
  imgCopy.resize(currW,currH);
  
  //remap the (x,y) in "imgCopy" coordinate system
  int currX=int(x/p[x][y].octaveIdx), currY=int(y/p[x][y].octaveIdx);
  
  //calculate 2-nd partial derivative Dxx, Dxy, Dyy
  float k=pow(2,1.0/(s*1.0)*p[x][y].dogIdx)*p[x][y].octaveIdx;  
  float sigma=1.6;
  sigma=sigma*k;
  float Gconst=1.0/(2*PI*pow(sigma,2)); 
  float sigmaNextLayer=sigma*pow(2,1.0/(s*1.0));
  float GconstNextLayer=1.0/(2*PI*pow(sigmaNextLayer,2));
  float dxx,dyy,dxy;
  float dxxNextLayer,dyyNextLayer,dxyNextLayer;
  float Dxx=0.0,Dyy=0.0,Dxy=0.0;
  
  for(int dy=-mkside;dy<=mkside;dy++){
    for(int dx=-mkside;dx<=mkside;dx++){
      
      //2-nd partial derivative with respect to xx, yy, xy
      dxx=Gconst * (pow(dx,2)-pow(sigma,2)) * exp(-(pow(dx,2)+pow(dy,2))/(2*pow(sigma,2))) * 1.0 / pow(sigma,4);
      dyy=Gconst * (pow(dy,2)-pow(sigma,2)) * exp(-(pow(dx,2)+pow(dy,2))/(2*pow(sigma,2))) * 1.0 / pow(sigma,4);
      dxy=Gconst * ((dx)*(dy)) * exp(-(pow(dx,2)+pow(dy,2))/(2*pow(sigma,2))) * 1.0 / pow(sigma,4);
      
      //"NextLayer" means one layer above the current layer in the same octave
      dxxNextLayer=GconstNextLayer * (pow(dx,2)-pow(sigmaNextLayer,2)) * exp(-(pow(dx,2)+pow(dy,2))/(2*pow(sigmaNextLayer,2))) * 1.0 / pow(sigmaNextLayer,4);
      dyyNextLayer=GconstNextLayer * (pow(dy,2)-pow(sigmaNextLayer,2)) * exp(-(pow(dx,2)+pow(dy,2))/(2*pow(sigmaNextLayer,2))) * 1.0 / pow(sigmaNextLayer,4);
      dxyNextLayer=GconstNextLayer * ((dx)*(dy)) * exp(-(pow(dx,2)+pow(dy,2))/(2*pow(sigmaNextLayer,2))) * 1.0 / pow(sigmaNextLayer,4);
      
      Dxx+=((dxx-dxxNextLayer)*red(imgCopy.get(currX-dx,currY-dy)));
      Dyy+=((dyy-dyyNextLayer)*red(imgCopy.get(currX-dx,currY-dy)));
      Dxy+=((dxy-dxyNextLayer)*red(imgCopy.get(currX-dx,currY-dy)));
   }
  }
  
  float r=10;
  float Det=Dxx*Dyy-Dxy*Dxy;
  float Tr=Dxx+Dxy;

  //nonsense!!!!
  Det*=10;
  Tr*=10;
  
  //reject some edge points
  if((pow(Tr,2)*1.0/Det) >= (pow(r+1,2)*1.0/r)){
    fill(0,255,0);
    stroke(0,255,0);
    ellipse(x,y,1,1);  //label in green point
  }
}

PImage [] output=new PImage [s+3];
PImage [] Dog= new PImage [s+2];

PImage [] output2=new PImage [s+3];
PImage [] Dog2= new PImage [s+2];

PImage [] output3=new PImage [s+3];
PImage [] Dog3= new PImage [s+2];

PImage [] output4=new PImage [s+3];
PImage [] Dog4= new PImage [s+2];

PImage [] output5=new PImage [s+3];
PImage [] Dog5= new PImage [s+2];

void draw(){
  pushMatrix();
  translate((width-img.width)/2,100);
  image(img,0,0);
  
  //first octave
  output=createOctave(gray,1);
  Dog=createDoGSpace(output);
  findPotentialExtrema(Dog,p,1);
  
  //second octave
  gray.resize(img.width/2,img.height/2);
  output2=createOctave(gray,2);
  Dog2=createDoGSpace(output2);
  findPotentialExtrema(Dog2,p,2);
  
  //third octave
  gray.resize(img.width/4,img.height/4);
  output3=createOctave(gray,4);
  Dog3=createDoGSpace(output3);
  findPotentialExtrema(Dog3,p,4);

 //fourth octave
  gray.resize(img.width/8,img.height/8);
  output4=createOctave(gray,8);
  Dog4=createDoGSpace(output4);
  findPotentialExtrema(Dog4,p,8);

  //fifth octave
  gray.resize(img.width/16,img.height/16);
  output5=createOctave(gray,16);
  Dog5=createDoGSpace(output5);
  findPotentialExtrema(Dog5,p,16);

  //low contrast points in green
  //edge points in blue
  //extrema points in red
  for(int j=mkside;j<img.height-mkside;j++){
    for(int i=mkside;i<img.width-mkside;i++){
      if(p[i][j].IsExtrema){
        ellipse(i,j,1,1);
      }
    }
  }
  
  for(int j=mkside;j<img.height-mkside;j++){
    for(int i=mkside;i<img.width-mkside;i++){ 
      if(p[i][j].IsExtrema){
        Taylor(p,grayCopy,i,j);
      }
    }
  }
  //println("doneTaylor");
  
  for(int j=mkside;j<img.height-mkside;j++){
    for(int i=mkside;i<img.width-mkside;i++){ 
      if(p[i][j].IsExtrema){
        Hessian(p,grayCopy,i,j);
      }
    }
  }
  //println("doneHessian");
  popMatrix();
  
  textAlign(CENTER,CENTER);
  fill(0,0,0);
  
  textSize(50);
  text("SIFT feature detection",width/2,50);
  
  textSize(24);
  text("red points: extrema points",width/2,img.height+24*5);
  text("green points: low contrast points",width/2,img.height+24*6);
  text("blue points: edges points",width/2,img.height+24*7);
  text("Gaussian filter size: "+mksize,width/2,img.height+24*8);
  text("# of octaves: 5",width/2,img.height+24*9);
  text("# of layers in one octave: "+(s+3),width/2,img.height+24*10);
}
