#include <bios.h>
#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <string.h>
#include <dir.h>
#include <math.h>

#include "key.h"

extern "C" void interrupt int8Entry(...);
extern "C" void interrupt dummyIRQ12(...);

union REGS reg;

#define byte unsigned char
#define word unsigned int

#define ddRam  0x80
#define cgRam  0x40
#define wr  4
#define wc  0
#define we  8
#define funSet47   0x28   // 4 bit  2 line  5x7
#define dispOn     0xc
#define dispOff    0x8
#define dispClear  0x1
#define modeSetInc 0x6
#define modeSetShf 0x5

#define J8  0xA0
#define J9  0xC0
#define RgnOut J9+1  //c1
#define RgnChk J9+2
#define AlPort J9+3

#define sizeArr    2048
#define fmin       244
#define DAC        0x8a

#include "convchar.h"

word tPause[8],tWork[8],cPause[8],cWork[8],cErr[8],rgnStatus,wrkStatus;
byte rgnWork,rgnError,rgnEnable,rgnImage,StartRgn,StopRgn,newMin,cntRes;
word bTimePause[8] = {60,80,100,7,18,16,22,14}; // Min
word bTimeWork[8]  = {128,332,225,3,4,6,8,2}; // Sec
int mhNum;
word rchk;

// Net
word Netqueue[32],headNet=0,tailNet=0,indexOsc=0,indOsc=0,RsStatus=0x2fd;
byte netTakt=0,fAdr=1,cntPN=0,typFile=0,nNext=64,flagP=0, flDAC=0;
byte sizeFile=5,identyf=1,cnw=0;
byte  inData[33],chkSumm,senData[33],shSendSm;

byte uArr[sizeArr],iArr[sizeArr],uSpr[32],Umax[16],Umin[16],Usr[16],Isr[16],Imax[16];
byte netOsc[4096], tArr[2048], fArr[2048];
word Ikv[16], Inn, In_1, Ikvd, Unn, Un_1;
byte Iogr,ImOgr,Idogr,Uspn,uSpark,uSp,Uogr,UmOgr;
byte iInf,uInf,Ikz,Ukz,Uxx,Ixx,iAvar,uOld;

word countF=0,countOn=0,takt=0,nSpark,netSp;
byte iLast,uLast,uLim,lastSync,NumbSpark,dU,Ud;
word Phi,PhiLast,PhiOld,PhiHpp,PhiHp,PhiOk,fn_1,fTayOld;
word fi,fb,fd,fv,fmax,fmxr,fvLast,fm,fg,fkm,fkb,fsk,fOff,ffhnd,fvmax;
word cnt,ct,cn_c,index,index_p,index_s,index_b,nSum,nMin,Uind,Iind,tCom;
byte flr,fle,flp,fli,flb,flg,flk,flo,flv,fly;
byte flt,flh,fln,fls,flf,fkt,flc,fl10,fl20,ftm,flw,flphi,flmd;
byte coi,cob,cod,cov,c100,cPaus,nhp,chp,nhf,chf,cnSr,pause,cnpause,countStart;

byte cind,kt,up,ip,wComm,commB2,fok,fwo,Usk,Isk,hPhi,nIntegr;
byte cntId,cnFp,fP_1,oldTay,Umn1,Isr1,enableHpp,cnHi,cnHppOk,EnablePkt;
byte offAlim,flHpr,flDist,flfOk,flmOk,flTs,fldS,Tok,Inok,Tay,Tvc,Tsp;
word lastScan,cntScan,headKb,tailKb,cntRepeat;
word queueKb[32];
word iSec,lSec,dSec,rSec,cntSec,cntMin,cntMih,okMih,uIntegr,sysTimer,cnFSec;
word w1=0xffff,w2,w3,w4,w5,w6,w7,w8;
byte b1,b2,b3,b4,b5,b6,b7,b8; // for debug

char *password="DEF";
char pass[9];
char *sysPassword = "604038698";
char passIn[12], temp[88];

#include "menu.h"

int fMenu=0,item=0,kFun=0,pos=0,com, maxVal, minVal,cItem;
int* wVal;
byte * cVal, lastVal;
word tVal;
char ** lVal;

char *inBuf ="1_ ", *inRgn ="  :  ";

byte dtImage=2, fImage=255, pcImage=4, sp=32, usp1, usp2, flwr=0;

int pc=J8+3,pf=J9+1,dt=J8+1,pi=J9+3,mTail=0,mHead=0;        // Zero
word messQueue[128];
int i,j,f,n,tolk=0;
word l,k,t01,t02;
float M0,Lp,dEf,Vg,zIn,eta,Nf,np,T;
float *cPar[] = { &M0,&Lp,&dEf,&Vg,&zIn,&eta,&Nf,&np,&T };
byte uOsc[1024],iOsc[1024];

// ����������������������������* CODE *���������������������������������

int rdFile( char *file, char *buf, unsigned int n) {
  if ( (f=_open(file,O_RDONLY|O_DENYWRITE))== -1 ) return 0;
  _read(f,buf,n); _close(f);  return 1;
}

int wrFile( char *file, char *buf, unsigned int n) {
  if((f=open(file,O_CREAT|O_TRUNC|O_WRONLY|O_DENYREAD,S_IWRITE))==-1) return 0;
  _write(f,buf,n); _close(f);  return 1;
}

void wrFls() {
char fs[2];
  fs[0]=fs[1]=fls;
  // wrFile("fls.cfg",fs,2);
  flw=0;
}

void rdFls() {
char fs[2];
  fls=0;  // if noWrite
  //if (rdFile("fls.cfg",fs,2))  if(fs[0]==fs[1]) fls=fs[0];
}

void rdRgnCfg() {
  if (!rdFile("regen.cfg",(char *)tPause,32))
    for (j=0; j<8; j++) { tPause[j]=bTimePause[j]; tWork[j]=bTimeWork[j]; }
  for (j=0; j<4; j++) { cErr[j]=0; cPause[j]=tPause[j]; cWork[j]=tWork[j]; }
  rgnStatus=0; wrkStatus=0xffff;
  rgnWork=rgnError=StartRgn=StopRgn=rgnImage=0; rgnEnable=0xff;
//??????  system("d:");
  rdFile("regen.cnt",(char *)cPause,56);
//  system("c:");
}

void wRgnCnt() {
  //system("d:");
  wrFile("regen.cnt",(char *)cPause,56);
  //system("c:");
}

void wRgncfg() {
  if (!flwr) return ;
  if (!wrFile("regen.cfg",(char *)tPause,32)) kFun=905;
}

void wAlcfg() {
  temp[0] =Iogr;  temp[1] =Uogr;  temp[2] =ImOgr;  temp[3] =UmOgr;
  temp[4] =nhp;   temp[5] =nhf;   temp[6] =Ikz;    temp[7] =Ukz;
  temp[8] =Ixx;   temp[9] =Uxx;   temp[10]=pause;  temp[11]=EnablePkt;
  temp[12]=(byte)PhiHpp;          temp[13]=uLim;   temp[14]=fAdr;
  temp[15]=Tsp;   temp[16]=Inok;  temp[17]=uSp;    temp[18]=Tok;
  temp[19]=Tvc;   temp[20]=offAlim;                temp[21]=flHpr;
  temp[25]=flTs;  temp[26]=fldS;  temp[27]=flfOk;  temp[28]=flmOk;
  temp[29]=Ud;    temp[30]=iAvar; temp[31]=fmax;   temp[32]=fmxr;
  temp[33]=flDist;   temp[34]=ffhnd;
  for (j=0; j<9; j++) temp[48+j]=password[j];
  temp[60]=usp1;   temp[61]=usp2;
  if (!wrFile("alconfig.sys",temp,64)) kFun=905;
}


//#define RgnOut J9+1
//#define RgnChk J9+2
void regener() {
word m1, m2; //inportb(RgnChk);
byte onMh[4]={0x8,0x4,0x2,0x1},nr,mr;                // Mask's of output line
word lM[4][2]={{0x4,0},{0x8,0},{0x2,0},{0x1,0}};  // Link Out-In; 0-None
byte onV[4]={0,0,0,0}; // � ����� ������� ������� �����; 0-�� ������ (1248)
// 0-�ᠤ�⥫��,1-��p���p��騥,2-���p��p�,3-��p����p�
  if (!(iSec%60)) newMin=1; //!!!!!!!!!!!!!! 60
  for(j=0,mr=1; j < 4; j++) {
    mr=1<<j; nr=~mr; m1=lM[j][0]; //m2=lM[j][1];
    if (rgnEnable & mr) {                     // ? Enable
     if (rgnWork & mr) {                        // ? Working
      if (tWork[j]<999)                        // ? mehnOff(j);
       if (--cWork[j]) {                       // ? mehnOff(j);
         if (((rchk & m1)>0)&(++cErr[j] > 3)) rgnError |= mr<<j;   // mehnCheck 1-Error
       //  if (((rchk & m2)>0)&(++cErr[j] > 1)) rgnError |= mr<<j+1;  // mehnCheck 1-Error
       } else { rgnWork &= nr; rgnImage &= ~onMh[j];   // mehnOff(j);
           outportb(RgnOut,~rgnImage); cWork[j]=tWork[j]; cPause[j]=tPause[j];
         if (flHpr & (j<2)) { if(nhp) nhp--; flf=1; }
     } }
     else if (newMin) {                         // Pause
        if (!onV[j]) {     // ��堭����
          if (!(--cPause[j]))        // ? Start
           { rgnStatus=0;
           if (rgnStatus)  cPause[j]++; // wait to all off
             else {  rgnWork |= mr; rgnImage |= onMh[j];
              outportb(RgnOut,~rgnImage);           // mehnOn(j);
              rgnError &=~(mr<<j);  rgnError &=~(mr<<j+1);
              cErr[j]=0; cPause[j]=tPause[j]; cWork[j]=tWork[j];
              if (flHpr) nhp++;  }
           }
        } else  {          // �������
         if ( onV[j] & rgnWork)
           if (!(--cPause[j])) {  rgnWork |= mr;  rgnImage |= onMh[j];
             outportb(RgnOut,~rgnImage);           // mehnOn(j);
             rgnError &=~(mr<<j);  rgnError &=~(mr<<j+1);
             cErr[j]=0; cPause[j]=tPause[j]; cWork[j]=tWork[j]; }
        } //if(!onV[j])
    }} else { rgnWork &= nr; rgnImage &= ~onMh[j];   // mehnOff(j);
           outportb(RgnOut,~rgnImage); cWork[j]=tWork[j]; cPause[j]=tPause[j];
         if (flHpr & (j<2)) { if(nhp) nhp--; flf=1; }}
  } //   for
  if (newMin) /* wRgnCnt(); */ newMin=0;
    // w1=rchk;
}

void wrByte(byte b, byte c=wr) {
 _BX=_DX=(mHead+1)&127; _AL=b; _AH=c;
    asm  cli
  messQueue[_BX]=_AX; mHead=_DX;
    asm  sti
}

void outChar(byte r, byte c) { wrByte(r|ddRam,wc); wrByte(tCh[(int)c]); }

void display(int col, char* mess) {   // Indicator
  for( i=0; mess[i] !=0,i<16; i++ )  outChar((byte)(col+i),mess[i]);
}

/*
void display(int col, char* mess) {   // Vga text mode
void far *vp;
  if (col >= 64) col+=96;  // -64 + 2*80
  for( i=0; mess[i] !=0; i++ ) {
    vp=MK_FP(0xb890,col+i*2);    *((char*)vp)=mess[i];
    vp=MK_FP(0xb890,1+col+i*2);  *((char*)vp)=4;
} }*/

char ntoChar(int k) {
   if (k==Num1) return '1';
   if (k==Num2) return '2';
   if (k==Num3) return '3';
   if (k==Num4) return '4';
   if (k==Num5) return '5';
   if (k==Num6) return '6';
   if (k==Num7) return '7';
   if (k==Num8) return '8';
   if (k==Num9) return '9';
   if (k==Num0) return '0';
   if (k==Point) return '.';
   if (k==Ctrl1) return 1;
   if (k==Ctrl2) return 2;
   if (k==Ctrl3) return 3;
   if (k==Ctrl4) return 4;
   if (k==Ctrl0) return 10;
   if (k==CtrlBs) return 11;
 return '?';
}

void makeMenu() {
  display(64,FullMenu[fMenu][item]);
}

void storeSet() {
  switch(com) {
   case 204 : offAlim=tolk;  break;
   case 205 : flHpr=tolk;    break;
   case 400 : flTs=tolk;     break;
   case 402 : fldS=tolk;     break;
   case 403 : flfOk=tolk;    break;
   case 406 : flmOk=tolk;    break;
   case 108 : flDist=tolk;   break;
}  }

char * findRegime() {
 char *Regime[8]={"�.�.","�.�.","���1","���2","���3","1���","�⪫","����"};
  if (flr&0x30) sp='*'; else sp=' ';
  for (byte fe=fle,j=0; fe; j++)  fe>>=1; if(j) return Regime[j-1];
  if (fls & 0xc0) return "D���";   // wdstttoa
  if (fls & 4)    return "���";   // wdstttoa
  if (fls & 3)    return "����";   // wdstttoa
  if (fls)        return "�⮯";   // wdstttoa
  if (wComm==4)   return "���.";
  if (wComm==5)   return "��� ";
  if (flr==2)     return "���";
  if (flr&4)      return "���.";
  if (flr&8)      return "�.�.";
  if (flr==0x42)  return "����";
  if (flr==0x40)  return "���I";
  if (flr==0x82)  return "��U�";
  if (flr==0x80)  return "���U";
  if (flr==0xC0)  return "���";
  return "���" ;
}

void scrShow() {    countStart=0; //for Dist
  sprintf(temp,"%s U=%3d%cI=%3d",findRegime(),uInf,sp,iInf ); display(0,temp); //Allwais
  if(!kFun) {   sprintf(temp,"��  1  2  3  4  ");
    for(j=0; j < 4; j++) {
      if (rgnWork & (1<<j)) temp[j*3+4]=0xf7;                     ;  // ? Work
      if (rgnError & (1<<j*2)) temp[j*3+5]='x';                     // ? Error
      if (rgnError & (1<<j*2+1)) temp[j*3+6]='x';                   // ? Error
  } }
   //sprintf(temp,"%3d% 3d%2d%2d %2d%2x",Phi,uOld,fd,cnFSec,Tsp,Tay);
   //sprintf(temp,"%3x %2x%4x %2x %2x",Phi,fd,cnFSec,Tsp,Tay);  //debug1
   //sprintf(temp,"%3x %2x%4x %2x %2x",Phi,fd,fv,flp,Tay);  //debug1
   //sprintf(temp,"%2x%4x %4x %4x",b4,w1,w2,w3);  //debug
  //sprintf(temp,"%3x %2x%4x %2x %2x",Phi,fd,fv,w2,w1);  //debug1
  if(kFun==1) return;
  if(kFun==2) sprintf(temp,"%s = %3d     ",Icon[fMenu][item],*cVal); // Change Value
  if(kFun==3) sprintf(temp,"%s = %s      ",Icon[fMenu][item],inBuf); // Input Value
  if(kFun==6) sprintf(temp,"%s %s",Icon[fMenu][item],lVal[tolk]); // Change lValue
  if(kFun==200) { sprintf(temp,"���.1  2  3  4  ");
    for(j=0; j < 4; j++) {
      if (rgnWork & (1<<j)) temp[j*3+4]=0xf7;                     ;  // ? Work
      if (rgnError & (1<<j*2)) temp[j*3+5]='x';                     // ? Error
      if (rgnError & (1<<j*2+1)) temp[j*3+6]='x';                   // ? Error
  } }
  if(kFun==200) {j=mhNum; byte m= 1<<j; char ch=':';
    if (!(rgnWork & m) & (lSec%3)) ch=' ';            // ? Working
    sprintf(temp,"M%1d �=%2d%c%02d �=%03d",j+1,cPause[j]/60,ch,cPause[j]%60,cWork[j]); }
  if(kFun==201) { j=mhNum;
    sprintf(temp,"T_����_%1d%4d:%02d",j+1,tPause[j]/60,tPause[j]%60); }
  if(kFun==221) sprintf(temp,"T_����_%1d  %s",mhNum+1,inRgn);
  if(kFun==202) { j=mhNum;
    sprintf(temp,"T_ࠡ���_%1d  %3d ",j+1,tWork[j]); }
  if(kFun==222) sprintf(temp,"T_ࠡ���_%1d  %s ",mhNum+1,inRgn);
  if(kFun==203) if (cItem < 8) sprintf(temp,"%s %4f",rPar[cItem],*cPar[cItem]);
                 else sprintf(temp,"T_���� = %5f",
       1.67*M0*Lp/(dEf*Vg*zIn*(1-pow(1-eta,1/Nf))*(pow(1-eta,(np-1)/Nf))));
  if(kFun==223) sprintf(temp,"%s %s",rPar[cItem],inRgn);
  if(kFun==300) sprintf(temp,"��஫� %s",passIn);  // in Password
  if(kFun==301) sprintf(temp,"������ ? Enter/*");
  if(kFun==500) sprintf(temp,"%3x %2x%4x %2x %2x",Phi,fd,fv,uSpark,Tay);
  //if(kFun==500) sprintf(temp,"%04x %04x %02x%04x",dSec,cntMih,uSpark,w1);
  if(kFun==501) sprintf(temp,"%4x  %4x %4x ",rgnWork,cPause[1],rchk);
  if(kFun==502) sprintf(temp,"%2x %2x% 2x %2x  %4x",inData[0],inData[1],inData[21],cnw,Phi);
  if(kFun==503) sprintf(temp,"%3x %2x%4x %2x %2x",Phi,fd,cnFSec,Tsp,Tay);
  if(kFun==504) sprintf(temp,"%3x %2x%4x %2x %2x",Phi,fd,nSpark,fln,Tay);
  if(kFun==505) sprintf(temp,"In %3d%3d %3d%3d",Isr[0],Isr[1],Isr[2],Isr[3]);
  if(kFun==905) sprintf(temp,"s","Write Error");

  if (iSec<2)
    if (!rdFile("alconfig.sys",temp,1))   {
        if (iSec) wAlcfg();
        sprintf(temp,"� �.���� �����!");   display(0,temp);
        sprintf(temp,"�ਢ�� �� ���� !");
    } else sprintf(temp,"������� ��.���");
  display(64,temp);
}

void doCommand() {
  com = 100*fMenu+item;  kFun=2;
  switch(com) {
   case 100 : cVal=&Iogr; maxVal=120; minVal=5;           break;
   case 101 : cVal=&Uogr; maxVal=90; minVal=25;           break;
   case 102 : cVal=&UmOgr; maxVal=200; minVal=50;         break;
   case 103 : cVal=&(byte)fmax; maxVal=200; minVal=70;    break;
   case 104 : cVal=&hPhi; maxVal=128; minVal=15; hPhi=15; break;
   case 105 : cVal=&nhp; maxVal=100; minVal=0;     break;
   case 106 : cVal=&nhf; maxVal=90; minVal=1;     break;
   case 107 : cVal=&(byte)PhiHpp; maxVal=250; minVal=150; break;
   case 108 : lVal=Have; kFun=6; tolk=flDist;     break;
   case 200: case 201: case 202: case 203:      kFun=com; break;
   case 204 : lVal=yesNo; kFun=6; tolk=offAlim;   break;
   case 205 : lVal=yesNo; kFun=6; tolk=flHpr;     break;
   case 300 : sprintf(passIn,"         "); pos=0; kFun=com; break; // change password
   case 301 : kFun=com;                           break;  // ������
   case 302 : cVal=&fAdr; maxVal=14; minVal=1;    break;
   case 303 : cVal=&uSp; maxVal=60; minVal=0;     break;
   case 304 : cVal=&Ukz; maxVal=60; minVal=0;     break;
   case 305 : cVal=&Ikz; maxVal=90; minVal=0;     break;
   case 306 : cVal=&Uxx; maxVal=110; minVal=4;     break;
   case 307 : cVal=&Ixx; maxVal=60; minVal=1;     break;
   case 308 : cVal=&pause; maxVal=20; minVal=0;   break;
   case 400 : lVal=opTay; kFun=6;  tolk=flTs;     break;
   case 401 : if (flTs) cVal=&Tsp; else cVal=&Tvc;
                 maxVal=250; minVal=1;            break;
   case 402 : lVal=dubleSp; kFun=6; tolk=fldS;    break;
   case 403 : lVal=findOK; kFun=6; tolk=flfOk;        break;
   case 404 : cVal=&Inok; maxVal=250; minVal=0;   break;
   case 405 : cVal=&Tok; maxVal=255; minVal=1;    break;
   case 406 : lVal=metOK; kFun=6;  tolk=flmOk;        break;
   case 407 : cVal=&Ud; maxVal=200; minVal=1;    break;
   case 500 : case 501: case 502: case 505:
              case 503: case 504:        kFun=com; break;
   case 506 : cVal=&iAvar; maxVal=250; minVal=1;   break;
   case 507 : cVal=&(byte)ffhnd; maxVal=140; minVal=0;    break;
  }  if(cVal) lastVal = *cVal;
}

int chkPassword() {
 for(k=0,j=0; password[j]!=0; j++) if(passIn[j] !=password[j]) { k=1; break; }
 if (k) for(j=0; j<9; j++) if(passIn[j] !=sysPassword[j]+16) return 0;
return 1;
}

void exit_() {
  asm { cli
        mov     ax,10
        mov     dx,0xff62
        out     dx,al
        mov     ax,0xd6d8
        mov     dx,0xff52
        out     dx,al
      }
  setvect(0x08,getvect(0x60));
  asm sti
}

void init_() {
  asm { cli
        mov     ax,2
        mov     dx,0xff62
        out     dx,al
        mov     ax,0x188/2
        mov     dx,0xff52
        out     dx,al }
     //outportb(0xff28,0xfffe);
  setvect(0x60,getvect(0x08));    // Set Int 60 to Int 8
  setvect(0x8,int8Entry);
 // UNIO48  ॣ����� ��室�� � 1
  outportb(J8,0);  outport (J8+1,-1);  outportb(J8+3,-1);
  outportb(J9,0);  outport (J9+1,-1);  outportb(J9+3,-1);
  outportb (J8,1);
        outportb(J8+1,-1);      // ��室�
        outport(J8+2,0);        // �室�
  outportb (J9,1);
        outport(J9+1,0x00ff);   // 1-��室�/2-�室�
        outportb(J9+3,0x0e);    // �室�/��室�
  outportb(J8,0);  outportb(J9,0);

  _AX=0xb3; _DX=1;  asm int 14h
  atexit(exit_);
  asm sti
}

void main(void) {  // �������������������������������������������������������
 wrByte(0x20,wc);   wrByte(funSet47,wc); wrByte(modeSetInc,wc);
 wrByte(dispOn,wc); wrByte(dispClear,wc);  // ! Pause 1,8 mS
Uspn=50; uSpark=40;              // Init public Data
Phi=PhiLast=PhiOld=PhiHpp=PhiHp=fn_1=fTayOld=fmin;
fi=fb=fd=6; ffhnd=fv=20; countStart=0;
fm=50; fg=150; fkm=-100; fkb=75;
flr=fle=fli=flb=flg=flk=flo=flv=fls=fl10=fl20=ftm=0; flp=0xff; EnablePkt=0;
cnt=-1; index=index_p=index_s=index_b=0; cnFp=50; cn_c=16; cntId=12;
netSp=nSpark=0; cPaus=4; com=hPhi=0; flf=1; Tay=2; ct=1; nIntegr=0; uIntegr=0;
cntSec=6340; lSec=3; rSec=dSec=cntMin=cntMih=0; fln=1; nMin=376;
lastSync=0; lastScan=cntScan=headKb=tailKb=0;  mhNum=0; newMin=0;

long sleepCount=9000000;

// BLINK
      outportb (0x87,1);        // ����
      delay(20);
      outportb (0x87,3);        // 3- off; 2-������

 if (rdFile("alconfig.sys",temp,64)) {
    Iogr=temp[0];   Uogr=temp[1];   ImOgr=temp[2];       UmOgr=temp[3];
    nhp =temp[4];   nhf =temp[5];   Ikz  =temp[6];       Ukz  =temp[7];
    Ixx =temp[8];   Uxx =temp[9];   pause=temp[10];  EnablePkt=temp[11];
    PhiHpp =temp[12]; PhiHpp &=255;  uLim=temp[13];       fAdr=temp[14];
    Tsp=temp[15];   Inok=temp[16];    uSp=temp[17];        Tok=temp[18];
    Tvc=temp[19];  offAlim=temp[20];                     flHpr=temp[21];
    flTs=temp[25]; fldS=temp[26];   flfOk=temp[27];      flmOk=temp[28];
    Ud=temp[29];  iAvar=temp[30];    fmax=temp[31];       fmxr=temp[32];
    flDist=temp[33];  ffhnd=temp[34];
    for (j=0; j<9; j++) password[j]=temp[48+j];
    usp1=temp[60]; usp2=temp[61];
 } else {
    Iogr=100;     Uogr=66;     ImOgr=200;     UmOgr =210;
    nhp = 0;       nhf=2;      Ikz  =10;        Ukz =5;
    Ixx = 3;       Uxx=98;     pause=0;    EnablePkt=0;
    uLim=254;   PhiHpp=fmin;    fAdr=1;         Inok=0;
    uSp =0;        Tok=250;      Tvc=8;          Tsp=12;
    flTs=2;       fldS=1;      flfOk=0;   flmOk=0;  flHpr = 0;
    iAvar=100; fmax=fmxr=80;  flDist=1      ;  ffhnd=0;
    usp1=160;     usp2 = 160; Ud=200;
 }

 rdRgnCfg(); rdFls();  cntRes++;

 init_();

 M0=1.1; Lp=3.0; dEf=0.137; Vg=1.5; zIn=10.0; eta=0.98; Nf=4; np=fAdr;
 for (j=0; j<4000; j++) ;  // pause for Indicator
 b1=b2=0;

while(k != AltX) {
 //   asm mov al,0ffh  // last debug
 //   asm out 0c2h,al
 //   printf("%1x-%4x  ",fl10,index);
 //   printf("%4x-%4x|",inport(0xff52),inport(0xff50));
 //printf("%4x-%4x-%4x|",inport(0xff50),inport(0xff58),inport(0xff60));
  if(!--sleepCount) kFun=fMenu=item=0; // �᫨ ����� �� ���� �� ������
  if(tailKb!=headKb/2) { tailKb=(tailKb+1)&31; k=queueKb[tailKb]; sleepCount=90000000;
    switch(k) {
      case Ctrl1: case Ctrl2:  case Ctrl3:  case Ctrl4:  j=ntoChar(k)-1;
      if(rgnWork & 1<<j) cWork[j]=1; else { cPause[j]=1; newMin=1; }
      break;
      case Ctrl0:   rgnEnable=0xff; break;
      case CtrlBs:  rgnEnable=0; break;
    } /* switch k */
   switch(kFun) {
   case 0 :            // input password
    switch(k) {
     case Enter :
      if(chkPassword()) {passIn[0]=kFun=1; item = fMenu=0; makeMenu();}
     case Esc :  pos=0;
        break;
     case Num1: case Num2: case Num3: case Num4: case Num5:
     case Num6: case Num7: case Num8: case Num9: case Num0:
      if (pos < 9) passIn[pos++]=16+ntoChar(k);
        break;
    } /* switch k 0*/
    break;
   case 1 :            // Menu
    switch(k) {
     case Enter : if(fMenu) doCommand(); else { fMenu=item+1; item=0; makeMenu();}
        break;
     case Esc : pos=0; if(fMenu) { item=fMenu-1; fMenu=0; makeMenu();
              wRgncfg(); }  else kFun=0;
        break;
     case GrMinus : if (--item < 0) item=sizeMenu[fMenu]; makeMenu();
        break;
     case GrPlus :  if (++item > sizeMenu[fMenu]) item=0; makeMenu();
        break;
     case Num1: case Num2: case Num3: case Num4: case Num5:
     case Num6: case Num7: case Num8: case Num9:
      if ((item=ntoChar(k)-49) > sizeMenu[fMenu]) item=sizeMenu[fMenu]; makeMenu();
        break;
    } /* switch k 1*/
    break;
   case 2 :            // modify byte
    switch(k) {
     case Enter :  case Esc :  kFun=1; makeMenu(); hPhi=0; flf=1;
        break;
     case GrMinus :  if( *cVal > minVal)  -- *cVal;
        break;
     case GrPlus :   if( *cVal < maxVal)  ++ *cVal;
        break;
     case Num1: case Num2: case Num3: case Num4: case Num5:
     case Num6: case Num7: case Num8: case Num9: case Num0:
      kFun=3; inBuf[0]=ntoChar(k); inBuf[1]='_'; inBuf[2]=' '; pos=1;
        break;
    } /* switch k 2*/
    break;
   case 3 :                         // input Byte Value
    switch(k) {
     case Enter :  inBuf[pos]=0; tVal = atoi(inBuf);       // end input
      if (tVal > maxVal) tVal=maxVal; if (tVal < minVal) tVal=minVal;
         *cVal=(byte)tVal;
     case Esc :    kFun=2;
        break;
     case BackSp : if(pos) {if (pos<3) inBuf[pos]=' '; inBuf[--pos]='_';}
        break;
     case Num1: case Num2: case Num3: case Num4: case Num5:
     case Num6: case Num7: case Num8: case Num9: case Num0:
      if (pos<3) { inBuf[pos++]=ntoChar(k); if (pos<3) inBuf[pos]='_'; }
        break;
    } /* switch k 3*/
    break;
   case 5 :            // modifiy Word
    switch(k) {
     case Enter :  case Esc :   kFun=1; makeMenu();
        break;
     case GrMinus : if(-- *wVal < minVal)  *wVal=minVal;
        break;
     case GrPlus :  if(++ *wVal > maxVal)  *wVal=maxVal;
        break;
    } /* switch k 5*/
    break;
   case 6 :            // Option
    switch(k) {
     case Enter : storeSet();
     case Esc :   kFun=1; makeMenu();
        break;
     case GrMinus : if (--tolk<0) { while (lVal[++tolk][0]!=0); tolk--;}
        break;
     case GrPlus :  if (lVal[++tolk][0]==0) tolk=0;
        break;
    } /* switch k 6*/
    break;
    break;
    case 200:         // Time Pause & Work  Show
    switch(k) {
     case Esc :  case Enter :  kFun=1; fMenu=2; makeMenu();
        break;
     case GrMinus :  if(-- mhNum < 0)  mhNum=3;
        break;
     case GrPlus :   if(++ mhNum > 3)  mhNum=0;
        break;
     case Num1: case Num2: case Num3: case Num4:  mhNum=ntoChar(k)-49;
        break;
    } /* switch k 200*/
    break;
    case 201:            // RGN_num_mehn _ Pause
    switch(k) {
     case Esc :   kFun=1; fMenu=2;  makeMenu();
        break;
     case GrMinus :  if(-- mhNum < 0)  mhNum=3;
        break;
     case GrPlus :   if(++ mhNum > 3)  mhNum=0;
        break;
     case Num1: case Num2: case Num3: case Num4: case Num5:
     case Num6: case Num7: case Num8: case Num9: case Num0:
       strcpy(inRgn,"  :  ");
       inRgn[0]=ntoChar(k); inRgn[1]='_'; pos=1; kFun=221;
        break;
    } /* switch k 201*/
    break;
   case 221 :            // input rgn value
    switch(k) {
     case Enter :  inRgn[2]=0;  flwr=1;               // end input
       tVal =(word)(atoi(inRgn))*60 + atoi(inRgn+3); tPause[mhNum]=tVal;
     case Esc :   kFun=201;
        break;
     case BackSp :
      if(pos) {   if(pos<5) inRgn[pos] = ' ';
       if(pos==3) pos--; inRgn[--pos] = '_'; }
        break;
     case Num1: case Num2: case Num3: case Num4: case Num5:
     case Num6: case Num7: case Num8: case Num9: case Num0:
      if(pos<5) { inRgn[pos++]=ntoChar(k);
       if(pos==2) pos++; if(pos<5) inRgn[pos]='_'; }
        break;
    } /* switch k 221*/
    break;
   case 202:            // RGN_num_mehn _ Work
    switch(k) {
     case Esc :  kFun=1; fMenu=2;  makeMenu();
        break;
     case GrMinus : if(-- mhNum < 0)  mhNum=3;
        break;
     case GrPlus :  if(++ mhNum > 3)  mhNum=0;
        break;
     case Num1: case Num2: case Num3: case Num4: case Num5:
     case Num6: case Num7: case Num8: case Num9: case Num0:
       kFun=222; strcpy(inRgn,"   ");
       inRgn[0]=ntoChar(k); inRgn[1]='_'; pos=1;
        break;
    } /* switch k 202*/
    break;
   case 222:            // input rgn value
    switch(k) {
     case Enter :  inRgn[3]=0;  flwr=1;               // end input
      tVal =(word)(atoi(inRgn)); tWork[mhNum]=tVal;
     case Esc :    kFun=202;
        break;
     case BackSp : if(pos) {   if(pos<3) inRgn[pos] = ' ';
          inRgn[--pos] = '_'; }
        break;
     case Num1: case Num2: case Num3: case Num4: case Num5:
     case Num6: case Num7: case Num8: case Num9: case Num0:
      if(pos<3) { inRgn[pos++]=ntoChar(k);
         if(pos<3) inRgn[pos]='_'; }
        break;
    } /* switch k 222*/
    break;
   case 203:            // Calc_RGN_Time
    switch(k) {
     case Esc :  kFun=1; fMenu=2; makeMenu();
        break;
     case Enter :  cItem=8;
        break;
     case GrMinus : if(-- cItem < 0)  cItem=8;
        break;
     case GrPlus :  if(++ cItem > 8)  cItem=0;
        break;
     case Num1: case Num2: case Num3: case Num4: case Num5:
     case Num6: case Num7: case Num8: case Num9: case Num0:
       if( cItem < 8) { kFun=223;   strcpy(inRgn,"     ");
         inRgn[0]=ntoChar(k); inRgn[1]='_'; pos=1; }
        break;
    } /* switch k 203*/
    break;
   case 223:            // input Rgn Calc value
    switch(k) {
     case Enter :  inRgn[5]=0; *cPar[cItem++]=(atof(inRgn));                                         // for next
     case Esc :    kFun=203;
        break;
     case BackSp : if(pos) { if(pos<6) inRgn[pos] = ' ';
          inRgn[--pos] = '_'; }
        break;
     case Num1: case Num2: case Num3: case Num4: case Num5:
     case Num6: case Num7: case Num8: case Num9: case Num0: case Point:
      if(pos<5) { inRgn[pos++]=ntoChar(k); if(pos<4) inRgn[pos]='_'; }
        break;
    } /* switch k 223*/
    break;
   case 300 :          // change password
    switch(k) {
     case Enter : for(j=0; j < pos; j++) password[j]=passIn[j];
                  passIn[0]=1; password[pos]=0; kFun=0;
        break;
     case Num1: case Num2: case Num3: case Num4: case Num5:
     case Num6: case Num7: case Num8: case Num9: case Num0:
                  if (pos < 6) passIn[pos++]=16+ntoChar(k);
        break;
    } /* switch k 300*/
    break;
   case 301 :        // Save AlConfig
    switch(k) {
     case Enter :  wAlcfg();
     case Esc :    kFun=1;  fMenu=3; item=1; makeMenu();
        break;
    } /* switch k 301 */
    break;
   case 302 : case 303 : case 304 : case 305 : case 306 : case 307 : case 308 :
    switch(k) {
     case Esc :    kFun=1;  fMenu=3; item=1; makeMenu();
        break;
    } /* switch k 302-8 */
   case 500 : case 501 : case 502 : case 503 : case 504 : case 505 :
    switch(k) {
     //case Esc :   item=com-500; kFun=1; fMenu=5; makeMenu();
     case Esc :   kFun=1; makeMenu();
        break;
    } /* switch k 500 */
    break;
   case 905 :
    switch(k) {
     case Esc :    kFun=1;
        break;
    } /* switch k 905 */
    break;
   } /* switch  kFun*/
   scrShow(); // refresg screen if pressed key
  } /* if (k) */
  if ((dSec>>1)!=lSec) { lSec=dSec>>1;  scrShow(); if (flw) wrFls(); }
  if (iSec!=rSec) {  rSec=iSec;   regener();  }
 } /* while  */
 sprintf(temp,"��� ����� 3.3.8");    display(0,temp);
 sprintf(temp,"Remote Concole |");    display(64,temp);
  while(mTail!=mHead);
 exit(0);
} /* main */
