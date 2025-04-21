#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#include "xparameters.h"

#include "xvtc.h"
#include "xil_cache.h"

#define PATTERN_GREY
#define PATTERN_COL_BARS

#define RED   0x00e00000
#define GREEN 0x0000e000
#define BLUE  0x000000e0

#define RED_HALF   0x00600000
#define GREEN_HALF 0x00006000
#define BLUE_HALF  0x00000060

#define RED_EIGHTS   0x00200000
#define GREEN_EIGHTS 0x00002000
#define BLUE_EIGHTS  0x00000020



#define BLUE_YUYV		0x6B1DFF1D
#define RED_YUYV		0xFF4C544C
#define GREEN_YUYV		0x15952B95
#define MAGENTA_YUYV	0xEA69D469
#define YELLOW_YUYV		0X94FF00FF
#define CYAN_YUYV		0x00B2ABB2
#define WHITE_YUYV		0x80FF80FF
#define BLACK_YUYV		0x80008000


#define WIDTH 1280
#define HEIGHT 720

int main()
{
    init_platform();


    int Status;
    XVtc VtcInst;


    print("VTC init started\n\r");
    // Initialise the VTC
    XVtc_Config *VTC_Config = XVtc_LookupConfig(XPAR_V_TC_0_BASEADDR);
    XVtc_CfgInitialize(&VtcInst, VTC_Config, VTC_Config->BaseAddress);
	
	/* VTC Configuration */
	XVtc_Timing XVtc_Timingconf;

    XVtc_ConvVideoMode2Timing(&VtcInst,XVTC_VMODE_720P,&XVtc_Timingconf);
    XVtc_SetGeneratorTiming(&VtcInst, &XVtc_Timingconf);
    XVtc_RegUpdate(&VtcInst);

	/* End of VTC Configuration */

    //Start the VTC generator
    XVtc_EnableGenerator(&VtcInst);
    print("VTC started!\n\r");



    
	UINTPTR Addr1=0x10000000;

    print("VDMA init started\n\r");
    
    print("Initialize frame buffer data...\n\r");
    //Disable the data cache to initialize the frame buffer with a blue color
    Xil_DCacheDisable();

	//Write grey bar pattern in memory
#ifdef PATTERN_GREY
    for(int line=0; line < HEIGHT; line++)
    	{
    		for(int column=0; column < WIDTH; column++)
    		{

    			if(column<WIDTH/8)
    			{
    				Xil_Out32(Addr1, 0x00e0e0e0);
    			}
    			else if(column<2*WIDTH/8)
    			{
    				Xil_Out32(Addr1, 0x00c0c0c0);
    			}
    			else if(column<3*WIDTH/8)
    			{
    				Xil_Out32(Addr1, 0x00a0a0a0);
    			}
    			else if(column<4*WIDTH/8)
    			{
    				Xil_Out32(Addr1, 0x00808080);
    			}
    			else if(column<5*WIDTH/8)
    			{
    				Xil_Out32(Addr1, 0x00606060);
    			}
    			else if(column<6*WIDTH/8)
    			{
    				Xil_Out32(Addr1, 0x00404040);
    			}
    			else if(column<7*WIDTH/8)
    			{
    				Xil_Out32(Addr1, 0x00202020);
    			}
    			else if(column<WIDTH-5)
    			{
    				Xil_Out32(Addr1, 0);
    			}
    			else
    			{
    				Xil_Out32(Addr1, 0);
    			}

    			Addr1+=4;
    		}

        }
#else
	#ifdef PATTERN_COL_BARS
    	//Write a color bar pattern in memory
    	for(int line=0; line < HEIGHT; line++)
    	{
    		for(int column=0; column < WIDTH; column++)
    		{

    			if(column<WIDTH/8)
    			{
    				Xil_Out32(Addr1, RED);
    			}
    			else if(column<2*WIDTH/8)
    			{
    				Xil_Out32(Addr1, GREEN);
    			}
    			else if(column<3*WIDTH/8)
    			{
    				Xil_Out32(Addr1, BLUE);
    			}
    			else if(column<4*WIDTH/8)
    			{
    				Xil_Out32(Addr1, RED_HALF);
    			}
    			else if(column<5*WIDTH/8)
    			{
    				Xil_Out32(Addr1, GREEN_HALF);
    			}
    			else if(column<6*WIDTH/8)
    			{
    				Xil_Out32(Addr1, BLUE_HALF);
    			}
    			else if(column<7*WIDTH/8)
    			{
    				Xil_Out32(Addr1, RED_EIGHTS);
    			}
    			else if(column<WIDTH-5)
    			{
    				Xil_Out32(Addr1, GREEN_EIGHTS);
    			}
    			else
    			{
    				Xil_Out32(Addr1, 0);
    			}

    			Addr1+=4;
    		}

        }
	#endif
#endif

	//Re-enable the data cache
    Xil_DCacheEnable();
    print("Done\n\r");

	/* Start of VDMA Configuration */
    xil_printf("VDMA Setup...");
    /* Configure the Read interface (MM2S)*/
    // MM2S Control Register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x00, 0x8B);
    // MM2S Start Address 1
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x5C, 0x10000000);
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x60, 0x10000000);
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x64, 0x10000000);
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x68, 0x10000000);

    // MM2S Frame delay / Stride register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x58, WIDTH*4);
    // MM2S HSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x54, WIDTH*4);
    // MM2S VSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x50, HEIGHT);

    xil_printf("VDMA started!\r\n");
    
    while(1);

    cleanup_platform();
    return 0;
}
