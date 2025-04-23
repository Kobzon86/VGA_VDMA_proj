#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#include "xparameters.h"

// #define VDMA_EN
#define TPG_EN

#include "xvtc.h"
#ifdef TPG_EN
    #include "xv_tpg.h"
#endif
#include "xil_cache.h"
#include "xclk_wiz.h"

#define FIRST_BUFFER_ADDR 0x10000000

// #define PATTERN_GREY
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

XVtc VtcInst;
XClk_Wiz ClkWizInst;
XV_tpg tpg_inst;


void res_change(u8 mode){
    xil_printf("Clock wizard init started\n\r");
    XClk_Wiz_Config *ClkWiz_CfgPtr = XClk_Wiz_LookupConfig(XPAR_CLK_WIZ_0_BASEADDR);
    XClk_Wiz_CfgInitialize(&ClkWizInst, ClkWiz_CfgPtr, ClkWiz_CfgPtr->BaseAddr);

    // u32 readed =  Xil_In32(ClkWiz_CfgPtr->BaseAddr + 0x204);

    Xil_Out32(ClkWiz_CfgPtr->BaseAddr + 0x200, 0x3405);

    switch(mode){
        case XVTC_VMODE_720P:
            Xil_Out32(ClkWiz_CfgPtr->BaseAddr + 0x208, 0xe);
        break;
        case XVTC_VMODE_SVGA:
            Xil_Out32(ClkWiz_CfgPtr->BaseAddr + 0x208, 0x1a);
        break;
        case XVTC_VMODE_XGA:
            Xil_Out32(ClkWiz_CfgPtr->BaseAddr + 0x208, 0x10);
        break;
        default:
            xil_printf("Not supported resolution\n\r");
            return;
        break;
    }
    
    usleep(1);
    Xil_Out32(ClkWiz_CfgPtr->BaseAddr + 0x25C, 0x3);
    usleep(10);
    xil_printf("Clock wizard started\n\r");

    xil_printf("VTC init started\n\r");
    // Initialise the VTC
    XVtc_Config *VTC_Config = XVtc_LookupConfig(XPAR_V_TC_0_BASEADDR);
    XVtc_CfgInitialize(&VtcInst, VTC_Config, VTC_Config->BaseAddress);
	
	/* VTC Configuration */
    XVtc_DisableGenerator(&VtcInst);
	XVtc_Timing XVtc_Timingconf;

    switch(mode){
        case XVTC_VMODE_720P:
            XVtc_ConvVideoMode2Timing(&VtcInst,XVTC_VMODE_720P,&XVtc_Timingconf);
        break;
        case XVTC_VMODE_SVGA:
            XVtc_ConvVideoMode2Timing(&VtcInst,XVTC_VMODE_SVGA,&XVtc_Timingconf);
        break;
        case XVTC_VMODE_XGA:
            XVtc_ConvVideoMode2Timing(&VtcInst,XVTC_VMODE_XGA,&XVtc_Timingconf);
        break;
        default:
        break;
    }

    XVtc_SetGeneratorTiming(&VtcInst, &XVtc_Timingconf);
    XVtc_RegUpdate(&VtcInst);

	/* End of VTC Configuration */

    //Start the VTC generator
    XVtc_EnableGenerator(&VtcInst);
    xil_printf("VTC started!\n\r");

}

void update_image(u8 res_mode, u8 pattern_mode){
    u32 vid_width;
    u32 vid_height;


    if(pattern_mode>0 && pattern_mode<21){
        switch(res_mode){
            case 3:
                vid_width = 1280;
                vid_height = 720;
                res_change(XVTC_VMODE_720P);
            break;
            case 1:
                vid_width = 800;
                vid_height = 600;
                res_change(XVTC_VMODE_SVGA);
            break;
            case 2:
                vid_width = 1024;
                vid_height = 768;
                res_change(XVTC_VMODE_XGA);
            break;
            default:
                xil_printf("Not supported resolution\n\r");
                return;
            break;
        }
        
        XV_tpg_Set_height(&tpg_inst, vid_height);
        XV_tpg_Set_width(&tpg_inst, vid_width);

        // Change the pattern to color bar
        XV_tpg_Set_bckgndId(&tpg_inst, pattern_mode);
                       
        //Start the TPG
        XV_tpg_EnableAutoRestart(&tpg_inst);
        XV_tpg_Start(&tpg_inst);
    }else{
        xil_printf("error: wrong pattern is chosen\n\r");
    }    

}

int main()
{
    init_platform();

    u32 resolution_choise;
    u32 pattern_choise;

    int Status;

    res_change(XVTC_VMODE_720P);

#ifdef VDMA_EN

    UINTPTR Addr1=FIRST_BUFFER_ADDR;

    xil_printf("VDMA init started\n\r");
    
    xil_printf("Initialize frame buffer data...\n\r");
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
                    else 
                    {
                        Xil_Out32(Addr1, 0);
                    }

                    Addr1+=3;
                }

            }
    #else
        #ifdef PATTERN_COL_BARS
            //Write a color bar pattern in memory
            for(int line=0; line < HEIGHT; line++)
            {
                for(int column=0; column < WIDTH; column++)
                {

                    if(column<WIDTH/9)
                    {
                        Xil_Out32(Addr1, RED);
                    }
                    else if(column<2*WIDTH/9)
                    {
                        Xil_Out32(Addr1, GREEN);
                    }
                    else if(column<3*WIDTH/9)
                    {
                        Xil_Out32(Addr1, BLUE);
                    }
                    else if(column<4*WIDTH/9)
                    {
                        Xil_Out32(Addr1, RED_HALF);
                    }
                    else if(column<5*WIDTH/9)
                    {
                        Xil_Out32(Addr1, GREEN_HALF);
                    }
                    else if(column<6*WIDTH/9)
                    {
                        Xil_Out32(Addr1, BLUE_HALF);
                    }
                    else if(column<7*WIDTH/9)
                    {
                        Xil_Out32(Addr1, RED_EIGHTS);
                    }
                    else if(column<8*WIDTH/9)
                    {
                        Xil_Out32(Addr1, GREEN_EIGHTS);
                    }
                    else 
                    {
                        Xil_Out32(Addr1, BLUE_EIGHTS);
                    }

                    Addr1+=3;
                }

            }
        #endif
    #endif

	//Re-enable the data cache
    Xil_DCacheEnable();
    xil_printf("Done\n\r");

	/* Start of VDMA Configuration */
    xil_printf("VDMA Setup...");
    /* Configure the Read interface (MM2S)*/
    // MM2S Control Register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x00, 0x83);//0x8B
    // MM2S Start Address 1
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x5C, FIRST_BUFFER_ADDR);
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x60, FIRST_BUFFER_ADDR);
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x64, FIRST_BUFFER_ADDR);
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x68, FIRST_BUFFER_ADDR);

    // MM2S Frame delay / Stride register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x58, WIDTH*3);
    // MM2S HSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x54, WIDTH*3);
    // MM2S VSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x50, HEIGHT);

    xil_printf("VDMA started!\r\n");
#endif

#ifdef TPG_EN    


    xil_printf("VPG init started\n\r");
    Status = XV_tpg_Initialize(&tpg_inst, XPAR_V_TPG_0_BASEADDR);
    if(Status!= XST_SUCCESS)
    {
            xil_printf("TPG configuration failed\r\n");
            return(XST_FAILURE);
    }



    // Set Resolution 
    XV_tpg_Set_height(&tpg_inst, HEIGHT);
    XV_tpg_Set_width(&tpg_inst, WIDTH);
    // Set Color Space to RGB24
    // u32 readed = XV_tpg_Get_colorFormat(&tpg_inst);
    // XV_tpg_Set_colorFormat(&tpg_inst, 0x0);
    
    // Change the pattern to color bar
    XV_tpg_Set_bckgndId(&tpg_inst, XTPG_BKGND_TARTAN_COLOR_BARS);
    
    // Set Overlay to moving box
    // Set the size of the box
    XV_tpg_Set_boxSize(&tpg_inst, 150);
    XV_tpg_Set_boxColorR(&tpg_inst, 0xff);
    XV_tpg_Set_boxColorG(&tpg_inst, 0xff);
    XV_tpg_Set_boxColorB(&tpg_inst, 0xff);

    // Set the speed of the box
    XV_tpg_Set_motionSpeed(&tpg_inst, 5);
    // Enable the moving box
    XV_tpg_Set_ovrlayId(&tpg_inst, 1);
    
    //Start the TPG
    XV_tpg_EnableAutoRestart(&tpg_inst);
    XV_tpg_Start(&tpg_inst);
   
    xil_printf("TPG started!\r\n");

#endif
    while(1){
        xil_printf("Enter resolution: \n\r 1 - 800x600,\n\r 2 - 1024x768,\n\r 3 - 1280x720\n\r");
        scanf("%d",&resolution_choise);
        xil_printf("Enter pattern:   1 - XTPG_BKGND_H_RAMP,\n\r \
2 - XTPG_BKGND_V_RAMP ,\n\r \
3 - XTPG_BKGND_TEMPORAL_RAM ,\n\r \
4 - XTPG_BKGND_SOLID_RED ,\n\r \
5 - XTPG_BKGND_SOLID_GREEN ,\n\r \
6 - XTPG_BKGND_SOLID_BLUE ,\n\r \
7 - XTPG_BKGND_SOLID_BLACK ,\n\r \
8 - XTPG_BKGND_SOLID_WHITE ,\n\r \
9 - XTPG_BKGND_COLOR_BARS ,\n\r \
10 - XTPG_BKGND_ZONE_PLATE ,\n\r \
11- XTPG_BKGND_TARTAN_COLOR_BARS ,\n\r \
12 - XTPG_BKGND_CROSS_HATCH ,\n\r \
13 - XTPG_BKGND_RAINBOW_COLOR,\n\r \
14 - XTPG_BKGND_HV_RAMP,\n\r \
15 - XTPG_BKGND_CHECKER_BOARD,\n\r \
16 - XTPG_BKGND_PBRS,\n\r \
17 - XTPG_BKGND_DP_COLOR_RAMP,\n\r \
18 - XTPG_BKGND_DP_BW_VERTICAL_LINE,\n\r \
19 - XTPG_BKGND_DP_COLOR_SQUARE,\n\r \
20 - XTPG_BKGND_LAST\n\r");

        scanf("%d",&pattern_choise);
        update_image(resolution_choise,pattern_choise);
    }
    cleanup_platform();
    return 0;
}
