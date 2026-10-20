#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Controller.h"
#include "Command.h"
#include "mc9s12a128.h"
#include "EEProm.h"
#include "Interrupts.h"
#include "Subroutines.h"

unsigned int jmpAddress;




void command ( char CmdBuf[] )
{
    char ParseBuf[64] = "\0";
	
	if ( CmdBuf == SIN0Buf )
	    printf ( "\r\n" );
			 
    ucase( CmdBuf );
	
	while( strlen( CmdBuf ) && CmdBuf[0] != '\r' )
    {

        parse (ParseBuf, CmdBuf);
        
        if( !( strncmp( ParseBuf, "HELP", 4 ) ) || !( strncmp( ParseBuf, "?", 1 ) ) )
        {
			printf ( "/--------------------------- HELP MENU -------------------------------\\\r\n" );
            printf ( "|RESET                                                                |\r\n" );
            printf ( "|LOAD                                                                 |\r\n" );			
            printf ( "|GO                                                                   |\r\n" );			
			printf ( "|REVISION                                                             |\r\n" );			
			printf ( "\\---------------------------------------------------------------------/\r\n" );
            ParseBuf[0]='\0';
        }
        else if( !( strncmp( ParseBuf, "RESET", 5 ) ) || !( strncmp( ParseBuf, "#RESET", 6 ) ) )
        {
			 COPCTL = 0x01;				//enable COP 
			 while (1);					//wait for reset
		}
        else if( !( strncmp( ParseBuf, "LOAD", 4 ) ) )
        {
		 	 char tmpstr[8];
			 int ByteCnt, i,j;
			 char *Address;
			 
			 while (1)
			 {
    			 Gen_Flags &= ~Gen_Flags_SIN0Rcvd;
    			 while ( !(Gen_Flags & Gen_Flags_SIN0Rcvd) );
    			 
                 if ( (CmdBuf[0] == 'S') && (CmdBuf[1] == '1'))
                 {
                    tmpstr[0]= CmdBuf[2];
                    tmpstr[1]= CmdBuf[3];
                    tmpstr[2]= 0;
                    ByteCnt = hextoi ( tmpstr );
                    for ( i=0;i<4;i++ )
                    {
                        tmpstr[i]= CmdBuf[i+4];
                    }
                    tmpstr[4] = 0;
                    Address = (char*)hextoi ( tmpstr );
                    for ( j=8,i=3;i<ByteCnt;i++,j+=2 )
                    {
                        tmpstr[0]= CmdBuf[j];
                        tmpstr[1]= CmdBuf[j+1];
                        tmpstr[2]= 0;
                        *Address++ = (char)hextoi ( tmpstr );
                    }
                 }
    			 else if ( (CmdBuf[0] == 'S') && (CmdBuf[1] == '9') )
    			   	break;
			 }
			 ParseBuf[0]='\0';
		}
        else if( !( strncmp( ParseBuf, "GO", 2 ) ) )
        {
			 parse( ParseBuf, CmdBuf );
			 
			 jmpAddress = (unsigned int)hextoi ( ParseBuf );
			 
#ifdef BUTTONBOX_GCC
             ((void (*)(void))jmpAddress)();
#else
			 asm ("pshx");
			 asm ("ldx _jmpAddress");
			 asm ("jsr 0,x");
			 asm ("pulx");
#endif
			 ParseBuf[0]='\0';
		}
        else if( !( strncmp( ParseBuf, "REVISION", 8 ) ) )
        {
			printf (  "Revision %s\r\n", Revision );
            ParseBuf[0]='\0';
		}
        if (strlen(ParseBuf))
        {
			//printf ( "Command not recognized\r\n" );
			CmdBuf[0] = '\0';
			ParseBuf[0]='\0';
        }
    }		   
    //printf ( "\r\n>" );
    
}




char Checksum ( char *str )
{
 	char tmpstr[64], ParseBuf[5];
	char cksum = 0; 
    strcpy (tmpstr, str);
	while ( tmpstr[0] )
	{
	    parse( ParseBuf, tmpstr );
	    cksum += (unsigned int)hextoi ( ParseBuf );
	}
	return cksum;
	
}

void parse(char *pbuf, char *str)
{
	 char *ptr;
     ltrim (str);
     ptr = (char *)memchr (str, '\n', strlen(str));
	 if (ptr)
	 	*ptr = '\0';
     ptr = (char *)memchr (str, '\r', strlen(str));
	 if (ptr)
	 	*ptr = '\0';
     ptr = (char *)memchr (str, ' ', strlen(str));
     if (ptr)
     {
        *ptr = '\0';

        strcpy (pbuf, str);
        ptr++;
        memmove(str, ptr, strlen(ptr) + 1);
        return;
      }
      else
      {
        strcpy (pbuf,str);
        *str = '\0';
        return;
      }
}

void ucase (char *str)
{
     while (*str)
     {
           if (*str >= 'a' && *str <= 'z')
              *str -= 0x20;
           str++;
     }
     return;
}

void ltrim (char *c_ptr)
{
    char *first = c_ptr;
    while (*first == ' ')
        ++first;
    if (first != c_ptr)
        memmove(c_ptr, first, strlen(first) + 1);
}

int hextoi(char *string)
{
  int number = 0;
  int index;
 
  for (index = 0; string[index] != '\0'; index++)
    {
      char tmp = string[index];
      int digit;
 
      if (tmp >= '0' && tmp <= '9')
        digit = tmp - '0';
      else if (tmp >= 'A' && tmp <= 'F')
        digit = tmp - 'A' + 10;
      else if (tmp >= 'a' && tmp <= 'f')
        digit = tmp - 'a' + 10;
      else
        break;
 
      number = number * 16 + digit;
    }
  return(number);
}
