#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Others.hpp"

using namespace othersDG;

// 레이어 쉽게 선택하기
GSErrCode	showLayersEasily (void)
{
	GSErrCode	err = NoError;
	
	API_Attribute	attrib;
	short			xx;
	short			nLayers;

	
	// 프로젝트 내 레이어 이름을 전부 읽어옴
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	for (xx = 1; xx <= nLayers && err == NoError ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			// 레이어 이름 추출
			//sprintf (buffer, "%d/%d : %s - flag: %d\n", xx, nLayers, attrib.layer.head.name, attrib.layer.head.flags);
		}
		if (err == APIERR_DELETED)
			err = NoError;
	}

	// 레이어 이름의 체계를 분석하여 구조체 초기화
	// ...

	// 다이얼로그 창 열기
	// ...


	// 참조할 코드
	// 레이어 보이기/숨김 설정 (APILay_Hidden 이 있으면 숨김, 없으면 보임)
	// 1. 특정 이름으로 레이어 속성을 찾은 다음 (ACAPI_Attribute_Search)
	// 2. 다이얼로그의 입력 값에 따라 숨김/보임 옵션을 부여하여 속성값 변경 (ACAPI_Attribute_Modify)
			
	// 전부 표시하기
	//if ((attrib.layer.head.flags & APILay_Hidden) == true) {
	//	attrib.layer.head.flags ^= APILay_Hidden;
	//	ACAPI_Attribute_Modify (&attrib, NULL);
	//}

	// 전부 숨기기
	//if ((attrib.layer.head.flags & APILay_Hidden) == false) {
	//	attrib.layer.head.flags |= APILay_Hidden;
	//	ACAPI_Attribute_Modify (&attrib, NULL);
	//}

	return err;
}

//// [다이얼로그 박스] MaxBIM 애드온 정보
//short DGCALLBACK aboutHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
//{
//	short	result;
//	short	itmIdx;
//	short	itmPosX, itmPosY;
//
//	switch (message) {
//		case DG_MSG_INIT:
//			// 다이얼로그 타이틀
//			DGSetDialogTitle (dialogID, "MaxBIM 애드온 정보");
//
//			// 확인 버튼
//			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 200, 260, 100, 25);
//			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
//			DGSetItemText (dialogID, DG_OK, "확인");
//			DGShowItem (dialogID, DG_OK);
//
//			itmPosX = 20;
//			itmPosY = 20;
//
//			// 라벨: 버전 (최근 배포일)
//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 230, 23);
//			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
//			DGSetItemText (dialogID, itmIdx, "버전 6.0 (배포일: 2020.12.14)");
//			DGShowItem (dialogID, itmIdx);
//			itmPosY += 30;
//
//			// 라벨: 작성자 (이메일)
//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 230, 23);
//			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
//			DGSetItemText (dialogID, itmIdx, "작성자: 정순범 (canvas84@naver.com)");
//			DGShowItem (dialogID, itmIdx);
//			itmPosY += 30;
//
//			// 라벨: 프로그램 기능
//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 230, 23*7);
//			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
//			DGSetItemText (dialogID, itmIdx, "1. 유로폼 배치\n  - 벽에 유로폼 배치\n  - 슬래브 하부에 유로폼 배치\n  - 보에 유로폼 배치\n  - 기둥에 유로폼 배치\n\n2. 유틸리티\n  - 레이어 쉽게 선택하기 (개발중)");
//			DGShowItem (dialogID, itmIdx);
//
//			break;
//		
//		case DG_MSG_CHANGE:
//			break;
//
//		case DG_MSG_CLICK:
//			switch (item) {
//				case DG_OK:
//					break;
//			}
//		case DG_MSG_CLOSE:
//			switch (item) {
//				case DG_CLOSEBOX:
//					break;
//			}
//	}
//
//	result = item;
//
//	return	result;
//}
