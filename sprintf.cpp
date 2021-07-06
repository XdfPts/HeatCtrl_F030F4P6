 #include <stdarg.h>
 #include <string.h>
 #include <math.h>
 #include "mytype.h"


 //void	mntPutChar	(int Sym);
 void	TrcChar(char ch);//{};  i2c

 #define MaxBufSz 12	//32Bits go up to 4GB + 1 Byte for \0


 static unsigned wasAdd;
 static char*	 StringSptr;

 //-------------------------------------------------------------
byte*		Dig2Ascii(byte* ptr,unsigned u_val,uns base,uns width,uns fill)
 //-------------------------------------------------------------
 {
    *--ptr = 0;
	do
	{
        byte ch = u_val % base + '0';
        if (ch > '9')
         {
          ch += 'A' - '9' - 1;
         }
        *--ptr = ch;
        u_val /= base;
        if(width) width--;
	} while (u_val);
	if (width) while(width--) *--ptr = fill; //insert padding chars
	return  ptr;
 }
 #ifdef USE_FPOINT
 static char	 gcvtBuf[21];
 //--------------------------------------------
char*		ftoa(double v)
 //--------------------------------------------
 { int i; char s;
   char* ptr;
   char* fptr;
   s = 0;
   if (v < 0) { s = '-'; v=-v; };
   int intp   = (int)v;
   ptr  = (char*)Dig2Ascii( (byte*)gcvtBuf + 12,intp,10,0,0);
   if (s) *--ptr = s;
   fptr = gcvtBuf + 11;
   *fptr++ = '.';
   double frq =  v - intp;
   for(i=0;i<3;i++)
   {
     frq *= 10.;
	 intp = (int)frq;
     *fptr++ = intp + '0';
	 frq -= intp;
   }
   *fptr = 0;
   return ptr;
 }
 #endif
 //-------------------------------------------------------------
u32		sss_cpy(putnF putch,cchar* p)
 //-------------------------------------------------------------
 {  cchar* strt = p;
    while(*p) { putch(*p++); }
	  return p - strt;

 }
 //-------------------------------------------------------------
unsigned	SSS(putnF putch,char const *format,void* ArgArr[])
 //-------------------------------------------------------------
 {
  byte   format_flag;
  byte   scratch[MaxBufSz];
  uns    base,argn;
  u32	 strn;
  //char*  ptr;

  byte 			issigned=0;
  u32			u_val=0;
  int 			s_val=0;

  unsigned char fill;
  unsigned char width;

  argn   =  wasAdd = 0;
  for (;;)
  {
    while ((format_flag = *(format++)) != '%')
	{      // Until '%' or '\0'
      if (!format_flag) return wasAdd;
	  putch(format_flag);
    }

    issigned=0; //default unsigned
    width	=0; //no formatting
    fill	=0; //no formatting
    base = 10;
    format_flag = *format++; //get char after '%'
    if(format_flag=='0' || format_flag==' ') //SPACE or ZERO padding  ?
    {
      fill=format_flag;
      format_flag = *format++; //get char after padding char
	}
    if(format_flag>='0' && format_flag<='9')
    {
        width=format_flag-'0';
        format_flag = *format++; //get char after width char
    }


    //islong=0; //default int value
    if(format_flag=='l' || format_flag=='L') //Long value
     {
      //islong=1;
      format_flag = *format++; //get char after 'l' or 'L'
     }

    switch (format_flag)
    {
    case 'c':
    case 'C':
      		format_flag = (unsigned)ArgArr[argn++];  // no break -> run into default
    default:
      		putch(format_flag);
      		continue;
	#ifdef USE_FPOINT
    case 'f':
			{
  			 double dblv;
			 memcpy(&dblv,&ArgArr[argn++],sz(dblv));
			 argn++;
			 sss_cpy(putch,ftoa(dblv);
      		 //while(*ptr) { putch(*ptr); ptr++; }
			 continue;
			}
	#endif

    case 'S':
    case 's':
			strn = sss_cpy(putch,(char*)ArgArr[argn++]);
			while (strn++ < width) putch(' ');
      		continue;

    case 'i':
    case 'I':
    case 'd':
    case 'D':
      		issigned=1;      // no break -> run into next case
    case 'u':
    case 'U':
      		goto CONVERSION_LOOP;
    case 'x':
    case 'X':
    case 'p':
    case 'P':
	      base = 16;
	CONVERSION_LOOP:
			if(issigned) //Signed types
	         {
				s_val = (int)ArgArr[argn++];
	        	if(s_val < 0) //Value negativ ?
    	     	{
	          	 s_val = - s_val; //Make it positiv
	          	 putch('-');    //Output sign
	         	}
				u_val = (u32)s_val;
			}
			else //Unsigned types
			{
			  u_val = (unsigned)ArgArr[argn++];
			}
			sss_cpy(putch,(const char*)
					Dig2Ascii(scratch + MaxBufSz,u_val,base,width,fill));
			continue;
    }//switch
  }//for;;
 }//SSS


 //----------------------------------------------
void		StrPutChar(unsigned char c)
 //----------------------------------------------
 {
	StringSptr[wasAdd++] = c;
	StringSptr[wasAdd]   = 0;
 }
 //----------------------------------------------
u32		vsprintf(char* outStr,const char*str,void* ArgArr[])
 //----------------------------------------------
 {
	 StringSptr = outStr;
     return  SSS(StrPutChar,str,ArgArr);
 }

 //----------------------------------------------
u32		vprintf(const char* str,void* pp)
 //----------------------------------------------
 {
     u32 rv=SSS((putnF)TrcChar,str,(void**)pp);
	 //mntPutChar(22);
	 return rv;
 }
 //----------------------------------------------
u32		sprintf(char* str,const char * sh,...)
 //----------------------------------------------
 {
	 void* pp;	 va_list ap;  va_start(ap,sh);	 pp = ap.__ap;	 va_end(ap);
     return  vsprintf(str,sh,(void**)pp);
 }

 extern "C"
 //----------------------------------------------
u32		printf(const char* str,...)
 //----------------------------------------------
 {
	 void* pp;	 va_list ap;	 va_start(ap,str);	 pp = ap.__ap;	 va_end(ap);
     return  vprintf(str,pp);
 }
 //----------------------------------------------
unsigned	Strlen(const char* Str)
 //----------------------------------------------
 { unsigned l = 0;
   for(;;l++)
   	if (Str[l] == 0) return l;
 }

