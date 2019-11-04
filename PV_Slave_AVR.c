// changefrom 1 to 4 depending on Slave
const char INDEX='0';

void Setup();
void writeDAC(unsigned dac,unsigned char channel); 
double CurrentFunction(double voltage,double j,double t);
double vFunction(double current,double j,double t);
void getJT();

char current_duty=127;
unsigned int adc_rd,pot;
float d=0;
char lowB,highB,test='v';
char output[20];
char txt[15];
char i=0,j=0;

int irr=900;
unsigned short temp=25;
const float VOC=36;
const float ISC=1.5;
float IREF=1.5,VREF=20;
float dac=1;

char b=0;
void sendUart();

void usart_interrupt() org IVT_ADDR_USART__RXC
{       
        
        output[b++]=(char)UART_Read();      
        if(b==12){
         b=0;
         //UART_Write_Text("uart:");
        // UART_Write_Text(output);
        }
      
}

void main() {

Setup();

while(1){
        
        
           //read Voltage and multiply by gain
           VREF=ADC_Read(3);
           VREF*=VOC;
           VREF*=1.415;
           VREF/=1023; 
           FloatToStr(VREF, txt);
           UART1_Write_Text("Vref :");
           UART1_Write('\n');
           UART1_Write_Text(txt);
           UART1_Write('\n'); 
                      
           //get irr and temp from serial
           getJT();         
           IREF=CurrentFunction(VREF,irr,temp); 
           
            //IREF=ADC_Read(2);
            //IREF*=ISC;
            //IREF/=1023;
             FloatToStr(IREF, txt);
             UART1_Write_Text("Iref :");
             UART1_Write('\n');
             UART1_Write_Text(txt);
             UART1_Write('\n'); 
			
			 //2800 represents max current 	
			 dac=IREF/ISC;
			 dac*=2800;
           
            //write Iref  , channel 1
            writeDAC((unsigned)dac,1);
           //write max voltage == VOC   , channel 0
            writeDAC(4092,0);
          
       
		 /*
		 Test voltage, should find Half VOC
         writeDAC(2046,0);
         writeDAC(4092,1);
		 */
		 
		 /*
         Test current
         writeDAC(1000,1);
         writeDAC(4092,0);
         */    
       }
}

void Setup(){

//GPIOs Setup
DDC0_bit = 0;
DDC1_bit = 1;
DDB0_bit = 1;
DDB1_bit = 1;
DDB2_bit = 1;

PORTB0_bit=0;
PORTB2_bit=1;
PORTC0_bit=0;
current_duty=30;

ADC_Init();
UART1_Init_Advanced(19200, _UART_NOPARITY, _UART_ONE_STOPBIT);

SREG_I_bit = 1;                    // enable global interrupt
RXCIE_bit=1;                   // enable interrupt on UARTs receive
            
SPI1_Init_Advanced(_SPI_MASTER,_SPI_FCY_DIV32,_SPI_CLK_LO_LEADING);
Delay_ms(200);
UART1_Write_Text("Setup ");
}

//DAC interface function, channel 0 is voltage , channel 1 is current
void writeDAC(unsigned dac,unsigned char channel){

      lowB=dac & 0xFF;
      highB=dac >> 8;
      //add command word
      highB+=16;
      highB+=(channel*128);
      PORTB2_bit=0;
      SPI1_Write(highB);
      SPI1_Write(lowB);
      PORTB2_bit=1;
}

//I,V curve, i = f(v)
double CurrentFunction(double voltage,double j,double t){
        double i,e;
        i=(ISC/1000)*j+0.1;
        e=(-10+0.038*(t-25)+(10/VOC)*voltage);
        i-=exp(e);
        return i;

    }

//I,V curve, v = f(i)	
double vFunction(double current,double j,double t){
        double i,ln;
        ln=(ISC/1000)*j+0.1-current;
        ln=log(ln);
        i=10-0.038*(t-25);
        i+=ln;
        i*=3.6;
        return i;

    }
//receive irradiances and temperature from Master device through UART	
void getJT(){           
		   
            char inc=0;      
            for(inc=0;inc<12;inc++){
            
              if(output[inc]==INDEX){
                  
                   lowi = output[inc+1]; 
                   highi = output[inc+2];
                   temp = output[inc+3];
                   
                   irr=highi*256;
                   irr+=lowi;       
              }  
           }                            
           /*
		   Debug
           UART1_Write(lowi);
           UART1_Write(highi);
           UART1_Write(temp);
           */
                         
}