/*
 * File:   newmain.c
 * Author: 김준규
 *
 * Created on 2021년 3월 11일 (목), 오후 1:04
 */


#define _XTAL_FREQ 20000000

//센서 입력
#define Vout_WS RA0 //풍속 센서
#define Xout RA1 //기울기 센서 x축
#define Yout RA2 //기울기 센서 y축

//LCD
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

//OUTPUT
#define HIGH RE0
#define LOW RE1

#include <xc.h>
#include "ESP8266_functions_0318.h"
#include <string.h>

//#include "MyLCD.h"
//#include "LCD_Lib.c"

#pragma config FOSC=HS
#pragma config WDTE=OFF //워치독 타이머 비활성화
//#pragma config MCLR=OFF 
#pragma config PWRTE=OFF // 파워 업 타이머 활성화
#pragma config BOREN=OFF //저전압 리셋 비활성화
#pragma config LVP = OFF // 저전압 (단일 전원) 인-서킷 직렬 프로그래밍 활성화 비트 (RB3는 디지털 I / O, HV 켜기 프로그래밍에 MCLR을 사용해야 함)
#pragma config CPD = OFF // 데이터 EEPROM 메모리 코드 보호 비트 (데이터 EEPROM 코드 보호 해제)
#pragma config WRT = OFF // 플래시 프로그램 메모리 쓰기 활성화 비트 (쓰기 방지 해제, 모든 프로그램 메모리 EECON 제어에 의해 기록 될 수 있음)
#pragma config CP = OFF // 플래시 프로그램 메모리 코드 보호 비트 (코드 보호 해제)

//LCD Functions Developed by Circuit Digest.
void Lcd_SetBit(char data_bit) //Based on the Hex value Set the Bits of the Data Lines
{
    if(data_bit& 1) 
        D4 = 1;
    else
        D4 = 0;

    if(data_bit& 2)
        D5 = 1;
    else
        D5 = 0;

    if(data_bit& 4)
        D6 = 1;
    else
        D6 = 0;

    if(data_bit& 8) 
        D7 = 1;
    else
        D7 = 0;
}

void Lcd_Cmd(char a)
{
    RS = 0;           
    Lcd_SetBit(a); //Incoming Hex value
    EN  = 1;         
    __delay_ms(4);
    EN  = 0;         
}

Lcd_Clear()
{
    Lcd_Cmd(0); //Clear the LCD
    Lcd_Cmd(1); //Move the curser to first position
}

void Lcd_Set_Cursor(char a, char b)
{
    char temp,z,y;
    if(a== 1)
    {
      temp = 0x80 + b - 1; //80H is used to move the curser
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
    else if(a== 2)
    {
        temp = 0xC0 + b - 1;
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
}

void Lcd_Start()
{
  Lcd_SetBit(0x00);
  for(int i=1065244; i<=0; i--)  NOP();  
  Lcd_Cmd(0x03);
    __delay_ms(5);
  Lcd_Cmd(0x03);
    __delay_ms(11);
  Lcd_Cmd(0x03); 
  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
  Lcd_Cmd(0x08); //Select Row 1
  Lcd_Cmd(0x00); //Clear Row 1 Display
  Lcd_Cmd(0x0C); //Select Row 2
  Lcd_Cmd(0x00); //Clear Row 2 Display
  Lcd_Cmd(0x06);
}

void Lcd_Print_Char(char data)  //Send 8-bits through 4-bit mode
{
   char Lower_Nibble,Upper_Nibble;
   Lower_Nibble = data&0x0F;
   Upper_Nibble = data&0xF0;
   RS = 1;             // => RS = 1
   Lcd_SetBit(Upper_Nibble>>4);             //Send upper half by shifting by 4
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP(); 
   EN = 0;
   Lcd_SetBit(Lower_Nibble); //Send Lower half
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP();
   EN = 0;
}

void Lcd_Print_String(char *a)
{
    int i;
    for(i=0;a[i]!='\0';i++)
       Lcd_Print_Char(a[i]);  //Split the string using pointers and call the Char function 
}

void Lcd_Write_Char(char c)
{
  RS = 1;  // characters
  PORTD = (PORTD & 0x0F) | (c & 0xF0);
  EN = 1;
  for(int i=2130483; i<=0; i--)  NOP();
  EN = 0;
  PORTD = (PORTD & 0x0F) | (c << 4);
  EN = 1;
  for(int i=2130483; i<=0; i--)  NOP(); 
  EN = 0;
  __delay_us(40);
}

void ESP8266_print_1(unsigned char *url)
{
    int i;
    unsigned char a[];
    _esp8266_print("AT+HTTPGETSIZE= ");
    _esp8266_print(url);
    _esp8266_print("\r\n");
    if(1)
    {
        for(i=0;a[i]!='\0';i++)
        {
            a[i] = _esp8266_getch();
            Lcd_Set_Cursor(i+1,1);
            Lcd_Print_Char(a[i]);
        }
    }
    
}

void ESP8266_check_Mode()
{
    //int i;
    char a;
    //_esp8266_print("AT+CIPMODE?\r\n");
    _esp8266_print("AT+CIPSTO?\r\n");
    Lcd_Set_Cursor(1,1);
    //_esp8266_waitResponse();
    do
    {
        a = _esp8266_getch();
        Lcd_Write_Char(a);
    }
    while(1);
            //a != '\0');
}

 void esp8266_get_stationIP1()
{
    char rex;
    ESP8266_send_string("AT+CWLIF\r\n");
    Lcd_Set_Cursor(1,1);
    Lcd_Print_String("IP:");
    do
    {
        rex = _esp8266_getch();
        Lcd_Print_String(rex);
    }while(rex!=',');
    Lcd_Set_Cursor(2,1);
    Lcd_Print_String("MAC:");
    do
    {
        rex = _esp8266_getch();
        Lcd_Print_String(rex);
    }while(rex!='O');
}

void main(void) {
    TRISD = 0x00;
    unsigned char test1 ="\0";
    int apiapi1 = 0;
    Lcd_Start();
    Initialize_ESP8266() ;
    Lcd_Set_Cursor(1,1);
    Lcd_Print_String("JINWOO ELTEC");
    __delay_ms(1500);
    Lcd_Clear(); //Lcd_Clear();
    char a;
     /* ESP_PIC 통신이 성공했는지 확인 */
    do
    {
        Lcd_Set_Cursor(1,1);
        Lcd_Print_String("ESP not found");
    }while (!esp8266_isStarted()); //wait till the ESP send back "OK"
    Lcd_Set_Cursor(1,1);
    Lcd_Print_String("ESP is connected");
    __delay_ms(1500);
    Lcd_Clear();
    /* 예 ESP 통신 성공 */
    
    /* 모듈을 Station 모드로 설정 */
    esp8266_mode(1);
    Lcd_Set_Cursor(1,1);
    Lcd_Print_String("ESP set as");
    Lcd_Set_Cursor(2,1);
    Lcd_Print_String("Station");
    __delay_ms(1500);
    Lcd_Clear();
    /* Module set as AP */

    /* AP 이름 및 암호 구성 */
    a = esp8266_config_Station("lab","lab12345");
    Lcd_Set_Cursor(1,1);
    Lcd_Print_String("AP configured");
    
    if(a==1)
    {
        Lcd_Set_Cursor(2,1);
        Lcd_Print_String("OK");
    }
    else
    {
        Lcd_Set_Cursor(2,1);
        Lcd_Print_String("ERROR");
    }
    __delay_ms(1500);
    Lcd_Clear();
    test1 = esp8266_getAPI();
    //apiapi1 = apiapi();
    Lcd_Print_String(test1);
    while(1)
    {
        //do nothing 
    }
}
