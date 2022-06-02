#ifndef	__LAYERS__
#define __LAYERS__

#include "MaxBIM.hpp"

using namespace std;

namespace layersDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_layerMakeDG {
		BUTTON_CODE	= 3,
		BUTTON_DONG,
		BUTTON_FLOOR,
		BUTTON_CAST,
		BUTTON_CJ,
		BUTTON_ORDER,
		BUTTON_OBJ,
		BUTTON_PRODUCT_SITE,
		BUTTON_PRODUCT_NUM,

		SEPARATOR_1,
		SEPARATOR_2,
		SEPARATOR_3,
		SEPARATOR_4,
		SEPARATOR_5,
		SEPARATOR_6,
		SEPARATOR_7,
		SEPARATOR_8,
		SEPARATOR_9,

		CHECKBOX_PRODUCT_SITE_NUM
	};
}

// 레이어 코드 체계
struct LayerNameSystem
{
	// 기본 정규 코드 형식: 일련번호 - 공사구분 - 동구분 - 층구분 - 타설번호 - CJ - CJ속시공순서 - 부재구분
	// 확장 정규 코드 형식: 일련번호 - 공사구분 - 동구분 - 층구분 - 타설번호 - CJ - CJ속시공순서 - 부재구분 - 제작처구분 - 제작번호
	/*
	 일련번호-공사구분: 01-S (구조), 02-A (건축마감), 03-M (기계설비), 04-E (전기설비), 05-T (가설재), 06-F (가시설), 07-Q (물량합판), 08-L (조경), 09-C (토목), 10-K (건설장비), 50-S,A,M,E,T,F,Q,L,C,K (각 공사별 2D 도면)
	 동 구분: 101~1599동 (0101~1599), SHOP (근린생활시설), SECU (경비실) ... (단, 구분이 없으면 0000)
	 층 구분: 1B9~9B1 (지하9층~1층), F01~F99 (지상1층~99층), PH1~PH9 (옥탑1층~9층) ...
	 타설번호: 01~99 (단, 구분이 없으면 01)
	 CJ: 01~99 (단, 구분이 없으면 01)
	 CJ 속 시공순서: 01~99 (단, 구분이 없으면 01)
	 부재구분: WALL (벽), COLU (기둥) ...
	 제작처구분: 현장제작, 공장제작
	 제작번호: 001~999
	 */
	// 예시(기본): 05-T-0000-F01-01-01-01-WALL
	// 예시(확장): 05-T-0000-F01-01-01-01-WALL-현장제작-001

	bool	extendedLayer;		// 확장이면 true, 기본이면 false

	// 1. 공사 구분 (필수)
	vector<string>	code_name;	// 코드 이름
	vector<string>	code_desc;	// 코드 설명
	bool	*code_state;		// On/Off 상태
	short	*code_idx;			// 다이얼로그 상의 인덱스
	bool	bCodeAllShow;		// 모두 선택 버튼 보여주기

	// 2. 동 구분 (필수)
	vector<string>	dong_name;
	vector<string>	dong_desc;
	bool	*dong_state;
	short	*dong_idx;
	bool	bDongAllShow;		// 모두 선택 버튼 보여주기

	// 3. 층 구분 (필수)
	vector<string>	floor_name;
	vector<string>	floor_desc;
	bool	*floor_state;
	short	*floor_idx;
	bool	bFloorAllShow;		// 모두 선택 버튼 보여주기

	// 4. 타설번호 (필수)
	vector<string>	cast_name;
	bool	*cast_state;
	short	*cast_idx;
	bool	bCastAllShow;		// 모두 선택 버튼 보여주기

	// 5. CJ 구간 (필수)
	vector<string>	CJ_name;
	bool	*CJ_state;
	short	*CJ_idx;
	bool	bCJAllShow;			// 모두 선택 버튼 보여주기

	// 6. CJ 속 시공순서 (필수)
	vector<string>	orderInCJ_name;
	bool	*orderInCJ_state;
	short	*orderInCJ_idx;
	bool	bOrderInCJAllShow;	// 모두 선택 버튼 보여주기

	// 7. 부재 구분 (필수)
	vector<string>	obj_name;
	vector<string>	obj_desc;
	vector<string>	obj_cat;
	bool	*obj_state;
	short	*obj_idx;

	// 8. 제작처 구분 (선택)
	vector<string>	productSite_name;
	bool	*productSite_state;
	short	*productSite_idx;
	bool	bProductSiteAllShow;// 모드 선택 버튼 보여주기

	// 9. 제작 번호 (선택)
	vector<string>	productNum_name;
	bool	*productNum_state;
	short	*productNum_idx;
	bool	bProductNumAllShow;	// 모드 선택 버튼 보여주기
};

struct StatusOfLayerNameSystem
{
	// 공사 구분 (필수)
	bool	code_state [25];

	// 동 구분 (필수)
	bool	dong_state [2000];

	// 층 구분 (필수)
	bool	floor_state [200];

	// 타설번호 (필수)
	bool	cast_state [120];

	// CJ 구간 (필수)
	bool	CJ_state [120];

	// CJ 속 시공순서 (필수)
	bool	orderInCJ_state [120];

	// 부재 구분 (필수)
	bool	obj_state [500];

	// 제작처 구분 (선택)
	bool	productSite_state [10];

	// 제작 번호 (선택)
	bool	productNum_state [1000];
};

void		allocateMemory (LayerNameSystem *layerInfo);		// 메모리 할당
void		deallocateMemory (LayerNameSystem *layerInfo);		// 메모리 해제
bool		isFullLayer (LayerNameSystem *layerInfo);			// 레이어 필드 코드에 누락이 없는가?
bool		importLayerInfo (LayerNameSystem *layerInfo);		// 레이어 정보 파일 가져오기

GSErrCode	showLayersEasily (void);		// 레이어 쉽게 선택하기
short DGCALLBACK layerShowHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그 박스] 레이어 쉽게 선택하기
GSErrCode	saveButtonStatus_show (void);	// 최근 버튼 상태 저장하기 (레이어 쉽게 보여주기)

GSErrCode	makeLayersEasily (void);		// 레이어 쉽게 만들기
short DGCALLBACK layerMakeHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그 박스] 레이어 쉽게 만들기
short DGCALLBACK layerMakeHandler_2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [다이얼로그 박스] 레이어 쉽게 만들기 2차

GSErrCode	assignLayerEasily (void);		// 레이어 쉽게 지정하기
short DGCALLBACK layerAssignHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [다이얼로그 박스] 레이어 쉽게 지정하기
short DGCALLBACK layerAssignHandler_2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [다이얼로그 박스] 레이어 쉽게 지정하기 2차
GSErrCode	saveButtonStatus_assign (void);	// 최근 버튼 상태 저장하기 (레이어 쉽게 지정하기)

GSErrCode	inspectLayerNames (void);		// 레이어 이름 검사하기
short DGCALLBACK layerNameInspectionHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [다이얼로그 박스] 레이어 이름 검사하기

#endif