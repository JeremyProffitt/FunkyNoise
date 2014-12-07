/*
Helpfull functions to keep the actual animation code short. 
 Contains so far:
 
 XY()
 FillNoise(byte layer)
 InitCoordinates()
 ShowFrame()
 Line(int x0, int y0, int x1, int y1, byte color)
 DimAll(byte value)
 CLS()
 -----------------------------------------------------------------
 */


// Translate the x/y coordinates into the right index in the
// framebuffer.
// The Smartmatrix has a simple line by line layout, no 
// serpentines. It safes 2 fps to keep this function short.
// The function is called over 200 000 times per second!

uint16_t XY( uint8_t x, uint8_t y) {
  uint16_t i;
  i = (y * kMatrixWidth) + x;
  return i;
}


// Fill the x/y array with 16-bit noise values 

void FillNoise(byte layer) {

  for(uint8_t i = 0; i < kMatrixWidth; i++) {

    uint32_t ioffset = scale_x[layer] * (i-CentreX);

    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      uint32_t joffset = scale_y[layer] * (j-CentreY);

      byte data = inoise16(x[layer] + ioffset, y[layer] + joffset, z[layer]) >> 8;

      // Marks data smoothing
      data = qsub8(data,16);
      data = qadd8(data,scale8(data,39));

      uint8_t olddata = noise[layer][i][j];
      uint8_t newdata = scale8( olddata, noisesmoothing ) + scale8( data, 256 - noisesmoothing );
      data = newdata;

      noise[layer][i][j] = data;
    }
  }
}


// Initialise the coordinates of the noise space with random
// values for an altering starting point.
// Set the zoom factor to a moderate level.

void InitCoordinates() {

  // just any free input pin
  random16_add_entropy(analogRead(18));

  for(int i = 0; i < NUM_LAYERS; i++) {
    x[i] = random16();
    y[i] = random16();
    z[i] = random16();
    scale_x[i] = 6000;
    scale_y[i] = 6000;
  }
}


// Update leds and show fps
// 216 fps when calling nothing else

void ShowFrame() {

  // update leds
  LEDS.show();

  // count and output the fps
  LEDS.countFPS();
  Serial.print("FPS: ");
  Serial.println(LEDS.getFPS());

  /*
  ShowParameters(0);
   ShowParameters(1);
   ShowParameters(2);
   */
  SerialWriteNoiseValue(0);
  SerialWriteNoiseValue(1);
  SerialWriteNoiseValue(2);
}


/*
// Bresenham line algorythm draws a colored line
 // between 2 given points
 
 void Line(int x0, int y0, int x1, int y1, byte color) {
 int dx =  abs(x1-x0), sx = x0 < x1 ? 1 : -1;
 int dy = -abs(y1-y0), sy = y0 < y1 ? 1 : -1;
 int err = dx + dy, e2; 
 for(;;) {  
 leds[XY(x0, y0)] = CHSV(color, 255, 255);
 if (x0 == x1 && y0 == y1) break;
 e2 = 2 * err;
 if (e2 > dy) { 
 err += dy; 
 x0 += sx; 
 } 
 if (e2 < dx) { 
 err += dx; 
 y0 += sy; 
 } 
 }
 }
 */


// Dim everything in leds a bit down.

void DimAll(byte value)  
{
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(value);
  }
}


// Delete the leds array.

void CLS()  
{
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = 0;
  }
}

//probably useless
void CorrectBrightness() {
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      uint16_t  bri = (noise[2][i][j]);

      bri = bri - 15000;

      bri = bri * 1.3;


    }
  }

}

// overlay layers 0&1&2 for color, layer 2 is brightness
void MergeMethod1(byte colorrepeat) { 
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      // map the noise values down to a byte range
      // layer 0 and 2 interfere for the color
      uint8_t color = ( (noise[0][i][j] )
        +(noise[1][i][j])
        +(noise[2][i][j]))
        /3
          ; 

      // layer 1 gives the brightness  
      uint8_t   bri = (noise[2][i][j]);
      //uint8_t   bri = 255;
      // assign a color depending on the actual palette
      CRGB pixel = ColorFromPalette( currentPalette, colorrepeat * (color+colorshift), bri );

      leds[XY(i,j)] = pixel;
    }
  }
} 

// overlay layers 0&1 for color, layer 2 is brightness
void MergeMethod2(byte colorrepeat) { 
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      // map the noise values down to a byte range
      // layer 0 and 2 interfere for the color
      uint8_t color = ( (noise[0][i][j] )
        +(noise[1][i][j]))
        /2
          ; 

      // layer 1 gives the brightness  
      uint8_t   bri = (noise[2][i][j]);
      //uint8_t   bri = 255;
      // assign a color depending on the actual palette
      CRGB pixel = ColorFromPalette( currentPalette, colorrepeat * (color+colorshift), bri );

      leds[XY(i,j)] = pixel;
    }
  }
}


// overlay layers 0&1 for color, brightness is layer1
void MergeMethod3(byte colorrepeat) { 
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      // map the noise values down to a byte range
      // layer 0 and 2 interfere for the color
      uint8_t color = ( (noise[0][i][j] )
        +(noise[1][i][j]))
        /2
          ; 

      // layer 1 gives the brightness  
      uint8_t   bri = noise[1][i][j];
      //uint8_t   bri = 255;
      // assign a color depending on the actual palette
      CRGB pixel = ColorFromPalette( currentPalette, colorrepeat * (color+colorshift), bri );

      leds[XY(i,j)] = pixel;
    }
  }
}



// draw the part between lower and upper limit of one layer
void ConstrainedMapping(byte layer, byte lower_limit, byte upper_limit, byte colorrepeat) {

  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      uint8_t data =  noise[layer][i][j] ;

      if ( data >= lower_limit  && data <= upper_limit) {

        CRGB pixel = ColorFromPalette( currentPalette, colorrepeat*(data+colorshift), data );

        leds[XY(i,j)] = pixel;
      }
    }
  }
}




void ShowAll(uint16_t count) {
  for(uint16_t i = 0; i < count; i++) {
    Lavalamp1();
    ShowFrame();
  }  
  for(uint16_t i = 0; i < count; i++) {
    Lavalamp2();
    ShowFrame();
  }

  for(uint16_t i = 0; i < count; i++) {
    Lavalamp3();
    ShowFrame();
  }


  for(uint16_t i = 0; i < count; i++) {
    Lavalamp4();
    ShowFrame();
  }

  for(uint16_t i = 0; i < count; i++) {
    Lavalamp5();
    ShowFrame();
  }
  
    for(uint16_t i = 0; i < count; i++) {
    Constrained1();
    ShowFrame();
  }
}

