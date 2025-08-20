
unsigned char g_IHC_Region[7][9][3] = {
{ //Red
  { 0x00,0x00,0x01}, //Enable 
  { 0x3f,0x80,0x00}, //Center 
  { 0x03,0xc0,0x00}, //Apture
  { 0x03,0xc0,0x00}, //Fading Saturation CCK
  { 0x03,0xc0,0x00}, //Fading Saturation CK
  { 0x00,0x59,0x9a}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x59,0x9a}, //Fading Saturation Max
  { 0x00,0x00,0x00} //Fading Saturation Min
}, 
{ //Green
  { 0x00,0x00,0x01}, //Enable 
  { 0x20,0x40,0x00}, //Center 
  { 0x05,0x00,0x00}, //Apture
  { 0x05,0x00,0x00}, //Fading Saturation CCK
  { 0x05,0x00,0x00}, //Fading Saturation CK
  { 0x00,0x59,0x9a}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x59,0x9a}, //Fading Saturation Max
  { 0x00,0x00,0x00} //Fading Saturation Min
}, 
{ //Blue
  { 0x00,0x00,0x01}, //Enable 
  { 0x01,0xc0,0x00}, //Center 
  { 0x05,0x00,0x00}, //Apture
  { 0x05,0x00,0x00}, //Fading Saturation CCK
  { 0x05,0x00,0x00}, //Fading Saturation CK
  { 0x00,0x59,0x9a}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x59,0x9a}, //Fading Saturation Max
  { 0x00,0x00,0x00} //Fading Saturation Min
}, 
{ //Cyan
  { 0x00,0x00,0x01}, //Enable 
  { 0x12,0x80,0x00}, //Center 
  { 0x05,0x00,0x00}, //Apture
  { 0x05,0x00,0x00}, //Fading Saturation CCK
  { 0x05,0x00,0x00}, //Fading Saturation CK
  { 0x00,0x59,0x9a}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x59,0x9a}, //Fading Saturation Max
  { 0x00,0x00,0x00} //Fading Saturation Min
}, 
{ //Magenta
  { 0x00,0x00,0x01}, //Enable 
  { 0x4d,0x40,0x00}, //Center 
  { 0x05,0x00,0x00}, //Apture
  { 0x05,0x00,0x00}, //Fading Saturation CCK
  { 0x05,0x00,0x00}, //Fading Saturation CK
  { 0x00,0x59,0x9a}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x59,0x9a}, //Fading Saturation Max
  { 0x00,0x00,0x00} //Fading Saturation Min
}, 
{ //Yellow
  { 0x00,0x00,0x01}, //Enable 
  { 0x2e,0xc0,0x00}, //Center 
  { 0x03,0xc0,0x00}, //Apture
  { 0x03,0xc0,0x00}, //Fading Saturation CCK
  { 0x03,0xc0,0x00}, //Fading Saturation CK
  { 0x00,0x59,0x9a}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x59,0x9a}, //Fading Saturation Max
  { 0x00,0x00,0x00} //Fading Saturation Min
}, 
{ //Flesh
  { 0x00,0x00,0x01}, //Enable 
  { 0x37,0x00,0x00}, //Center 
  { 0x02,0x00,0x00}, //Apture
  { 0x02,0x00,0x00}, //Fading Saturation CCK
  { 0x02,0x00,0x00}, //Fading Saturation CK
  { 0x00,0x59,0x9a}, //Saturation Max
  { 0x00,0x00,0x00}, //Saturation Min
  { 0x00,0x59,0x9a}, //Fading Saturation Max
  { 0x00,0x00,0x00} //Fading Saturation Min
}, 
}; 

/*LUT_IHC_gain_Region = {
	   0, 114, 114,  90,  58,  10,   0,   0,  36,  84, 116, 108,  76,  36,   0,   0,   0,
	  74,  98, 114, 114,  82,  34,   0,   0,  36,  92, 116,  92,  52,   4,   0,   0,   0,
	  42,  74,  98, 114, 106,  58,   0,   0,  36, 100, 116,  76,  20,   0,   0,   0,   0,
	  10,  34,  66,  98, 114,  90,  26,   0,  36, 108, 100,  44,   0,   0,   0,   0,   0,
	   0,   0,  26,  58,  98, 114,  58,   0,  36, 116,  76,   0,   0,   0,   0,   0,   3,
	   0,   0,   0,   0,  42,  98, 106,   0,  36, 116,  20,   0,   0,   0,   3,  35,  51,
	   0,   0,   0,   0,   0,  26,  98,  58,  36,  76,   0,   0,   3,  43,  67,  83,  91,
	  14,   0,   0,   0,   0,   0,   0,  98,  36,   0,   3,  67,  91, 107, 115, 115, 115,
	  62,  62,  62,  62,  62,  62,  62,  62,  91,  91,  91,  91,  91,  91,  91,  91,  91,
	  94,  94,  86,  78,  62,  30,   0,  31,   0, 101,   0,   0,   0,  19,  27,  43,  43,
	  62,  54,  30,   6,   0,  23,  31,  33,   0,  61, 101,  29,   0,   0,   0,   0,   0,
	  14,   0,   0,   0,  39,  31,   0,  81,   0,   0, 109, 101,  45,   0,   0,   0,   0,
	   0,   0,  23,  47,  31,   0,  33,  81,   0,   0,  61, 117, 101,  61,  29,   0,   0,
	   7,  31,  55,  31,   0,   1,  65,  73,   0,   0,  29,  93, 117, 101,  69,  37,  13,
	  39,  55,  31,   0,   0,  33,  81,  65,   0,   0,   0,  61, 109, 117, 101,  77,  45,
	  55,  31,   0,   0,   9,  57,  89,  49,   0,   0,   0,  37,  85, 117, 117, 101,  77,
	   0,   0,   0,   0,  33,  73,  81,  49,   0,   0,   0,  13,  61,  93, 117, 117,   0,
};*/
