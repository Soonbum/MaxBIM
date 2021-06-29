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

	return	err;
}

// 팔레트에 대한 콜백 함수 1
static short DGCALLBACK qElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData /*msgData*/)
{
	qElem		*dlgData = (qElem *) userData;
	API_UCCallbackType	ucb;
	GSErrCode	err = NoError;
	GSErrCode	inputErr;
	bool	regenerate = true;

	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("물량합판(핫스팟축소).gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				tempStr [256];
	double				horLen, verLen;
	bool				bValid;

	// 라인 입력 변수
	API_GetPointType	pointInfo;
	double				dx, dy, dz;

	API_Coord3D		p1, p2, p3, p4, p5;

	API_StoryInfo	storyInfo;

	
	err = ACAPI_CallUndoableCommand ("물량합판 부착하기", [&] () -> GSErrCode {
		switch (message) {
			case DG_MSG_INIT:
				// 라벨
				DGSetItemText (dialID, LABEL_EXPLANATION, "3점 클릭: 직사각형, 5점 클릭: 창문형");

				// 레이어
				BNZeroMemory (&ucb, sizeof (ucb));
				ucb.dialogID = dialID;
				ucb.type	 = APIUserControlType_Layer;
				ucb.itemID	 = USERCONTROL_PLYWOOD_TYPE;
				ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
				DGSetItemValLong (dialID, USERCONTROL_PLYWOOD_TYPE, 1);

				// 팝업 컨트롤
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "보");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "기둥(독립)");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "기둥(벽체)");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "벽체(내벽)");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "벽체(외벽)");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "벽체(합벽)");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "벽체(파라펫)");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "벽체(방수턱)");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "슬래브(기초)");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "슬래브(RC)");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "슬래브(데크)");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "슬래브(램프)");
				DGPopUpInsertItem (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_PLYWOOD_TYPE, DG_POPUP_BOTTOM, "계단");

				// 버튼
				DGSetItemText (dialID, BUTTON_DRAW_RECT, "물량합판 그리기\n(직사각형)");
				DGSetItemText (dialID, BUTTON_DRAW_WINDOW, "물량합판 그리기\n(창문형)");

				// 층 정보 저장
				BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
				ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
				qElemInfo.floorInd = storyInfo.actStory;
				BMKillHandle ((GSHandle *) &storyInfo.data);

				// 레이어 정보 저장
				qElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_PLYWOOD_TYPE);

				// 분류 정보 저장
				if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "보") == 0) {
					strcpy (qElemInfo.m_type, "보");
					qElemInfo.panel_mat = 78;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "기둥(독립)") == 0) {
					strcpy (qElemInfo.m_type, "기둥(독립)");
					qElemInfo.panel_mat = 20;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "기둥(벽체)") == 0) {
					strcpy (qElemInfo.m_type, "기둥(벽체)");
					qElemInfo.panel_mat = 77;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "벽체(내벽)") == 0) {
					strcpy (qElemInfo.m_type, "벽체(내벽)");
					qElemInfo.panel_mat = 75;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "벽체(외벽)") == 0) {
					strcpy (qElemInfo.m_type, "벽체(외벽)");
					qElemInfo.panel_mat = 76;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "벽체(합벽)") == 0) {
					strcpy (qElemInfo.m_type, "벽체(합벽)");
					qElemInfo.panel_mat = 72;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "벽체(파라펫)") == 0) {
					strcpy (qElemInfo.m_type, "벽체(파라펫)");
					qElemInfo.panel_mat = 32;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "벽체(방수턱)") == 0) {
					strcpy (qElemInfo.m_type, "벽체(방수턱)");
					qElemInfo.panel_mat = 12;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "슬래브(기초)") == 0) {
					strcpy (qElemInfo.m_type, "스라브(기초)");
					qElemInfo.panel_mat = 66;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "슬래브(RC)") == 0) {
					strcpy (qElemInfo.m_type, "스라브(RC)");
					qElemInfo.panel_mat = 100;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "슬래브(데크)") == 0) {
					strcpy (qElemInfo.m_type, "스라브(데크)");
					qElemInfo.panel_mat = 99;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "슬래브(램프)") == 0) {
					strcpy (qElemInfo.m_type, "스라브(램프)");
					qElemInfo.panel_mat = 3;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "계단") == 0) {
					strcpy (qElemInfo.m_type, "계단");
					qElemInfo.panel_mat = 73;
				}
 
				if (ACAPI_RegisterModelessWindow (dialID, PaletteAPIControlCallBack,
							API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
							API_PalEnabled_InteriorElevation + API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_3D + API_PalEnabled_Layout) != NoError)
					DBPrintf ("Test:: ACAPI_RegisterModelessWindow failed\n");

				break;

			case DG_MSG_CHANGE:

				switch (item) {
					case USERCONTROL_PLYWOOD_TYPE:
						qElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_PLYWOOD_TYPE);
						break;

					case POPUP_PLYWOOD_TYPE:
						if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "보") == 0) {
							strcpy (qElemInfo.m_type, "보");
							qElemInfo.panel_mat = 78;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "기둥(독립)") == 0) {
							strcpy (qElemInfo.m_type, "기둥(독립)");
							qElemInfo.panel_mat = 20;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "기둥(벽체)") == 0) {
							strcpy (qElemInfo.m_type, "기둥(벽체)");
							qElemInfo.panel_mat = 77;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "벽체(내벽)") == 0) {
							strcpy (qElemInfo.m_type, "벽체(내벽)");
							qElemInfo.panel_mat = 75;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "벽체(외벽)") == 0) {
							strcpy (qElemInfo.m_type, "벽체(외벽)");
							qElemInfo.panel_mat = 76;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "벽체(합벽)") == 0) {
							strcpy (qElemInfo.m_type, "벽체(합벽)");
							qElemInfo.panel_mat = 72;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "벽체(파라펫)") == 0) {
							strcpy (qElemInfo.m_type, "벽체(파라펫)");
							qElemInfo.panel_mat = 32;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "벽체(방수턱)") == 0) {
							strcpy (qElemInfo.m_type, "벽체(방수턱)");
							qElemInfo.panel_mat = 12;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "슬래브(기초)") == 0) {
							strcpy (qElemInfo.m_type, "스라브(기초)");
							qElemInfo.panel_mat = 66;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "슬래브(RC)") == 0) {
							strcpy (qElemInfo.m_type, "스라브(RC)");
							qElemInfo.panel_mat = 100;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "슬래브(데크)") == 0) {
							strcpy (qElemInfo.m_type, "스라브(데크)");
							qElemInfo.panel_mat = 99;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "슬래브(램프)") == 0) {
							strcpy (qElemInfo.m_type, "스라브(램프)");
							qElemInfo.panel_mat = 3;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_PLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_PLYWOOD_TYPE))).ToCStr ().Get (), "계단") == 0) {
							strcpy (qElemInfo.m_type, "계단");
							qElemInfo.panel_mat = 73;
						}

						break;
				}

				break;

			case DG_MSG_CLICK:
				switch (item) {
					case BUTTON_DRAW_RECT:
						do {
							BNZeroMemory (&pointInfo, sizeof (API_GetPointType));

							CHCopyC ("다각형의 1번째 노드(좌하단)를 클릭하십시오.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p1.x = pointInfo.pos.x;
							p1.y = pointInfo.pos.y;
							p1.z = pointInfo.pos.z;

							CHCopyC ("다각형의 2번째 노드(우하단)를 클릭하십시오.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p2.x = pointInfo.pos.x;
							p2.y = pointInfo.pos.y;
							p2.z = pointInfo.pos.z;

							CHCopyC ("다각형의 3번째 노드(우상단)를 클릭하십시오.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p3.x = pointInfo.pos.x;
							p3.y = pointInfo.pos.y;
							p3.z = pointInfo.pos.z;

							// 객체 로드
							BNZeroMemory (&elem, sizeof (API_Element));
							BNZeroMemory (&memo, sizeof (API_ElementMemo));
							BNZeroMemory (&libPart, sizeof (libPart));
							GS::ucscpy (libPart.file_UName, gsmName);
							err = ACAPI_LibPart_Search (&libPart, false);
							if (libPart.location != NULL)
								delete libPart.location;
							if (err != NoError)
								break;

							ACAPI_LibPart_Get (&libPart);

							elem.header.typeID = API_ObjectID;
							elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

							ACAPI_Element_GetDefaults (&elem, &memo);
							ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

							// 라이브러리의 파라미터 값 입력 (공통)
							elem.header.floorInd = qElemInfo.floorInd;
							elem.object.libInd = libPart.index;
							elem.object.xRatio = aParam;
							elem.object.yRatio = bParam;
							elem.header.layer = qElemInfo.layerInd;

							setParameterByName (&memo, "m_type", qElemInfo.m_type);
							setParameterByName (&memo, "PANEL_MAT", qElemInfo.panel_mat);

							// 합판두께 (문자열)
							strcpy (tempStr, "12");		// 12mm
							setParameterByName (&memo, "m_size", tempStr);

							// 품명
							strcpy (tempStr, "합판면적");
							setParameterByName (&memo, "m_size2", tempStr);

							dx = p2.x - p1.x;
							dy = p2.y - p1.y;

							elem.object.angle = atan2 (dy, dx);
							elem.object.pos.x = p1.x;
							elem.object.pos.y = p1.y;
							elem.object.level = p1.z;

							// 가로길이
							horLen = GetDistance (p1, p2);
							setParameterByName (&memo, "NO_WD", horLen);
							elem.object.xRatio = horLen;

							// 세로길이
							verLen = GetDistance (p2, p3);
							setParameterByName (&memo, "no_lg1", verLen);
							elem.object.yRatio = verLen;

							bValid = false;
							strcpy (tempStr, "벽에 세우기");	// 기본값

							// 설치위치
							strcpy (tempStr, "비규격");
							setParameterByName (&memo, "m_size1", tempStr);

							// p1, p2, p3 점의 고도가 모두 같을 때
							if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
								double angle1, angle2;

								dx = p2.x - p1.x;
								dy = p2.y - p1.y;
								angle1 = RadToDegree (atan2 (dy, dx));

								dx = p3.x - p2.x;
								dy = p3.y - p2.y;
								angle2 = RadToDegree (atan2 (dy, dx));

								// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 큼
								if (abs (angle2 - angle1 - 90) < EPS) {
									strcpy (tempStr, "바닥덮기");
									bValid = true;
								}

								// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 작음
								if (abs (angle1 - angle2 - 90) < EPS) {
									strcpy (tempStr, "바닥깔기");
									moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
									bValid = true;
								}
							// p1, p2 점의 고도가 같고 p3 점의 고도가 다를 때
							} else {
								// p2와 p3의 x, y 좌표는 같음
								if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
									// ???
									strcpy (tempStr, "벽에 세우기");
									setParameterByName (&memo, "ZZYZX", verLen);
									bValid = true;
								
								// p2와 p3의 x, y 좌표는 다름
								} else {
									strcpy (tempStr, "경사설치");
									bValid = true;

									// 설치각도: asin ((p3.z - p2.z) / (p3와 p2 간의 거리))
									// 설치각도: acos ((p3와 p2 간의 평면 상의 거리) / (p3와 p2 간의 거리))
									// 설치각도: atan2 ((p3.z - p2.z) / (p3와 p2 간의 평면 상의 거리))
									dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
									dy = abs (p3.z - p2.z);
									dz = verLen;
									setParameterByName (&memo, "cons_ang", DegreeToRad (180.0) - acos (dx/dz));
								}
							}
							setParameterByName (&memo, "CONS_DR", tempStr);

							// 객체 배치
							if ((horLen > EPS) && (verLen > EPS) && (bValid == true)) {
								ACAPI_Element_Create (&elem, &memo);
							}

							ACAPI_DisposeElemMemoHdls (&memo);

							// 화면 새로고침
							ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
							ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						} while (inputErr != APIERR_CANCEL);

						break;

					case BUTTON_DRAW_WINDOW:
						do {
							BNZeroMemory (&pointInfo, sizeof (API_GetPointType));

							CHCopyC ("다각형의 1번째 노드(좌하단)를 클릭하십시오.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p1.x = pointInfo.pos.x;
							p1.y = pointInfo.pos.y;
							p1.z = pointInfo.pos.z;

							CHCopyC ("다각형의 2번째 노드(우하단)를 클릭하십시오.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p2.x = pointInfo.pos.x;
							p2.y = pointInfo.pos.y;
							p2.z = pointInfo.pos.z;

							CHCopyC ("다각형의 3번째 노드(우상단)를 클릭하십시오.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p3.x = pointInfo.pos.x;
							p3.y = pointInfo.pos.y;
							p3.z = pointInfo.pos.z;

							CHCopyC ("다각형의 4번째 노드(개구부 좌하단)를 클릭하십시오.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p4.x = pointInfo.pos.x;
							p4.y = pointInfo.pos.y;
							p4.z = pointInfo.pos.z;

							CHCopyC ("다각형의 5번째 노드(개구부 우상단)를 클릭하십시오.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p5.x = pointInfo.pos.x;
							p5.y = pointInfo.pos.y;
							p5.z = pointInfo.pos.z;

							// 객체 로드
							BNZeroMemory (&elem, sizeof (API_Element));
							BNZeroMemory (&memo, sizeof (API_ElementMemo));
							BNZeroMemory (&libPart, sizeof (libPart));
							GS::ucscpy (libPart.file_UName, gsmName);
							err = ACAPI_LibPart_Search (&libPart, false);
							if (libPart.location != NULL)
								delete libPart.location;
							if (err != NoError)
								break;

							ACAPI_LibPart_Get (&libPart);

							elem.header.typeID = API_ObjectID;
							elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

							ACAPI_Element_GetDefaults (&elem, &memo);
							ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

							// 라이브러리의 파라미터 값 입력 (공통)
							elem.header.floorInd = qElemInfo.floorInd;
							elem.object.libInd = libPart.index;
							elem.object.xRatio = aParam;
							elem.object.yRatio = bParam;
							elem.header.layer = qElemInfo.layerInd;

							setParameterByName (&memo, "m_type", qElemInfo.m_type);
							setParameterByName (&memo, "PANEL_MAT", qElemInfo.panel_mat);

							// 합판두께 (문자열)
							strcpy (tempStr, "12");		// 12mm
							setParameterByName (&memo, "m_size", tempStr);

							// 품명
							strcpy (tempStr, "합판면적");
							setParameterByName (&memo, "m_size2", tempStr);

							dx = p2.x - p1.x;
							dy = p2.y - p1.y;

							elem.object.angle = atan2 (dy, dx);
							elem.object.pos.x = p1.x;
							elem.object.pos.y = p1.y;
							elem.object.level = p1.z;

							// 가로길이
							horLen = GetDistance (p1, p2);
							setParameterByName (&memo, "NO_WD", horLen);
							elem.object.xRatio = horLen;

							// 세로길이
							verLen = GetDistance (p2, p3);
							setParameterByName (&memo, "no_lg1", verLen);
							elem.object.yRatio = verLen;

							bValid = false;
							strcpy (tempStr, "벽에 세우기");	// 기본값

							// 설치위치
							strcpy (tempStr, "창문형");
							setParameterByName (&memo, "m_size1", tempStr);

							// 설치방향
							API_Coord3D		origin;
							API_Coord3D		target_before, target_after;
								
							// p1, p2, p3 점의 고도가 모두 같을 때
							if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
								double angle1, angle2;

								dx = p2.x - p1.x;
								dy = p2.y - p1.y;
								angle1 = RadToDegree (atan2 (dy, dx));

								dx = p3.x - p2.x;
								dy = p3.y - p2.y;
								angle2 = RadToDegree (atan2 (dy, dx));

								// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 큼
								if (abs (angle2 - angle1 - 90) < EPS) {
									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", abs (origin.x - target_after.x));				// 폭1: vw1
									setParameterByName (&memo, "vh1", abs (origin.y - target_after.y));				// 높이1: vh1

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", abs (origin.x + horLen - target_after.x));	// 폭2: vw2
									setParameterByName (&memo, "vh2", abs (origin.y + verLen - target_after.y));	// 높이2: vh2

									strcpy (tempStr, "바닥덮기");
									bValid = true;
								}

								// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 작음
								if (abs (angle1 - angle2 - 90) < EPS) {
									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", abs (origin.x - target_after.x));				// 폭1: vw1
									setParameterByName (&memo, "vh2", abs (origin.y - target_after.y));				// 높이2: vh2

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", abs (origin.x + horLen - target_after.x));	// 폭2: vw2
									setParameterByName (&memo, "vh1", verLen - abs (origin.y - target_after.y));	// 높이1: vh1

									strcpy (tempStr, "바닥깔기");
									moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
									bValid = true;
								}
								
							// p1, p2 점의 고도가 같고 p3 점의 고도가 다를 때
							} else {
								// ???
								if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
									// p2와 p3의 x, y 좌표는 같음
									strcpy (tempStr, "벽에 세우기");
									setParameterByName (&memo, "ZZYZX", verLen);
									bValid = true;

									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", target_after.x - origin.x);						// 폭1: vw1
									setParameterByName (&memo, "vh1", target_after.z - origin.z);						// 높이1: vh1

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", origin.x + horLen - target_after.x);				// 폭2: vw2
									setParameterByName (&memo, "vh2", origin.z + verLen - target_after.z);				// 높이2: vh2
								}
							}
							setParameterByName (&memo, "CONS_DR", tempStr);

							// 객체 배치
							if ((horLen > EPS) && (verLen > EPS) && (bValid == true)) {
								ACAPI_Element_Create (&elem, &memo);
							}

							ACAPI_DisposeElemMemoHdls (&memo);

							// 화면 새로고침
							ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
							ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						} while (inputErr != APIERR_CANCEL);

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

		return err;
	});

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