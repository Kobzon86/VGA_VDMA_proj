#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#include "xparameters.h"

#include "xv_tpg.h"
#include "xvtc.h"

#define WIDTH 1280
#define HEIGHT 720

int main()
{
    init_platform();


    int Status;
    XV_tpg tpg_inst;
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



    print("VPG init started\n\r");
    Status = XV_tpg_Initialize(&tpg_inst, XPAR_V_TPG_0_BASEADDR);
    if(Status!= XST_SUCCESS)
    {
            xil_printf("TPG configuration failed\r\n");
            return(XST_FAILURE);
    }



    // Set Resolution to 1280x720
    XV_tpg_Set_height(&tpg_inst, HEIGHT);
    XV_tpg_Set_width(&tpg_inst, WIDTH);
    // Set Color Space to RGB24
    // u32 readed = XV_tpg_Get_colorFormat(&tpg_inst);
    // XV_tpg_Set_colorFormat(&tpg_inst, 0x0);
    
    // Change the pattern to color bar
    XV_tpg_Set_bckgndId(&tpg_inst, XTPG_BKGND_COLOR_BARS);
    
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
    
    while(1);

    cleanup_platform();
    return 0;
}
