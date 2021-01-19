#ifndef	__LAYERS__
#define __LAYERS__

#include "MaxBIM.hpp"

using namespace std;

// 레이어 코드 체계
struct LayerNameSystem
{
	// 정규 코드 예시: 05-T-(0101)-9B1-(01)-(01)-COLU-(UFOM)  단, 괄호 안은 선택사항

	// 공사 구분 (필수)
	vector<string>	code_name;
	vector<string>	code_desc;
	bool	*code_state;
	short	*code_idx;

	// 동 구분 (선택)
	vector<string>	dong_name;
	vector<string>	dong_desc;
	bool	*dong_state;
	short	*dong_idx;
	bool	bDongAllShow;		// 모두 선택 버튼 보여주기

	// 층 구분 (필수)
	vector<string>	floor_name;
	vector<string>	floor_desc;
	bool	*floor_state;
	short	*floor_idx;

	// CJ 구간 (선택)
	vector<string>	CJ_name;
	bool	*CJ_state;
	short	*CJ_idx;
	bool	bCJAllShow;			// 모두 선택 버튼 보여주기

	// CJ 속 시공순서 (선택)
	vector<string>	orderInCJ_name;
	bool	*orderInCJ_state;
	short	*orderInCJ_idx;
	bool	bOrderInCJAllShow;	// 모두 선택 버튼 보여주기

	// 부재 구분 (필수)
	vector<string>	obj_name;
	vector<string>	obj_desc;
	vector<string>	obj_cat;
	bool	*obj_state;
	short	*obj_idx;

	// 객체 구분 (선택)
	vector<string>	subObj_name;
	vector<string>	subObj_desc;
	vector<string>	subObj_cat;
	bool	*subObj_state;
	short	*subObj_idx;
};

struct StatusOfLayerNameSystem
{
	// 공사 구분 (필수)
	bool	code_state [10];

	// 동 구분 (선택)
	bool	dong_state [2000];

	// 층 구분 (필수)
	bool	floor_state [200];

	// CJ 구간 (선택)
	bool	CJ_state [10];

	// CJ 속 시공순서 (선택)
	bool	orderInCJ_state [10];

	// 부재 구분 (필수)
	bool	obj_state [500];

	// 객체 구분 (선택)
	bool	subObj_state [500];
};

void		allocateMemory (void);			// 메모리 할당
void		deallocateMemory (void);		// 메모리 해제
GSErrCode	showLayersEasily (void);		// 레이어 쉽게 선택하기
short DGCALLBACK layerShowHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그 박스] 레이어 보여주기
GSErrCode	saveButtonStatus (void);		// 최근 버튼 상태 저장하기

#endif