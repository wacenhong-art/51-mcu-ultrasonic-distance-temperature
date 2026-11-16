#include <REG52.H>
#include <intrins.h>
#ifndef IMPROVED
#define IMPROVED 1
#endif
#ifndef TEMP_P26
#define TEMP_P26 0
#endif
typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;
sbit TRIG=P1^0;
sbit ECHO=P1^1;
sbit KEY=P3^1;
sbit BEEP=P2^5;
#if TEMP_P26
sbit DQ=P2^6;
#else
sbit DQ=P3^7;
#endif
static u8 code digits[]={0x3f,6,0x5b,0x4f,0x66,0x6d,0x7d,7,0x7f,0x6f};
static volatile u8 screen[8],key_event,alarm;
static volatile u16 ticks;
static u8 locked,temp_valid;
static int temp10;
static u16 distance,shown;
static u8 valid,shown_valid;

/* Timer 1 remains a hardware timebase during ISR and 1-Wire slots. */
static u16 timer1(void) { u8 h,l; do {h=TH1;l=TL1;} while(h!=TH1); return ((u16)h<<8)|l; }
static void wait_us(u16 us) {u16 s=timer1(),n=(u16)(((u32)us*576+624)/625);while((u16)(timer1()-s)<n);}
static u16 now(void) {u16 n;EA=0;n=ticks;EA=1;return n;}
void scan(void) interrupt 1
{
    static u8 pos,stable=1,last=1,count,armed=1;
    u8 k;
    TH0=0xfc;TL0=0x66;++ticks;
    P0=0;P2=(P2&0xe3)|((7-pos)<<2);P0=screen[pos];pos=(pos+1)&7;
    if(alarm)BEEP=!BEEP;else BEEP=0;
    k=KEY;
    if(k!=last){last=k;count=0;}else if(count<20)++count;
    if(count==20 && stable!=k){stable=k;if(k)armed=1;else if(armed){armed=0;key_event=1;}}
}
static void render(void)
{
    u8 b[8],i;
#if IMPROVED
    u16 a;
#endif
    for(i=0;i<8;i++)b[i]=0;
    if(shown_valid){b[0]=digits[shown/100];b[1]=digits[shown/10%10]|0x80;b[2]=digits[shown%10];}
    else {b[0]=b[1]=b[2]=0x40;}
    b[3]=locked?0x38:0; /* L indicates distance hold. */
#if IMPROVED
    {
        if(!temp_valid){for(i=4;i<8;i++)b[i]=0x40;}
        else {
            a=temp10<0?-temp10:temp10;
            if(temp10<0){b[4]=0x40;b[5]=digits[a/100%10];}
            else {b[4]=a>=1000?digits[a/1000]:0;b[5]=digits[a/100%10];}
            b[6]=digits[a/10%10]|0x80;b[7]=digits[a%10];
        }
    }
#endif
    EA=0;for(i=0;i<8;i++)screen[i]=b[i];EA=1;
}
#if IMPROVED
static u8 reset_ds(void)
{
    u8 present;
    DQ=0;wait_us(500);EA=0;DQ=1;wait_us(70);present=!DQ;EA=1;wait_us(430);
    return present && DQ;
}
static void write_ds(u8 b)
{
    u8 i;
    for(i=0;i<8;i++){
        EA=0;DQ=0;
        if(b&1){_nop_();_nop_();DQ=1;EA=1;wait_us(65);}
        else {wait_us(60);DQ=1;EA=1;wait_us(5);}
        b>>=1;
    }
}
static u8 read_ds(void)
{
    u8 i,b=0;
    for(i=0;i<8;i++){
        EA=0;DQ=0;_nop_();_nop_();DQ=1;wait_us(5);
        if(DQ)b|=(1<<i);EA=1;wait_us(60);
    }
    return b;
}
static u8 start_temp(void) {if(!reset_ds())return 0;write_ds(0xcc);write_ds(0x44);return 1;}
static u8 get_temp(void)
{
    u8 b[9],i,j,crc=0,m;int raw;
    if(!reset_ds())return 0;
    write_ds(0xcc);write_ds(0xbe);
    for(i=0;i<9;i++)b[i]=read_ds();
    crc=0;
    for(i=0;i<9;i++){m=b[i];for(j=0;j<8;j++){u8 mix;mix=(crc^m)&1;crc>>=1;if(mix)crc^=0x8c;m>>=1;}}
    if(crc)return 0;
    raw=(int)(((u16)b[1]<<8)|b[0]);if(raw < -880 || raw>2000)return 0;
    temp10=(int)((long)raw*10/16);return 1;
}
#endif
static void measure(void)
{
    u16 s,n;u32 v,d;
    valid=0;
    if(ECHO)return;
    TRIG=1;wait_us(15);TRIG=0;
    s=timer1();while(!ECHO)if((u16)(timer1()-s)>27648U)return;
    s=timer1();while(ECHO)if((u16)(timer1()-s)>27648U)return;
    n=timer1()-s;
    v=(IMPROVED && temp_valid)?(u32)(33140L+6L*temp10):34000UL;
    /* d in 0.1 cm, f_timer=921600Hz, v in cm/s. No intermediate overflow. */
    d=((u32)n*v+92160UL)/184320UL;
    if(d<20 || d>999)return;
    distance=(u16)d;valid=1;
}
void main(void)
{
    u16 last_measure,last_temp,t;u8 converting=0;
    EA=0;P0=0;P1=0xff;TRIG=0;KEY=1;DQ=1;
    P2=0xc0;BEEP=0;SCON=0;ES=0;
    TMOD=0x11;TH1=TL1=0;TR1=1;ET1=0;
    TH0=0xfc;TL0=0x66;ET0=1;TR0=1;EA=1;
    shown_valid=1;shown=0;render();
    last_measure=now();last_temp=now();
#if IMPROVED
    converting=start_temp();
#endif
    while(1){
        t=now();
        if(key_event){EA=0;key_event=0;EA=1;locked=!locked;render();}
        if((u16)(t-last_measure)>=80){
            last_measure=t;measure();alarm=valid && distance>0 && distance<100;
            if(!locked){shown=distance;shown_valid=valid;}render();
        }
#if IMPROVED
        if((u16)(t-last_temp)>=1000){
            temp_valid=converting?get_temp():0;
            converting=start_temp();last_temp=now();render();
        }
#endif
    }
}
