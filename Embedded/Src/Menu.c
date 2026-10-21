/* Internal menu navigation/editing. Storage remains in Subroutines.c. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Menu.h"
#include "Controller.h"
#include "Interrupts.h"
#include "Settings.h"
#include "Display.h"

void setDefaultMenu(void)
{
    int i;
	
	for (i = 0; i < MenuSize; i++)
	{
	    Menuc[i] = defaultMenuc[i];
    }
	
	memcpy(MenuStackc, defaultMenuStackc, sizeof(defaultMenuStackc));
	StackPointer = 0;
	VariableFlag = 0;    
}

int NullFunction ( void )
{
    return 0;
}

int setSerial ( void )
{

    if ( VariableFlag >= 5 )
    {
        VariableFlag = 0;
       	LoadMenu ( MenuStackc[StackPointer].Index );
       	InsertCursor ();
        Save_Serial_Num();
	}

    if( SerialNum[4] == '-' || SerialNum[3] == '-' || SerialNum[2] == '-' 
        || SerialNum[1] == '-' || SerialNum[0] == '-' )
    {
        if ( VariableFlag )
        {
            VariableFlag++;
        }
        else
        {
            VariableFlag = 1;
            increment = 1.0;    //numerical amount to increment or decrement
            lLimit = 0.0;      //numerical lower limit
            uLimit = 9.0;     //numerical upper limit
            nDigits = SER_NUM;    //number of digits to display including decimal x10

            LoadMenu ( MenuStackc[StackPointer].Index );
        }
    }
    btnDisplayTitler ();
    return 0;
}

int ToggleF1Enable(void)
{
    if (!strcmp(F1Enable, " ON")) strcpy(F1Enable, "OFF");
    else strcpy(F1Enable, " ON");
    NormalizeFunctionSettings();
    LoadMenu(MenuStackc[StackPointer].Index);
    InsertCursor();
    btnDisplayTitler();
    return 0;
}

int ToggleF2Enable(void)
{
    if (!strcmp(F2Enable, " ON")) strcpy(F2Enable, "OFF");
    else strcpy(F2Enable, " ON");
    NormalizeFunctionSettings();
    LoadMenu(MenuStackc[StackPointer].Index);
    InsertCursor();
    btnDisplayTitler();
    return 0;
}

int Var1to13Funct ( void )
{
    if ( VariableFlag )
    {
        VariableFlag = 0;
        LoadMenu ( MenuStackc[StackPointer].Index );
        InsertCursor ();
    }
    else
    {
        VariableFlag = 1;
        increment = 1.0;    //numerical amount to increment or decrement
        lLimit = 1.0;      //numerical lower limit
        uLimit = 13.0;     //numerical upper limit
        nDigits = 30;    //number of digits to display including decimal x10

        LoadMenu ( MenuStackc[StackPointer].Index );
    }
    btnDisplayTitler ();
    return 0;
}

int Var1to30Funct ( void )
{
    if ( VariableFlag )
    {
        VariableFlag = 0;
        LoadMenu ( MenuStackc[StackPointer].Index );
        InsertCursor ();
    }
    else
    {
        VariableFlag = 1;
        increment = 1.0;    //numerical amount to increment or decrement
        lLimit = 1.0;      //numerical lower limit
        uLimit = 30.0;     //numerical upper limit
        nDigits = 30;    //number of digits to display including decimal x10

        LoadMenu ( MenuStackc[StackPointer].Index );
    }
    btnDisplayTitler ();
    return 0;
}

int ExitMenu ( void )
{
    DeSelect ();
    return 0;
}

void CursorUp( void )
{
    if ( VariableFlag )
    {
         IncVariable ();
    }
    else
    {
       MenuStackc[StackPointer].CursorPos--;
       LoadMenu ( MenuStackc[StackPointer].Index );
       InsertCursor ();
       btnDisplayTitler ();
    }
}

void CursorDown( void )
{
    if ( VariableFlag )
    {
         DecVariable ();
    }
    else
    {
       MenuStackc[StackPointer].CursorPos++;
       LoadMenu ( MenuStackc[StackPointer].Index );
       InsertCursor ();
       btnDisplayTitler ();
    }
}

void IncVariable ( void )
{
    int k;
    int varIndex;       //index of variable
    double j = 0;
    char *vPtr2 = NULL;        //will also point to variable to be decremented

    if (StackPointer >= MenuStackSize)
        return;

     //Find out which menu we are on
     for ( k=0;k<MenuSize;k++ )
     {
        if ( Menuc[k].Index[0] == MenuStackc[StackPointer].Index[0] && 
            Menuc[k].Index[1] == MenuStackc[StackPointer].Index[1] && 
            Menuc[k].Index[2] == MenuStackc[StackPointer].Index[2] && 
            Menuc[k].Index[3] == MenuStackc[StackPointer].Index[3] )
        {
            varIndex = MenuStackc[StackPointer].CursorPos+MenuStackc[StackPointer].FirstLine-1;
            if (varIndex < 0 || varIndex >= 11)
                return;
            vPtr2 = Menuc[k].VarPntr[varIndex];     //copy pointer to variable
            if (vPtr2 == NULL) return;

            if( nDigits == OFF_ON ) {
                if ( !(strcmp ( vPtr2, "OFF" )) )
                    j = 1;      //switch to ON
                else
                    j = 0;      //switch to OFF
                break;     
            }

            if( nDigits == SER_NUM ) {
                if (VariableFlag < 1 || VariableFlag > 5) return;              //limit to numerical serial numbers
                if( SerialNum[VariableFlag -1]  == '-' )
                    SerialNum[VariableFlag -1]  = '0';
                else
                {
                    if( SerialNum[VariableFlag -1] < '9' )     //ensure it's within lower limit
                        SerialNum[VariableFlag -1]++;         //decrement value
                    else
                        SerialNum[VariableFlag -1] = '0';         //wrap around to lower limit
                }
                break;
            }

            //determine if variable is a numeric value
            if( nDigits > OFF_ON ) {
                //convert text to numeric value
                j =  atof ( vPtr2 );

                if( (j + increment) <= uLimit )     //ensure it's within upper limit
                    j += increment;     //increment value
                else
                    j = lLimit;        //wrap around to lower limit           
                break;     
            }
        }
    }
    if (vPtr2 == NULL)
        return;
    dispVariable(vPtr2, &j, nDigits);
    LoadMenu ( MenuStackc[StackPointer].Index );
    btnDisplayTitler ();
}

void DecVariable ( void )
{
    int k;
    int varIndex;       //index of variable
    double j = 0;
    char *vPtr2 = NULL;        //will also point to variable to be decremented

    if (StackPointer >= MenuStackSize)
        return;

    //Find out which menu we are on
    for ( k=0;k<MenuSize;k++ )
    {
        if ( Menuc[k].Index[0] == MenuStackc[StackPointer].Index[0] && Menuc[k].Index[1] == 
            MenuStackc[StackPointer].Index[1] && Menuc[k].Index[2] == 
            MenuStackc[StackPointer].Index[2] && Menuc[k].Index[3] ==
            MenuStackc[StackPointer].Index[3] )
        {
            //VarPnt = MenuStackc[StackPointer].CursorPos+MenuStackc[StackPointer].FirstLine-1;
            varIndex = MenuStackc[StackPointer].CursorPos+MenuStackc[StackPointer].FirstLine-1;
            if (varIndex < 0 || varIndex >= 11)
                return;
            vPtr2 = Menuc[k].VarPntr[varIndex];     //copy pointer to variable
            if (vPtr2 == NULL) return;
            
            if( nDigits == OFF_ON ) 
            {
                if ( !(strcmp ( vPtr2, "OFF" )) )
                    j = 1;      //switch to ON
                else
                    j = 0;      //switch to OFF
                break;     
            }
            if( nDigits == SER_NUM )               //limit to numerical serial numbers
            {
                if (VariableFlag < 1 || VariableFlag > 5) return;
                if( SerialNum[VariableFlag -1]  == '-' )
                    SerialNum[VariableFlag -1]  = '0';
                else
                {
                    if( SerialNum[VariableFlag -1] > '0' )     //ensure it's within upper limit
                        SerialNum[VariableFlag -1]--;         //increment value
                    else
                        SerialNum[VariableFlag -1] = '9';         //wrap around to upper limit
                }
                break;
            }
            //determine if variable is a numeric value format for numeric values is greater than OFF_ON(6)
            if( nDigits > OFF_ON )
            {
                //convert text to numeric value
                j =  atof ( vPtr2 );
                j -= increment;         //decrement value
                if( j < lLimit )        //
                {
                    if( j < ( lLimit - (increment/2) ) )        //ensure it's past lLimit
                        j = uLimit;         //wrap around to upper limit
                    else
                        j = lLimit;
                }
                break;
            }
        }
    }
    if (vPtr2 == NULL)
        return;
    dispVariable(vPtr2, &j, nDigits);
    LoadMenu ( MenuStackc[StackPointer].Index );
    btnDisplayTitler ();
}

/*****************************************
    display variable
//portable function to insert formatted variables into Menuc
//argument vPtr (variable pointer) points to the location of the variable
//argument value is the number to be displayed
//argument format, selects the format of the value
******************************************/

void dispVariable(char* vPtr, double *value, char format)
{
    switch( format ) {
        case 10:
            sprintf ( vPtr, "%1d", (int)*value );
            break;
        case 20:
            sprintf ( vPtr, "%2d", (int)*value );
            break;
        case 30:
            sprintf ( vPtr, "%3d", (int)*value );
            break;
        case 31:
            sprintf ( vPtr, "%#3.1f", *value );
            break;
        case 40:
            sprintf ( vPtr, "%4d", (int)*value );
            break;
        case 41:
            sprintf ( vPtr, "%#4.1f", *value );
            break;
        case 42:
            sprintf ( vPtr, "%#4.2f", *value );
            break;
        case 50:
            sprintf ( vPtr, "%5d", (int)*value );
            break;
        case 52:
            sprintf ( vPtr, "%#5.2f", *value );
            break;
        case SER_NUM:        // 8
            break;
        case OFF_ON:        // 9
            if( *value )     //on
                sprintf ( vPtr, " ON" );
            else            //off
                sprintf ( vPtr, "OFF" );
            break;
    }
}

void Select ( void )
{
     int k;
     
     //Save current menu pointers
     MenuStackc[StackPointer+1].Index[0]=MenuStackc[StackPointer].Index[1];
     MenuStackc[StackPointer+1].Index[1]=MenuStackc[StackPointer].Index[2];
     MenuStackc[StackPointer+1].Index[2]=MenuStackc[StackPointer].Index[3];
     MenuStackc[StackPointer+1].Index[3] = MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine;

     //look for selected menu 
     for ( k=0;k<MenuSize;k++ )
     {
        if ( Menuc[k].Index[0] == MenuStackc[StackPointer+1].Index[0] && 
            Menuc[k].Index[1] == MenuStackc[StackPointer+1].Index[1] && 
            Menuc[k].Index[2] == MenuStackc[StackPointer+1].Index[2] && 
            Menuc[k].Index[3] == MenuStackc[StackPointer+1].Index[3] )
        {
             //If found increment stackpointer and initialize cursor
             StackPointer++;
             //Cursor to menu entry 1
             MenuStackc[StackPointer].CursorPos = 1;
             MenuStackc[StackPointer].FirstLine = 0;
             LoadMenu ( MenuStackc[StackPointer].Index );
             InsertCursor ();
             btnDisplayTitler ();
             break;
        }
     } 

     if ( k >= MenuSize )
     {
         //Find out which menu we are on
         for ( k=0;k<MenuSize;k++ )
         {
            if ( Menuc[k].Index[0] == MenuStackc[StackPointer].Index[0] && Menuc[k].Index[1] == MenuStackc[StackPointer].Index[1] && 
                 Menuc[k].Index[2] == MenuStackc[StackPointer].Index[2] && Menuc[k].Index[3] == MenuStackc[StackPointer].Index[3] )
            {
                 //Execute function for that entry 
                 Menuc[k].FunctPtr[(MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine)-1]();//(MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine,0);
                 //LoadMenu ( MenuStackc[StackPointer].Index );
                 //DisplayMenu ();
                 break;
            }
         }
     }
}

void DeSelect ( void )
{
     if (StackPointer )
     {
        StackPointer--;
        LoadMenu ( MenuStackc[StackPointer].Index );
        InsertCursor ();
        btnDisplayTitler ();
     }
     else
     {
        Gen_Flags &= ~Gen_Flags_Menu_Active;
        ClearTitler ();
        Save_Variables ();
     }
	 VariableFlag = 0;
}

void LoadMenu ( unsigned char Index[] )
{
    int i, j, k;

    //find menu
    for ( k=0;k<MenuSize;k++ )
    {
        if ( Menuc[k].Index[0] == Index[0] && Menuc[k].Index[1] == Index[1] && 
             Menuc[k].Index[2] == Index[2] && Menuc[k].Index[3] == Index[3] )
             break;
    } 

    //Get Menu Title
    for ( j=0;j<=20;j++ )
	{
        Menu[0][j] = Menuc[k].Entry[0][j];
	}

    //See if this entry is a valid menu entry, if not start cursor back at top
    if ( MenuStackc[StackPointer].CursorPos > 0 && Menuc[k].Entry[MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine][1] == ' ' )
    {
         MenuStackc[StackPointer].CursorPos = 1;
         MenuStackc[StackPointer].FirstLine = 0;
    }
        
    //Allow a maximum of 12 entries, if past 12, start cursor back at top
    if ( MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine >= 12 )
    {
         MenuStackc[StackPointer].CursorPos = 1;
         MenuStackc[StackPointer].FirstLine = 0;
    }
        
    //If cursor is too far down, check to see if there is another entry, if so, show it and set cursor to bottom entry.
    if ( MenuStackc[StackPointer].CursorPos == 9 )
    {
         if ( Menuc[k].Entry[9+MenuStackc[StackPointer].FirstLine][1] != ' ' )
         {
            MenuStackc[StackPointer].FirstLine++;
            MenuStackc[StackPointer].CursorPos = 8;
         }
         else
         {
             MenuStackc[StackPointer].CursorPos = 1;
             MenuStackc[StackPointer].FirstLine = 0;
         }
    }

    //If cursor is too far up, and the first entry is not the first line then move firstline up and set cursor to top entry
    if ( MenuStackc[StackPointer].CursorPos == 0 )
    {
         if ( MenuStackc[StackPointer].FirstLine )
         {
            MenuStackc[StackPointer].FirstLine--;
            MenuStackc[StackPointer].CursorPos = 1;
         }
         else
         {
             //Find last entry
             for ( i=11;i>0;i-- )
             {
                 if ( Menuc[k].Entry[i][1] != ' ' && Menuc[k].Entry[i][0] == ' ' )
                      break;
             }
             //If more than 8 entries, cursor will be at the bottom (8) and first line will be adjusted
             if ( i > 8 )
             {
                 MenuStackc[StackPointer].CursorPos = 8;
                 MenuStackc[StackPointer].FirstLine = i - 8;
             }
             //Otherwise the cursor will be on that entry with the firstline at 0
             else
             {
                 MenuStackc[StackPointer].CursorPos = i;
                 MenuStackc[StackPointer].FirstLine = 0;
             }
         }
    }

    //Load appropriate menu entries into Menu[i][j]
    for ( i=1;i<=8;i++ )
	{
	    for ( j=0;j<=20;j++ )
		{
            Menu[i][j] = Menuc[k].Entry[i+MenuStackc[StackPointer].FirstLine][j];
		}
	}

	//Load any variables
    for ( i=1;i<=8;i++ )
	{
        if ( Menuc[k].Pos[i+MenuStackc[StackPointer].FirstLine-1] )
        {
             if ( VariableFlag && i == MenuStackc[StackPointer].CursorPos )
                  Menu[i][Menuc[k].Pos[i+MenuStackc[StackPointer].FirstLine-1]-1] = '>';
             for ( j=0;j<=20;j++ )
             {
                 if ( *(Menuc[k].VarPntr[i+MenuStackc[StackPointer].FirstLine-1]+j) )
                 {
                    Menu[i][Menuc[k].Pos[i+MenuStackc[StackPointer].FirstLine-1]+j] = *(Menuc[k].VarPntr[i+MenuStackc[StackPointer].FirstLine-1]+j);
                 }
                 else
                 {
                     break;
                 }
             }
        }
	}
}

void InsertCursor ( void )
{
	//Display Cursor Pointer
    Menu[MenuStackc[StackPointer].CursorPos][0] = '>';
}

void btnDisplayTitler ( void )
{
	int i, j, k;
	
	for ( i=0;i<=8;i++ )
	{
	    for ( j=0;j<=15;j+=5 )
		{
		 	for ( k=0;k<=4;k++ )
			{
				DispBuf[i*20+j+k+6] = Menu[i][j+k];  //the menu is treated as recieve over CAN
									  				 //the first 6 char are (stx)MENU:
													 //strcpy() would work better here
			}
    	    COP_Trig();
		}
	}
	showMenu ();
}

int RestoreDefaults ( void )
{
    strcpy(F1Enable, " ON");
    strcpy(F2Enable, " ON");
    NormalizeFunctionSettings();
    FirstVar[0] = 0xFF;    //indicates data needs to be replaced with defaults
    Save_Variables();         
    ResetProc ();
    return 0;
}
