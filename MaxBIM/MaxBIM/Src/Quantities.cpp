#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Quantities.hpp"

using namespace quantitiesDG;

qElem	qElemInfo;

// 물량합판을 부착할 수 있는 팔레트를 띄움
GSErrCode	placeQuantityPlywood (void)
{
	GSErrCode	err = NoError;

	qElemInfo.dialogID = 0;

	if ((qElemInfo.dialogID == 0) || !DGIsDialogOpen (qElemInfo.dialogID)) {
		qElemInfo.dialogID = DGModelessInit (ACAPI_GetOwnResModule (), 32520, ACAPI_GetOwnResModule (), qElemDlgCallBack, (DGUserData) &qElemInfo, 1);
	}

	//if ((qElemInfo.dialogID != 0) || DGIsDialogOpen (qElemInfo.dialogID)) {
	//	DGModelessClose (qElemInfo.dialogID);
	//	qElemInfo.dialogID = 0;
	//}

	// ...

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// 팔레트에 대한 콜백 함수 1
static short DGCALLBACK qElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData /*msgData*/)
{
	qElem		*dlgData = (qElem *) userData;

	switch (message) {
		case DG_MSG_INIT:
	//		DGSetItemValLong (dialID, (short) (dlgData->circleBased ? Dial_TypeCylind : Dial_TypeRect), 1);
	//		DGSetItemValLong (dialID, Dial_RubberLine, (dlgData->rubberIsOn) ? 1 : 0);
	//		DGSetItemValLong (dialID, Dial_FeedBack, (dlgData->disableDefaultFeedback) ? 0 : 1);
	//		DGSetItemValLong (dialID, Dial_Sensitive, (dlgData->filterIsOn) ? 1 : 0);
	//		DGSetItemValDouble (dialID, Dial_SpaceValue, dlgData->defSpacing);
	//		DGSetFocus (dialID, DG_NO_ITEM);

			if (ACAPI_RegisterModelessWindow (dialID, PaletteAPIControlCallBack,
						API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
						API_PalEnabled_InteriorElevation + API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_3D + API_PalEnabled_Layout) != NoError)
				DBPrintf ("Test:: ACAPI_RegisterModelessWindow failed\n");
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case BUTTON_GEAR:
					// ...
					break;

				case BUTTON_RECTANGLE:
					// ...
					break;

				case BUTTON_WINDOW:
					// ...
					break;

				case DG_CLOSEBOX:
					return item;	// 이것은 DG_MSG_CLOSE 메시지와 같음
			}
			break;

		case DG_MSG_DOUBLECLICK:
			break;

		case DG_MSG_CLOSE:
			ACAPI_UnregisterModelessWindow (dlgData->dialogID);
			dlgData->dialogID = 0;
			break;
	}

	return 0;
}

// 팔레트에 대한 콜백 함수 2
static GSErrCode __ACENV_CALL	PaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
{
	if (referenceID == qElemInfo.dialogID) {
		switch (messageID) {
			case APIPalMsg_ClosePalette:		DGModelessClose (qElemInfo.dialogID);
												break;

			case APIPalMsg_HidePalette_Begin:	break;
			case APIPalMsg_HidePalette_End:		break;

			case APIPalMsg_DisableItems_Begin:	break;
			case APIPalMsg_DisableItems_End:	break;
			case APIPalMsg_OpenPalette:			break;
			default:							break;
		}
	}

	return NoError;
}