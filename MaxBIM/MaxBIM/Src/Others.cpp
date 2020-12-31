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

	char	substr [10];
	char	*token;

	
	// 프로젝트 내 레이어 이름을 전부 읽어옴
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	for (xx = 1; xx <= nLayers && err == NoError ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			// 레이어 이름 추출
			//sprintf (buffer, "%d/%d : %s\n", xx, nLayers, attrib.layer.head.name);

			// 레이어 이름을 "-" 문자 기준으로 쪼개기
			//token = strtok (attrib.layer.head.name, "-");
			//while (token != NULL) {
			//	sprintf (substr, "%s", token);
			//	ACAPI_WriteReport (substr, true);
			//	token = strtok (NULL, "-");
			//}
		}
		if (err == APIERR_DELETED)
			err = NoError;
	}

	// 레이어 이름의 체계를 분석하여 구조체 초기화
	// ...

	// 알고리즘 (초안)
	// 1. 공사 구분(필수) 토큰이 유효한지 검토 -> 공사 구분이 존재하는 것만 저장
		// 01-S 구조, 02-A 건축마감, 03-M 기계설비, 04-E 전기설비, 05-T 가설재, 06-F 가시설, 07-Q 물량합판, 08-L 조경, 09-C 토목, 10-K 건설장비
	// 2. 동 구분(선택) 토큰이 있는가, 없는가? -> 동이 존재하면 동 정보 저장함
		// 4글자 코드이어야 하며 층 구분 토큰이 아닌 경우 (숫자로만 되어 있으면 동 번호, 그 외에는?)
	// 3. 층 구분(필수) 토큰이 유효한지 검토 -> 층이 존재하는 것만 저장
		// 4글자 코드 (지하층, 지상층, 옥탑층)
	// 4. CJ구간 (Construction Joint) 토큰이 있는가, 없는가? -> CJ 구간이 존재하면 CJ 구간 정보 저장함
		// 숫자 코드(01~99)
	// 5. CJ속 시공순서 토큰이 있는가, 없는가?
		// 숫자 코드(01~99)
	// 6. 부재 및 객체 구분이 있는가, 없는가?
		// 공사 구분 토큰과 관련된 부재 종류 토큰이 있는지 확인

	// 문자열 분해 알고리즘
	/*
		1. 처음 2개의 토큰을 먼저 받는다.
			- 공사 구분 코드가 맞는지 확인한다. (01~10까지만 인식, 50(2D도면)이나 나머지는 무시)
			- 만약 맞다면, 구조체의 공사 구분 코드 flag를 set시킴*
		2. 다음 토큰을 받는다.
			- 4글자이면 동 구분 (0101~1504, SHOP, SECU 등) - 선택
			- 3글자이면 층 구분 (1B9~9B1, F01~F99, PH1~PH3) - 필수
		3. 동 구분이라면?
			- 기존에 저장된 동 구분 문자열이 없다면 list에 저장할 것
			- "코드 문자열" - "설명 문자열"
		4. 층 구분이라면?
			- 기존에 저장된 층 구분 문자열이 없다면 list에 저장할 것
			- "코드 문자열" - "설명 문자열"
		5. 다음 토큰을 받는다.
			- 2글자이면 CJ 구간 (01~99) : list에 저장할 것
			- 그 외에는 부재 및 객체 구분
		6. 다음 토큰을 받는다.
			- 2글자이면 CJ 속 시공순서 (01~99) : list에 저장할 것
			- 그 외에는 부재 및 객체 구분
		7. 부재 및 객체 구분 (필수)
			- list에 저장할 것
		
		정규 코드: 01-S-(0101)-9B1-(01)-(01)-WALL  단, 괄호 안은 선택사항
	*/

	// 다이얼로그 표시 알고리즘
	/*
		1. 공사 구분 코드 flag가 true인 것만 버튼 표시 + 모두 표시 버튼
		2. 동 구분 (있을 경우): 버튼 표시 + 모두 표시 버튼
		3. 층 구분: 버튼 표시 + (지하층, 지상층, 옥탑층 각자 모두 표시 버튼)
		4. CJ 구간 (있을 경우): 버튼 표시 + 모두 표시 버튼
		5. CJ 속 시공순서 (있을 경우): 버튼 표시 + 모두 표시 버튼
		6. 부재 및 객체 구분
	*/

	// 문자열 조합 알고리즘
	/*
		1. 공사 코드 flag를 읽어옴 (전부이면 모두 읽어옴)
		2. 동 코드를 읽어옴 (전부 가능, 코드 생략 있을 수 있음)
		3. 층 코드를 읽어옴 (전부 가능)
		4. CJ 구간 코드를 읽어옴 (전부 가능, 코드 생략 있을 수 있음)
		5. CJ 속 시공순서 코드를 읽어옴 (전부 가능, 코드 생략 있을 수 있음)
		6. 부재 및 객체 구분 코드를 읽어옴 (전부 가능)
	*/

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
