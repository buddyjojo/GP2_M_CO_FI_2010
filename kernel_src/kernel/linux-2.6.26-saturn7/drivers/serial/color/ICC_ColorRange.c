
unsigned char g_ICC_Region[7][9][3] = {
{ //Red
  { 0x00,0x00,0x01}, //Enable 
  { 0x3f,0x80,0x00}, //Center 
  { 0x03,0xc0,0x00}, //Apture
  { 0x03,0xc0,0x00}, //Fading Saturation CCK
  { 0x03,0xc0,0x00}, //Fading Saturation CK
  { 0x00,0x33,0x33}, //Saturation Max
  { 0x00,0x19,0x9a}, //Saturation Min
  { 0x00,0x4c,0xcd}, //Fading Saturation Max
  { 0x00,0x06,0x66}, //Fading Saturation Min
}, 
{ //Green
  { 0x00,0x00,0x00}, //Enable 
  { 0x00,0x00,0x00}, //Center 
  { 0x00,0x00,0x00}, //Apture
  { 0x00,0x00,0x00}, //Fading Saturation CCK
  { 0x00,0x00,0x00}, //Fading Saturation CK
  { 0x00,0x00,0x00}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x00,0x00}, //Fading Saturation Max
  { 0x00,0x00,0x00}, //Fading Saturation Min
}, 
{ //Blue
  { 0x00,0x00,0x00}, //Enable 
  { 0x00,0x00,0x00}, //Center 
  { 0x00,0x00,0x00}, //Apture
  { 0x00,0x00,0x00}, //Fading Saturation CCK
  { 0x00,0x00,0x00}, //Fading Saturation CK
  { 0x00,0x00,0x00}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x00,0x00}, //Fading Saturation Max
  { 0x00,0x00,0x00}, //Fading Saturation Min
}, 
{ //Cyan
  { 0x00,0x00,0x00}, //Enable 
  { 0x00,0x00,0x00}, //Center 
  { 0x00,0x00,0x00}, //Apture
  { 0x00,0x00,0x00}, //Fading Saturation CCK
  { 0x00,0x00,0x00}, //Fading Saturation CK
  { 0x00,0x00,0x00}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x00,0x00}, //Fading Saturation Max
  { 0x00,0x00,0x00}, //Fading Saturation Min
}, 
{ //Magenta
  { 0x00,0x00,0x00}, //Enable 
  { 0x00,0x00,0x00}, //Center 
  { 0x00,0x00,0x00}, //Apture
  { 0x00,0x00,0x00}, //Fading Saturation CCK
  { 0x00,0x00,0x00}, //Fading Saturation CK
  { 0x00,0x00,0x00}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x00,0x00}, //Fading Saturation Max
  { 0x00,0x00,0x00}, //Fading Saturation Min
}, 
{ //Yellow
  { 0x00,0x00,0x00}, //Enable 
  { 0x00,0x00,0x00}, //Center 
  { 0x00,0x00,0x00}, //Apture
  { 0x00,0x00,0x00}, //Fading Saturation CCK
  { 0x00,0x00,0x00}, //Fading Saturation CK
  { 0x00,0x00,0x00}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x00,0x00}, //Fading Saturation Max
  { 0x00,0x00,0x00}, //Fading Saturation Min
}, 
{ //Flesh
  { 0x00,0x00,0x00}, //Enable 
  { 0x00,0x00,0x00}, //Center 
  { 0x00,0x00,0x00}, //Apture
  { 0x00,0x00,0x00}, //Fading Saturation CCK
  { 0x00,0x00,0x00}, //Fading Saturation CK
  { 0x00,0x00,0x00}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x00,0x00}, //Fading Saturation Max
  { 0x00,0x00,0x00}, //Fading Saturation Min
}, 
}; 

/*LUT_ICC_gain_Region = {
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   0,   0,
	   9,   0,   0,   0,  73,  57,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  89,   0,   0,
	  73,   0,   0,   0,  73,  65,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,  97,  49,   0,   0,  17,   0,   0,   0,  57,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  89,   0,   0,   0,  89,  89,   1,   0,  49,   0,   0,   0,  49,  49,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
}; */
