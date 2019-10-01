int mksize=15;
int mkside=(mksize-1)/2;
int segma=2;

float [][] xSqrtDervMask = new float [mksize][mksize];
float [][] ySqrtDervMask = new float [mksize][mksize];
float [][] xyDervMask = new float [mksize][mksize];

PImage img, gray;
float [][] R;

void setup(){
  size(480,320);
  //img=loadImage("face.bmp");
  img=loadImage("test1.jpg");
  //img=loadImage("test2.jpg");
  img.resize(480,320);
  R = new float[img.width][img.height];
  
  ellipseMode(CENTER);
  fill(255,0,0);
  stroke(255,0,0);
  
  //RGB to grayscale
  gray=createImage(img.width,img.height,ALPHA);
  img.loadPixels();
  for(int j=0;j<img.height;j++){
    for(int i=0;i<img.width;i++){
      color c=img.get(i,j);
      float C=red(c)*0.3+green(c)*0.59+blue(c)*0.11;
      gray.pixels[j*img.width+i]=color(C);
    }
  }
  gray.updatePixels();

  //second derivative of Gausian filter G
  //G = exp(-(x^2+y^2)/(2*(segma^2)))
  //[dG/dx] = Gx = (-x/(segma^2))*G
  //[d^2G/dx^2] = Gxx = ((x^2-semga^2))/(segma^4)*G
  for(int y=-mkside;y<=mkside;y++){
    for(int x=-mkside;x<=mkside;x++){
      xSqrtDervMask[y+mkside][x+mkside]=
      (pow(x,2)-pow(segma,2)) * exp(-(pow(x,2)+pow(y,2))/(2*pow(segma,2))) * 1.0 / pow(segma,4);
    }
  }

  //G = exp(-(x^2+y^2)/(2(segma^2)))
  //[dG/dy] = Gy = (-y/(segma^2))*G
  //[d^2G/dy^2] = Gyy = ((y^2-semga^2))/(segma^4)*G
  for(int y=-mkside;y<=mkside;y++){
    for(int x=-mkside;x<=mkside;x++){
      ySqrtDervMask[y+mkside][x+mkside]=
      (pow(y,2)-pow(segma,2)) * exp(-(pow(x,2)+pow(y,2))/(2*pow(segma,2))) * 1.0 / pow(segma,4);
    }
  }

  //G = exp(-(x^2+y^2)/(2(segma^2)))
  //[dG/dx] = Gx = (-y/(segma^2))*G
  //[d^2G/(dxdy)] = Gxy = (xy)/(segma^4)*G
  for(int y=-mkside;y<=mkside;y++){
    for(int x=-mkside;x<=mkside;x++){
      xyDervMask[y+mkside][x+mkside]=
      (x*y) * exp(-(pow(x,2)+pow(y,2))/(2*pow(segma,2))) * 1.0 / pow(segma,4);
    }
  }  
}

//Harris response calculation (R value)
float k=0.05; //k is empirically set as [0.04, 0.06]
float [][] calRvalue(float [][] R){
  gray.loadPixels();
  for(int j=mkside;j<gray.height-mkside;j++){
    for(int i=mkside;i<gray.width-mkside;i++){
      float xx=0.0, yy=0.0, xy=0.0;
      for(int dy=-mkside;dy<=mkside;dy++){
        for(int dx=-mkside;dx<=mkside;dx++){
          //differentiation can be written as a filter D
          //D conv (G conv I) = (D conv G) conv I
          
          //that is, in wikipedia page, w(u,v) conv Ix(u,v)^2 = wx(u,v)^2 conv I(u,v)
          //Ix and Iy are the partial derivatives of I
          //Here, we obtain the second derivative G (Gxx, Gyy, Gxy) and convolute with I
          
          //Gxx conv I 
          xx+=xSqrtDervMask[dy+mkside][dx+mkside]*gray.get(i+dx,j+dy);
          //Gyy conv I
          yy+=ySqrtDervMask[dy+mkside][dx+mkside]*gray.get(i+dx,j+dy);
          //Gxy conv I
          xy+=xyDervMask[dy+mkside][dx+mkside]*gray.get(i+dx,j+dy);
        }
      }
      //A = Gxx conv I 
      //B = Gyy conv I
      //C = Gxy conv I
      //M = [ A  C ] 
      //    [ C  B ]
      //R=det(M)-k*(trace(M)^2)
      R[i][j]=(xx*yy-xy*xy)-k*(pow((xx+yy),2));
    }
  }
  return R;
}

//Non-maximal suppression
int gap=7;
void NonMaximalSuppression(float globalMaxRvalue){
  for(int j=gap;j<img.height-gap;j++){
    for(int i=gap;i<img.width-gap;i++){
      int Maxi=i,Maxj=j;
      float MaxR=-1000000000;
      for(int dy=-gap;dy<=gap;dy++){
        for(int dx=-gap;dx<=gap;dx++){
          if(R[i+dx][j+dy]>MaxR){
            MaxR=R[i+dx][j+dy];
            Maxi=i+dx;
            Maxj=j+dy;
          }//end if
        }//end dx
      }//end dy
      
      //find the local maxima as corners within the window which is a (2*gap+1)-by-(2*gap+1) filter
      //Here, we add one more constraint, local maxima > (0.01 * globalMaxRvalue)
      if(Maxi==i && Maxj==j && MaxR>0.01*globalMaxRvalue){
        ellipse(i,j,5,5); //labels it as a corner
      }
    }//end i
  }//end y
}

//find global maximum of R value
float GlobalMaxR(float [][] R){
  float Max=-100000000;
  for(int j=mkside;j<img.height-mkside;j++){
    for(int i=mkside;i<img.width-mkside;i++){
      if(R[i][j]>Max){
        Max=R[i][j];
      }
    }
  }
  return Max;
}

void draw(){
  noLoop();
  image(img,0,0);
  //image(gray,0,0);
  NonMaximalSuppression(GlobalMaxR(calRvalue(R)));  
}
