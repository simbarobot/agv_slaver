//*****************************************************************************
//
// timers.c - Timers example.
//
// Copyright (c) 2013-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the EK-TM4C1294XL Firmware Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/pwm.h"
#include "driverlib/adc.h"
#include "driverlib/systick.h"
#include "utils/uartstdio.h"
#include "utils/cpu_usage.h"
#include "system.h"
#include "motor.h"
#include "sensor.h"
#include "rfid.h"
#include "rfid.h"
#include "zigbee.h"
#include "t6963c.h"
#include "batterysensor.h"
#include "config.h"
#include "modbus-rtu.h"
//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Timer (timers)</h1>
//!
//! This example application demonstrates the use of the timers to generate
//! periodic interrupts.  One timer is set up to interrupt once per second and
//! the other to interrupt twice per second; each interrupt handler will toggle
//! its own indicator throught the UART.
//!
//! UART0, connected to the Virtual Serial Port and running at 115,200, 8-N-1,
//! is used to display messages from this application.
//
//*****************************************************************************

//****************************************************************************
//
// System clock rate in Hz.
//
//****************************************************************************
char ZIGBEE_ID;
/*
 * [WARNING] System defination
 *
 */
uint32_t g_ui32CPUUsage;
uint32_t g_ui32Seconds;
uint32_t g_ui32SysClock;
uint8_t g_UART3RX1[UARTRX_BUFFER_SIZE];
uint8_t g_UART3RX2[UARTRX_BUFFER_SIZE];
uint8_t g_UART2RX1[UART2RX_BUFFER_SIZE];
uint8_t g_UART2RX2[UART2RX_BUFFER_SIZE];
//*****************************************************************************
//
// This example application demonstrates the use of the timers to generate
// periodic interrupts.
//
//*****************************************************************************
uint8_t sensor1[8];
uint8_t senso = 0;
int8_t xa = 0;
int32_t phai = 0;
int32_t trai = 0;
int den = 0;
int chieu = 1;
int loi = 0;
int loi1 = 1;
int loi2 = 0;
int loi10 = 1;
int dung = 0;
int di = 0;
int hienthi = 1;
int nan_ha = 0;
int tram0 = 0;
int tram1 = 0;
int boqua = 1;
int thongbao = 0;
int32_t time = 0;
int32_t speed = 10;
uint8_t ROBOT_STATE = 0;
uint8_t SENSOR_STATE = 0;
uint32_t tocdo = 900; // toc do robot max 600
uint32_t biengiamtoc = 3; // gia tri giam toc
//uint32_t bientantoc = 5000;  // gia tri tan toc
uint32_t bientantoc;
uint32_t tocdogiam = 300;   // tang giam thoi gian cham dan
uint8_t tocdotan = 10;    // tang giam thoi gian nhah dan
int i;
int ht_tram1 = 1;
int ht_tram0 = 1;
int binh = 1;
int xuong = 1;
int  bientram4  = 0;
int32_t leftm, rightm;
uint32_t noline_counter = 0;

//                0   1   2   3   4   5   6    7
//int32_t cap[8] = { 4500, 100, 200, 250, 390, 520, 690, 700 };
//int32_t cap[8] = { 5000, 200, 500, 1000, 1800, 2900, 3300, 4000 };
int32_t cap[8] = { 6000, 300, 800, 1500, 2300, 3200, 4300, 5500 };
//int32_t cap[8] = { 4500, 200, 500, 900, 1500, 2000, 2600, 3400 };
void runsenso2(void);
void stop1(void);
void dithang(void);

uint16_t data[10];

int8_t zigbeesentpackage[BUFFER_SIZE];
uint32_t rfid_location;

uint8_t batteryStatus[3][6] = { "HIGH", "MEDIUM", "LOW" };

/*
 *
 */
// REGISTER 0 => Respond (Report cho Master)
// REGISTER 1 => Requested (Server yeu cau di den tram)
/*
 *
 */
uint32_t adcvalue[12];
uint32_t robotstatus = 0;
uint32_t debound1=0;
uint32_t debound2=0;
//////////////reset  khi treo////////////////////////
//ROM_WatchdogEnable(WATCHDOG0_BASE);

/////////////////////////////////////////////
void main(void)
{
	init();
	ROM_IntMasterDisable();
	if (STATE == 0) {
		MotorInit();
		GPDSensorInit();
		LineSensorInit();
		rfidInit();
		zigbeeInit(1, UART3_BASE);
		ADCInit();
		GLCD_Initalize();
		STATE = 1;
	}
	ROM_IntMasterEnable();
	bientantoc = 5000;
	//
	// Enable processor interrupts.
	//
	ROM_TimerEnable(TIMER0_BASE, TIMER_A);
	ROM_TimerEnable(TIMER1_BASE, TIMER_A);
	ROM_TimerEnable(TIMER3_BASE, TIMER_A);

	SysCtlDelay(g_ui32SysClock / 1000);

	GLCDPrintfNormal(0, 0, "  -----  ROBOTIC AGV -----");
	//GLCDPrintfNormal(0, 3, "Err : 0              ");
//      GLCDPrintfNormal(0, 2, "Batery: %d (volt)  ", adcvalue / 124);

	// Loop forever while the timers run.
//     ROM_TimerEnable(TIMER0_BASE, TIMER_A);
	//   MotorController(4000, 4000);
	UARTprintf("robot 0k \n");
	dung = 1;
	uint32_t batterypercent = 0;
	while (1) {
		ROM_IntMasterDisable();
		//////////////////////////////////////////////////////
		if (binh == 1) {
			//GLCDPrintfNormal(5, 1, "RX1: %d, RX2: %d ", ROBOTRX_Buffer[0], ROBOTRX_Buffer[1]);
			batterypercent = ADCGet(adcvalue);
			GLCDPrintfNormal(0, 3, "Battery     : %2d (%%) ", batterypercent);
			binh = 0;
		}

//		GLCDPrintfNormal(0, 1, "Robot RX Data: %d %d %d %d %d",
//				ROBOTRX_Buffer[0], ROBOTRX_Buffer[1], ROBOTRX_Buffer[2],
//				ROBOTRX_Buffer[3], ROBOTRX_Buffer[4]);
//
		senso = GPIOPinRead(GPIO_PORTM_BASE, 0xff);
        GLCDPrintfNormal(0, 1, "Line Value   : %d%d%d%d%d%d%d%d", (senso & 0x80)>>7,
                         (senso & 0x40)>>6, (senso & 0x20)>>5, (senso & 0x10)>>4, (senso & 0x08)>>3,
                         (senso & 0x04)>>2, (senso & 0x02)>>1, (senso & 0x01));
//        UARTprintf("Line Value   : %d%d%d%d%d%d%d%d\n", (senso & 0x80)>>7,
//                   (senso & 0x40)>>6, (senso & 0x20)>>5, (senso & 0x10)>>4, (senso & 0x08)>>3,
//                   (senso & 0x04)>>2, (senso & 0x02)>>1, (senso & 0x01));

		GLCDPrintfNormal(0, 2, "Motor Speed  : %3d %%, %3d %%", rightm/240, leftm/240);
		///////////////////////////////////////////////////
		if (loi2 == 0) {
			if (loi1 != loi) {
				switch (loi) {
				case 1:
					GLCDPrintfNormal(0, 5, "Error       : forewarning 1");
					break;

				case 2:
					GLCDPrintfNormal(0, 5, "Error       : forewarning 2");
					break;

				case 3:
					GLCDPrintfNormal(0, 5, "Error       : forewarning 3");
					break;

				case 4:
					GLCDPrintfNormal(0, 5, "Error       : forewarning 4");
					break;

				case 5:
					GLCDPrintfNormal(0, 5, "Error       : forewarning 5");
					break;

				case 9:
					GLCDPrintfNormal(0, 5, "Error       : accident      ");
					break;

				case 0:
					GLCDPrintfNormal(0, 5, "Error       : NONE          ");
					break;
				}
				loi1 = loi;
				loi10 = 1;
			}
		} else {
			if (loi10 == 1) {
				GLCDPrintfNormal(0, 3, "Err : no line        ");
				loi10 = 0;
			}
		}
		///////////////////////////////////////////////////
		//if (hienthi == 1) {
		if (1) {
			GLCDPrintfNormal(0, 4, "RFID        : %s",RFID_ID);

			hienthi = 0;
			if (++robotstatus == 100) {
				if (dung == 1) {
					GLCDPrintfNormal(0, 6, "Robot       : Ready(%d)   ", boqua);
				} else {
					GLCDPrintfNormal(0, 6, "Robot       : Stop   ");
				}
			} else if (robotstatus == 150) {
				robotstatus = 0;
				GLCDPrintfNormal(0, 6, "Robot       :           ");
			}
		}
		if (1) {
			GLCDPrintfNormal(0, 7, "Requesting  : %d => %d ", tram0, tram1);
			ht_tram1 = 0;
		}

		//////////////////// nan ha khay //////////////////////////
		if (ROBOTRX_Buffer[1] == 1 && nan_ha == 0) {
			nan_ha = 1;
			GLCDPrintfNormal(0, 4, "The Carrier INCREASED !!!");
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
		}

		if (ROBOTRX_Buffer[1] == 0 && nan_ha == 1) {
			nan_ha = 0;
			GLCDPrintfNormal(0, 4, "                         ");
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
		}

//		if (tram1 == tram0) {
//			if (nan_ha == 1 && tram0 == 5) {
//
//				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
//				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
//				xuong = 0;
//
//			} else {
//				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
//				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
//
//				xuong = 1;
//			}
//		} else {
//			nan_ha = 0;
//			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
//			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
//		}
		if(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2) == GPIO_PIN_2)
		{
		    if(++debound1 == 10)
		    {
		        ROBOT_STATE = 1;
		        dung = 1;
		        loi = 0;
		    }
		}
		else
		{
		    debound1 = 0;
		}
        if(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_3) == 0)
        {
            if(++debound2 == 10)
            {
                dung = 0;
                //ROBOT_STATE = 0;
            }
        }
        else
        {
            debound2 = 0;
        }
		ROM_IntMasterEnable();
////////////////////////////////////////////////////////////
		SysCtlDelay(g_ui32SysClock / 1000);
	}
}

//*****************************************************************************
//
// System Interupt Handler
//
//*****************************************************************************

void PORTJIntHandler(void) {
	uint32_t PortFmask = GPIOIntStatus(GPIO_PORTJ_BASE,
	GPIO_PIN_0 | GPIO_PIN_1);
	if (di == 1 && loi == 0) {
		if (PortFmask & GPIO_PIN_0) {
			/////////////////////////////////////////////////////////////////////////////////////////////
			UARTprintf("\n test loi");
//        loi = 1;
//         time = 0;
			/////////////////////////////////////////////////////////////////////////////////////////////
			SysCtlDelay(SysCtlClockGet() / 200);
			GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);
		}
		if (PortFmask & GPIO_PIN_1) {
			/////////////////////////////////////////////////////////////////////////////////////////////
			// UARTprintf("\n ha xuong !");
			/////////////////////////////////////////////////////////////////////////////////////////////
			SysCtlDelay(SysCtlClockGet() / 200);
			GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_1);
		}
	} else {
		GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);
	}

}
//*****************************************************************************
//
// The interrupt handler for the first timer interrupt.
//
//*****************************************************************************
void Timer0IntHandler(void) {
////////////////////////////////////////////

	if (tram1 == tram0 ) {
		stop1();
	} else {
		if( bientram4  == 0){
		if (boqua == 1) {
			if (GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6) == 64) {
				loi = 1;
				time = 0;
			}
			////////////
			if (

			GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) == 128) {
				loi = 2;
				time = 0;
			}
			if (

			GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_0) == 1) {
				loi = 3;
				time = 0;
			}
			if (

			GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_3) == 8) {
				loi = 4;
				time = 0;
			}
			if (

			GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_2) == 4) {
				loi = 5;
				time = 0;
			}
		}
		if (GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_1) == 2) {
			loi = 9;
			dung = 0;
			hienthi = 1;
			time = 0;
		}

		dithang();
		}
	}
	//////////////////////////////////////////////

	ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

}

//*****************************************************************************
//
// The interrupt handler for the second timer interrupt.
//
//*****************************************************************************
void Timer1IntHandler(void) {
	//ADCGet(&adcvalue);
	//
	// Clear the timer interrupt.  ///////7170BC2 // F3E0BA2

	/////0DA9BB2 // EB88975

	//
	//UARTprintf("Timer1IntHandler \n");
	if (RFID_ID[0] == ':') {
		binh = 1;
		//   UARTprintf("RFID ID%s\n", RFID_ID);
		if (
		        (strcmp(RFID_ID, ":1507A88") == 0) ||
		        (strcmp(RFID_ID, STATION1IDM1) == 0) ||
                (strcmp(RFID_ID, ":650C935") == 0)
		   )
		{
			UARTprintf("da toi tram 1\n");
			ROBOTTX_Buffer[0] = 1;
			tram0 = 1;
			ht_tram0 = 1;

		}
		else if (
		        (strcmp(RFID_ID, ":C4D1060") == 0) ||
		        (strcmp(RFID_ID, STATION2IDM2) == 0) ||
                (strcmp(RFID_ID, ":4FAD925") == 0)
		        )
		{
			UARTprintf("da toi tram 2\n");
			ROBOTTX_Buffer[0] = 2;
			tram0 = 2;
			ht_tram0 = 1;

		} else if (
                (strcmp(RFID_ID, ":C41EBE0") == 0) ||
                (strcmp(RFID_ID, ":32AAB12") == 0) ||
		        (strcmp(RFID_ID, STATION3IDM3) == 0)
		        )
		{
			UARTprintf("da toi tram 3\n");
			tram0 = 3;
			ht_tram0 = 1;
			ROBOTTX_Buffer[0] = 3;

		} else if (
                (strcmp(RFID_ID, ":C42A510") == 0) ||
                (strcmp(RFID_ID, ":98D8925") == 0) ||
		        (strcmp(RFID_ID, STATION4IDM4) == 0)
		        )
		{
			UARTprintf("da toi tram 4\n");
			ROBOTTX_Buffer[0] = 4;
			tram0 = 4;
			ht_tram0 = 1;
			bientram4 = 1;
		} else if (
                (strcmp(RFID_ID, ":C476220") == 0) ||
                (strcmp(RFID_ID, ":047ABB2") == 0) ||
		        (strcmp(RFID_ID, STATION5IDM5) == 0)
		        )
		{
			ROBOTTX_Buffer[0] = 5;
			ht_tram0 = 1;
			UARTprintf("da toi tram 5\n");
			tram0 = 5;
		}
		else if (
		        (strcmp(RFID_ID, ":7170BC2") == 0) ||
				(strcmp(RFID_ID, ":F3E0BA2") == 0) ||
				(strcmp(RFID_ID, ":EB84BB2") == 0) ||
				// Nguyen Truong
                (strcmp(RFID_ID, ":8C34D72") == 0) ||
                (strcmp(RFID_ID, ":4C21D72") == 0)
				)
		{
			bientantoc = 4000;
			boqua = 0;

		}
		else if (
		        (strcmp(RFID_ID, ":0DA9BB2") == 0) ||
				(strcmp(RFID_ID, ":EB88975") == 0)
				// Nguyen Truong
				)
		{
			bientantoc = 10000;
			boqua = 1;

		}
//////////////////////////////////////////////////////////////////////
		else if (
		        (strcmp(RFID_ID, ":D395925") == 0) ||
				(strcmp(RFID_ID, ":38C3BB2") == 0) ||
				(strcmp(RFID_ID, ":28AAB02") == 0) ||
				(strcmp(RFID_ID, ":FB1D935") == 0) ||
				(strcmp(RFID_ID, ":B40D935") == 0) ||
				(strcmp(RFID_ID, ":50E7925") == 0) ||
				(strcmp(RFID_ID, ":5CC2925") == 0) ||
                (strcmp(RFID_ID, ":5070925") == 0) ||
				// Nguyen Truong
                (strcmp(RFID_ID, ":A08BD72") == 0) ||
                (strcmp(RFID_ID, ":DC6EDE2") == 0) ||
                (strcmp(RFID_ID, ":C779D62") == 0) ||
                (strcmp(RFID_ID, ":D050D72") == 0) ||
                (strcmp(RFID_ID, ":94D5925") == 0) ||
                (strcmp(RFID_ID, ":F747B12") == 0)
				 )
		{
			UARTprintf("tang toc \n");
			bientantoc = 10000;
			boqua = 1;
		}
		/////  ////////////               giam toc /////////////////////////
		else if (
		        (strcmp(RFID_ID, ":5D99925") == 0) ||
				(strcmp(RFID_ID, ":F636925") == 0) ||
				(strcmp(RFID_ID, ":65EB925") == 0) ||
				(strcmp(RFID_ID, ":62C4BA2") == 0) ||
				(strcmp(RFID_ID, ":EFAABB2") == 0) ||
				(strcmp(RFID_ID, ":1BC7BB2") == 0) ||
                (strcmp(RFID_ID, ":4DEBBB2") == 0) ||
                // Nguyen Truong added
                (strcmp(RFID_ID, ":27C7D62") == 0) ||
                (strcmp(RFID_ID, ":EA31D72") == 0) ||
                (strcmp(RFID_ID, ":EFAABB2") == 0) ||
                (strcmp(RFID_ID, ":6A96D72") == 0) ||
                (strcmp(RFID_ID, ":DA84D62") == 0) ||
                (strcmp(RFID_ID, ":0067B22") == 0) ||
                (strcmp(RFID_ID, ":99C8BB2") == 0) ||
                (strcmp(RFID_ID, ":EA31D72") == 0) ||
                (strcmp(RFID_ID, ":9174D72") == 0) ||
                (strcmp(RFID_ID, ":D225D72") == 0)
				)
		{
			UARTprintf("giam toc \n");
			bientantoc = 4000;
			boqua = 1;
		}

//////////////////////////////////////////////////////////////////////
		else {
			UARTprintf("ERROR: Unknown RFID %s ????\n", RFID_ID);
		}
		RFID_ID[0] = 0;
	}
	rfid_location = tram0;
	ROM_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}

//*****************************************************************************
//
// The interrupt handler for the second timer interrupt.
//
//*****************************************************************************
void Timer3IntHandler(void) {
	//
	// Clear the timer interrupt.
	//
	ROM_TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
	if (1) {
		if (ROBOTRX_Buffer[0] > 0) {
			ROBOTTX_Buffer[1] = ROBOTRX_Buffer[0];
		//	ht_tram0 = 1;
			// UARTprintf("Received location: %d\n", ROBOTRX_Buffer[5]);
//			if (ROBOTRX_Buffer[0] == 6 && tram1 != 4) {
//				tram1 = 4;
//                tram0 = 0;
//                dung = 1;
//				ht_tram1 = 1;
//				UARTprintf("di tram 4\n");
//			}

			if (ROBOTRX_Buffer[0] == 1 && tram1 != 1) {
				tram1 = 1;
			//	tram0 = 0;
				bientram4 = 0;
				if (ROBOT_STATE == 1) {
					dung = 1;
				}

				ht_tram1 = 1;
				UARTprintf("di tram 1\n");
			}
			if (ROBOTRX_Buffer[0] == 2 && tram1 != 2) {
				tram1 = 2;
			//	tram0 = 0;
				ht_tram1 = 1;
				if (ROBOT_STATE == 1) {
					dung = 1;
				}
				UARTprintf("di tram 2\n");
			}
			if (ROBOTRX_Buffer[0] == 3 && tram1 != 3) {
				tram1 = 3;
			//	tram0 = 0;
				if (ROBOT_STATE == 1) {
					dung = 1;
				}
				loi = 0;
				ht_tram1 = 1;
				UARTprintf("di tram 3\n");
			}
			if (ROBOTRX_Buffer[0] == 4 && tram1 != 4) {
				tram1 = 4;
			//	tram0 = 0;
				ht_tram1 = 1;
				if (ROBOT_STATE == 1) {
					dung = 1;
				}
				UARTprintf("di tram 4\n");
			}
			if (ROBOTRX_Buffer[0] == 5 && tram1 != 5) {
				tram1 = 5;
			//	tram0 = 0;
				if (ROBOT_STATE == 1) {
					dung = 1;
				}
				loi = 0;
				ht_tram1 = 1;
				UARTprintf("di tram 5\n");
			}
//             if (ROBOTRX_Buffer[0] == 6 && dung != 1)
//             {
//
//
//                 dung = 1;
//                 hienthi = 1;
//
//                 UARTprintf("run\n");
//             }
//			if (ROBOTRX_Buffer[0] == 7 && tram1 != 1) {
//
//				tram1 = 1;
//				tram0 = 0;
//				dung = 1;
//				ht_tram1 = 1;
//
//				UARTprintf("di tram 1\n");
//
//				UARTprintf("stop\n");
//			}
//             if (ROBOTRX_Buffer[0] == )
//             {
//                 //      ROM_TimerDisable(TIMER0_BASE, TIMER_A);
//                 dung = 0;
//
//                 UARTprintf("stop\n");
//             }
		}
	} else {
		UARTprintf("[WARNING] Robot in processing!!!\n");
	}

}
//*****************************************************************************
//
// The interrupt handler for the second timer interrupt.
//
//*****************************************************************************

//*****************************************************************************
//
// The interrupt handler for the second timer interrupt.
//
//*****************************************************************************

void PORTBIntHandler(void) {
	uint32_t PortBmask = GPIOIntStatus(GPIO_PORTB_BASE,
	GPIO_PIN_2 | GPIO_PIN_3);
	SysCtlDelay(SysCtlClockGet() / 100);
	hienthi = 1;
	if (PortBmask & GPIO_PIN_2) {
		/////////////////////////////////////////////////////////////////////////////////////////////
		UARTprintf("\n run!");
		ROBOT_STATE = 1;
		dung = 1;
		loi = 0;
		/////////////////////////////////////////////////////////////////////////////////////////////
		//  SysCtlDelay(SysCtlClockGet() / 100);
		GPIOIntClear(GPIO_PORTB_BASE, GPIO_PIN_2);
	}
	if (PortBmask & GPIO_PIN_3) {
		/////////////////////////////////////////////////////////////////////////////////////////////
		UARTprintf("\n stop !");
		dung = 0;
		ROBOT_STATE = 0;
		/////////////////////////////////////////////////////////////////////////////////////////////
		// SysCtlDelay(SysCtlClockGet()/ 100);
		GPIOIntClear(GPIO_PORTB_BASE, GPIO_PIN_3);
	}

}

//*****************************************************************************
//
// The interrupt handler for the second timer interrupt.
//
//*****************************************************************************
void PORTCIntHandler(void) {
//    uint32_t PortCmask = GPIOIntStatus(
//    GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);
//    if (di == 1 && loi == 0)
//    {
//    	SysCtlDelay(SysCtlClockGet() / 500);
//        if (PortCmask & GPIO_PIN_6)
//        {
//            /////////////////////////////////////////////////////////////////////////////////////////////
//
//            UARTprintf("\n vat can xa 3 !");
//            loi = 3;
//            time = 0;
//            UARTprintf("\n ma loi: %d", loi);
//            /////////////////////////////////////////////////////////////////////////////////////////////
//            SysCtlDelay(SysCtlClockGet() / 1000);
//            GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_6);
//        }
//        if (PortCmask & GPIO_PIN_7)
//        {
//            /////////////////////////////////////////////////////////////////////////////////////////////
//            UARTprintf("\n vat can xa 4 !");
//            loi = 4;
//            time = 0;
//            UARTprintf("\n ma loi: %d", loi);
//            /////////////////////////////////////////////////////////////////////////////////////////////
//            SysCtlDelay(SysCtlClockGet() / 1000);
//            GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_7);
//        }
//    }
//    else
//    {
//        GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_7);
//        GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_6);
//
//    }
	///////////////////
	GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_7);
	GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_6);

}

void PORTHIntHandler(void) {
//    uint32_t PortHmask = GPIOIntStatus(
//            GPIO_PORTH_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
//    if (di == 1 && loi == 0 )
//    {
//    	SysCtlDelay(SysCtlClockGet() / 500);
//        if (PortHmask & GPIO_PIN_0)
//        {
//            /////////////////////////////////////////////////////////////////////////////////////////////
//            UARTprintf("\n vat can xa 5 !");
//            loi = 5;
//            time = 0;
//            UARTprintf("\n ma loi: %d", loi);
//            /////////////////////////////////////////////////////////////////////////////////////////////
//            SysCtlDelay(SysCtlClockGet() / 10000);
//            GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_0);
//        }
//        if (PortHmask & GPIO_PIN_1)
//        {
//            /////////////////////////////////////////////////////////////////////////////////////////////
//            UARTprintf("\n vat can xa 6!");
//            loi = 9;
//            dung = 0;
//            UARTprintf("\n ma loi: %d", loi);
//            /////////////////////////////////////////////////////////////////////////////////////////////
//            SysCtlDelay(SysCtlClockGet() / 10000);
//            GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_1);
//        }
//        if (PortHmask & GPIO_PIN_2)
//        {
//            /////////////////////////////////////////////////////////////////////////////////////////////
//            UARTprintf("\n vat can xa 2 !");
//            loi = 2;
//            time = 0;
//            UARTprintf("\n ma loi: %d", loi);
//            /////////////////////////////////////////////////////////////////////////////////////////////
//            SysCtlDelay(SysCtlClockGet() / 10000);
//            GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_2);
//        }
//        if (PortHmask & GPIO_PIN_3)
//        {
//            /////////////////////////////////////////////////////////////////////////////////////////////
//            UARTprintf("\n vat can xa 1 !");
//            loi = 1;
//            time = 0;
//            UARTprintf("\n ma loi: %d", loi);
//            /////////////////////////////////////////////////////////////////////////////////////////////
//            SysCtlDelay(SysCtlClockGet() / 10000);
//            GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_3);
//        }
//    }
//    else
//    {
//        GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_3);
//        GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_2);
//        GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_0);
//
//    }
//    if (PortHmask & GPIO_PIN_1)
//         {
//             /////////////////////////////////////////////////////////////////////////////////////////////
//             UARTprintf("\n vat can xa 6!");
//             loi = 9;
//             dung = 0;
//             UARTprintf("\n ma loi: %d", loi);
//             /////////////////////////////////////////////////////////////////////////////////////////////
//             SysCtlDelay(SysCtlClockGet() / 10000);
//             GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_1);
//         }
/////////////////////////////////////

	GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_3);
	GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_2);
	GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_0);
	GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_1);

}

//*****************************************************************************
//
// The interrupt handler for the second timer interrupt.
//
//*****************************************************************************
//void
//Timer4IntHandler(void)
//{
//    //
//    // Clear the timer interrupt.
//    //
//    ROM_TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
//}
//*****************************************************************************
//
// The interrupt handler for the second timer interrupt.
//
//*****************************************************************************
void Timer5IntHandler(void) {

	//
	// Clear the timer interrupt.
	//
	ROM_TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
}

// Vector Rx/Tx UART0 from PC
void UART0IntHandler(void) {
	UARTIntClear(UART0_BASE, UART_INT_RX);
	while (UARTCharsAvail(UART0_BASE)) {
		switch (UARTCharGet(UART0_BASE)) {
		case 'a':

			UARTprintf("\n stop robot !");
			dung = 0;
			hienthi = 1;
			// loi = 1;
			break;
		case 'd':

			UARTprintf("\n run!");
			dung = 1;
			hienthi = 1;
			loi = 0;

			break;
		case 'w':

			UARTprintf("\n nan len");
			nan_ha = 1;

			break;
		case 's':

			UARTprintf("\n ha xuong");
			nan_ha = 0;

			break;
		case '1':
		    memcpy(RFID_ID, STATION1IDM1, sizeof(STATION1IDM1));
			break;
		case '2':
            memcpy(RFID_ID, STATION2IDM2, sizeof(STATION2IDM2));
			break;
		case '3':
            memcpy(RFID_ID, STATION3IDM3, sizeof(STATION3IDM3));
			break;
		case '4':
            memcpy(RFID_ID, STATION4IDM4, sizeof(STATION4IDM4));
			break;
		case '5':
            memcpy(RFID_ID, STATION5IDM5, sizeof(STATION5IDM5));
		    break;
		default:
			UARTprintf("Unknow\n");
			break;
		}
	}
}

void stop1(void) {
	//  UARTprintf("\n giam: %d", biengiamtoc);
	di = 0;
	den = 0;
	if (biengiamtoc > 2000) {
		biengiamtoc = biengiamtoc - tocdogiam;
		tocdo = biengiamtoc;
	} else {
		biengiamtoc = 3;
	}

	MotorController(biengiamtoc, biengiamtoc);

}
void dithang(void) {

	if (dung == 1) {
		if (loi == 0) {

			runsenso2();

			den = 1;
			di = 1;

		} else {
			stop1();
			time = time + 1;
			//UARTprintf("\n time: %d", time);

			if (time >= 300) {
				loi = 0;
			}
//			} else {
//				if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == 0
//						|| GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6) == 64
//						|| GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) == 128
//						|| GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_0) == 1
//						|| GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_1) == 2
//						|| GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_2) == 4
//						|| GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_3) == 8) {
//
////					UARTprintf("\n c6= %d",
////							GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6));
////					UARTprintf("\n c7= %d",
////							GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7));
////					UARTprintf("\n h0= %d",
////							GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_0));
////					UARTprintf("\n h1= %d",
////							GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_1));
////					UARTprintf("\n h2= %d",
////							GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_2));
////					UARTprintf("\n h3= %d",
////							GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_3));
//					time = 0;
//				}
//			}

		}
	} else {
		den = 0;
		trai = 3;
		phai = 3;
		MotorController(trai, phai);
		tocdo = 100;
	}

}
void runsenso2(void) {
	senso = GPIOPinRead(GPIO_PORTM_BASE, 0xff);

	unsigned short mask = 128;
	for (i = 0; i < 8; i++) {
		if (senso & mask) {
			//    UARTprintf("1");
			sensor1[i] = 1;
		} else {
			//   UARTprintf("0");
			sensor1[i] = 0;
		}
		mask >>= 1;
	}
	if (tocdo > bientantoc) {

		// tocdo = 5500;
		tocdo = tocdo - 300 ;
		//tocdo = bientantoc;
	}
//	UARTprintf("\n");
	//	UARTprintf("%d %d %d %d %d %d %d %d",);
	//SysCtlDelay(SysCtlClockGet() / 1000);
	////////////////////////////
	//  UARTprintf("\n td: %d", tocdo);
//         tocdo = tocdo + tocdotan;
//         biengiamtoc = tocdo;
	/////////////////////////////////
	if (sensor1[3] == 1 && sensor1[4] == 1) {
		tocdo = tocdo + tocdotan;
		biengiamtoc = tocdo;

		phai = tocdo;
		trai = tocdo;

	} else {

		if (sensor1[2] == 1 && sensor1[3] == 1) {
			tocdo = tocdo + tocdotan;
			biengiamtoc = tocdo;
			phai = tocdo + cap[2];
			trai = tocdo - cap[2];
			loi2 = 0;
		} else {
			if (sensor1[1] == 1 && sensor1[2] == 1) {
				tocdo = tocdo + tocdotan;
				biengiamtoc = tocdo;
				phai = tocdo + cap[4];
				trai = tocdo - cap[4];

			} else {
				if (sensor1[0] == 1 && sensor1[1] == 1) {
					tocdo = tocdo + tocdotan;
					biengiamtoc = tocdo;
					phai = tocdo + cap[6];
					trai = tocdo - cap[6];

				} else {
					if (sensor1[3] == 1) {
						tocdo = tocdo + tocdotan;
						biengiamtoc = tocdo;
						phai = tocdo + cap[1];
						trai = tocdo - cap[1];

					}
					if (sensor1[2] == 1) {
						tocdo = tocdo + tocdotan;
						biengiamtoc = tocdo;
						phai = tocdo + cap[3];
						trai = tocdo - cap[3];

					}
					if (sensor1[1] == 1) {
						tocdo = tocdo + tocdotan;
						biengiamtoc = tocdo;
						phai = tocdo + cap[5];
						trai = tocdo - cap[5];

					}
					if (sensor1[0] == 1) {
						tocdo = tocdo + tocdotan;
						biengiamtoc = tocdo;
						phai = tocdo + cap[7];
						trai = tocdo - cap[7];
					}
				}
			}
		}

////////////////////////////////////////

		if (sensor1[4] == 1 && sensor1[5] == 1) {
			tocdo = tocdo + tocdotan;
			biengiamtoc = tocdo;
			phai = tocdo - cap[2];
			trai = tocdo + cap[2];
		} else {
			if (sensor1[5] == 1 && sensor1[6] == 1) {
				tocdo = tocdo + tocdotan;
				biengiamtoc = tocdo;
				phai = tocdo - cap[4];
				trai = tocdo + cap[4];

			} else {

				if (sensor1[6] == 1 && sensor1[7] == 1) {
					tocdo = tocdo + tocdotan;
					biengiamtoc = tocdo;
					phai = tocdo - cap[6];
					trai = tocdo + cap[6];

				}

				else {
					if (sensor1[7] == 1) {
						tocdo = tocdo + tocdotan;
						biengiamtoc = tocdo;
						phai = tocdo - cap[7];
						trai = tocdo + cap[7];

					}
					if (sensor1[4] == 1) {
						tocdo = tocdo + tocdotan;
						biengiamtoc = tocdo;
						phai = tocdo - cap[1];
						trai = tocdo + cap[1];

					}

					if (sensor1[6] == 1) {
						tocdo = tocdo + tocdotan;
						biengiamtoc = tocdo;
						phai = tocdo - cap[5];
						trai = tocdo + cap[5];

					}
					if (sensor1[5] == 1) {
						tocdo = tocdo + tocdotan;
						biengiamtoc = tocdo;
						phai = tocdo - cap[3];
						trai = tocdo + cap[3];

					}
				}
			}

		}
	}

    if (sensor1[0] == 0 && sensor1[1] == 0 && sensor1[2] == 0 && sensor1[3] == 0
            && sensor1[4] == 0 && sensor1[5] == 0 && sensor1[6] == 0
            && sensor1[7] == 0)

    {
        if(++noline_counter == NOLINE_TIMEOUT)
        {
            stop1();
            loi2 = 10;
            noline_counter=0;
        }
    }
    else
    {
        noline_counter = 0;
        // UARTprintf( "\ntrai: %d phai: %d  \n", trai, phai);
        MotorController(phai, trai);
        //  UARTprintf("\n trai: %d  phai %d", phai , trai);
    }
}
// Vector Rx/Tx UART2 from RFID
void UART1IntHandler(void) {
	UARTIntClear(UART1_BASE, UART_INT_RX);
	int32_t incoming = 0;
	while (UARTCharsAvail(UART1_BASE)) {
		incoming = UARTCharGet(UART1_BASE);
		UARTprintf("INCOMING %3d: %x\n", incoming);

	}
}
