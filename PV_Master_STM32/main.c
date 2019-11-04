//stm32f103, 72 Mhz,external quartz +PLLx9

char kk,p;
char readbuff[64],writebuff[64];

void InitADC();
void InitTimer3();
unsigned Vop();
void readJT();
unsigned int VRead();

unsigned char ROWS=32;
unsigned int J[4]={900,900,900,900};
unsigned char T[4]={25,25,25,25};
unsigned int v1,v2;
unsigned int vp;

char val1[8],val2[4];
char index[4]={'0','1','2','3'};
unsigned char lowB,highB;
char error;

void Setup()
{
      
  UART1_Init_Advanced(19200, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART1_PA9_10);
  error = Soft_UART_Init(&GPIOB_ODR, 11, 10,19200, 1);
 
  InitTimer3();
  InitADC();
  HID_Enable(&readbuff,&writebuff); // enable usb interface                        
  Delay_ms(200);                    
  UART1_Write_Text("Setup \r\n"); 
}

//critical point update
void Timer3_interrupt() iv IVT_INT_TIM3
{
  TIM3_SR.UIF = 0;       
  vp=VRead();  
}

void main()
{

  Setup(); 
  while(1) 
  {        
      ROWS=Vop();     
      USB_Polling_Proc();
      kk = HID_Read();
      if(kk != 0)
      {
      if((char)readbuff[0]=='O'){  
                           
          //send number of panels according to VOC
          writebuff[0]=ROWS;
          HID_Write(writebuff,64);
          
      }else if((char)readbuff[0]=='L'){
           //send critical point 
          lowB=vp&0xFF;
          highB=vp>>8;
          writebuff[0]=highB;
          writebuff[1]=lowB;
          HID_Write(writebuff,64);
        
        //read temperature and irrandiances from Coniguration and send them to the 4 slaves 
      }else readJT();                       
     }              
  }
}

// read temperature & irradiance from the 4 pv_slaves
void readJT(){
char i=0,j=0;
for(j=0;j<4;j++){

lowB=readbuff[i++]&0xFF;
highB=readbuff[i++];
J[j]= lowB | highB<< 8;
T[j]=readbuff[i++];
}

for(j=0;j<4;j++){

lowB=J[j]&0xFF;
highB=J[j];

//send to 4 slaves through soft uart
Soft_UART_Write(index[j]);
Soft_UART_Write(lowB);
Soft_UART_Write(highB);
Soft_UART_Write(T[j]);
}
}

//Optimized Read voltage function (average of 950 ADC readings)
 unsigned int VRead(){
 double avg1=0,avg2=0;
 unsigned i=0;
 unsigned int v=0;
 
 while(i<950){
 avg1+=(double)ADC1_Read(2);
 avg2+=(double)ADC1_Read(3);
 i++;
 }
 v1=(int)(avg1/950);
 v2=(int)(avg2/950);
 v=(unsigned)(v1-v2);  
 
 //Calliborate offset 
 v+=8;
 v=abs(v);     
 return v;
}

// calculate VOC and returns number of panels
unsigned Vop(){

      double vc=0;
      unsigned count=0;   
     //Gain
     vc=(double)(vp*3.3);
     vc/=4095;
     vc*=126.4;
         
     if(vc<=36)count=1;
     else if(vc>36 && vc<=72)count=2;
     else if(vc>72 && vc<=108)count=3;
     else if(vc>108)count=4;
     /*
     sprintf(val1, "%7f", vc);             
     UART1_Write_Text("Vp= :");       
     UART1_Write_Text(val1);             
     UART1_Write_Text("\r\n");
     UART1_Write_Text("Rows = ");
     sprintf(val1,"%d",count);
     UART1_Write_Text(val1);
     UART1_Write_Text("\r\n");
     */     
     return count*36;
}

void InitADC(){

  ADC_Set_Input_Channel(_ADC_CHANNEL_0);
  ADC_Set_Input_Channel(_ADC_CHANNEL_1);
  ADC_Set_Input_Channel(_ADC_CHANNEL_2);
  ADC_Set_Input_Channel(_ADC_CHANNEL_3);
  ADC1_Init();
}

void InitTimer3(){

 RCC_APB1ENR.TIM3EN = 1;       // Enable clock gating for timer module 2
 TIM3_CR1.CEN = 0;             // Disable timer
 // 10 hz == one sec
 TIM3_PSC = 109;              // Set timer prescaler.
 TIM3_ARR = 65514;
 NVIC_IntEnable(IVT_INT_TIM3); // Enable timer interrupt
 TIM3_DIER.UIE = 1;            // Update interrupt enable
 TIM3_CR1.CEN = 1;             // Enable timer
}