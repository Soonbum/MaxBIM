#ifndef	__EXPORT__
#define __EXPORT__

#include "MaxBIM.hpp"

using namespace std;

namespace exportDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_exportDG {
		LABEL_DIST_BTW_COLUMN		= 3,
		EDITCONTROL_DIST_BTW_COLUMN
	};

	enum	layerTypeEnum {
		UNDEFINED,	// 정의되지 않음
		WALL,	// 벽 -> 면의 앞뒷면 캡쳐할 것
		SLAB,	// 슬래브 -> 면의 아래면 캡쳐할 것
		COLU,	// 기둥 -> 네측면 캡쳐할 것
		BEAM,	// 보 -> 측면, 하부 캡쳐할 것
		WLBM	// 눈썹보 -> 측면, 하부 캡쳐할 것
	};

	enum	filterSelectionDG {
		BUTTON_ALL_SEL = 3,
		BUTTON_ALL_UNSEL,
		CHECKBOX_INCLUDE_UNKNOWN_OBJECT
	};
}

// 개별 기둥의 정보
struct ColumnInfo
{
	short	floorInd;	// 층 인덱스
	double	posX;		// 중심점 X좌표
	double	posY;		// 중심점 Y좌표
	double	horLen;		// 가로 길이
	double	verLen;		// 세로 길이
	double	height;		// 높이

	short	iHor;		// 가로 순번
	short	iVer;		// 세로 순번
};

// 기둥의 위치 정보
struct ColumnPos
{
	ColumnInfo	columns [5000];		// 기둥 정보
	short	nColumns;				// 기둥 개수

	// 주열 정보
	double	node_hor [100][100];	// 층별 가로 방향 기둥 좌표 (X) : dim1 - 층, dim2 - 노드 X좌표
	short	nNodes_hor [100];		// 층별 가로 방향 기둥 개수
	double	node_ver [100][100];	// 층별 세로 방향 기둥 좌표 (Y) : dim1 - 층, dim2 - 노드 Y좌표
	short	nNodes_ver [100];		// 층별 세로 방향 기둥 개수

	short	nStories;				// 층 수를 의미함, = storyInfo.lastStory - storyInfo.firstStory + (storyInfo.skipNullFloor) * 1 (0층을 생략했다면 +1)
	short	firstStory;				// 최하위 층 인덱스 (예: 지하 4층인 경우, -4)
	short	lastStory;				// 최상위 층 인덱스 (예: 지상 35층인 경우, 34)
};

// 선택한 부재들의 요약 정보 (new)
class SummaryOfObjectInfo
{
public:
	SummaryOfObjectInfo ();		// 생성자

public:
	// 키
	vector<string>	nameKey;		// 객체를 구분할 수 있는 값(문자열)이 들어 있는 변수 이름 (예: sup_type)
	vector<string>	nameVal;		// 객체의 이름 (예: KS프로파일)

	vector<short>	nInfo;			// 표시할 정보 필드 개수

	// 객체별 정보 (Object 타입)
	vector<string>	var1name;					// 변수1 이름 (예: nom)
	vector<string>	var1desc;					// 변수1 이름에 대한 설명 (예: 규격)
	vector<short>	var1showFlag;				// 변수1 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var1value;			// 변수1 이름에 대한 값
	vector<API_AddParID>	var1type;			// 변수1 이름에 대한 값의 타입

	vector<string>	var2name;					// 변수2 이름 (예: nom)
	vector<string>	var2desc;					// 변수2 이름에 대한 설명 (예: 규격)
	vector<short>	var2showFlag;				// 변수2 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var2value;			// 변수2 이름에 대한 값
	vector<API_AddParID>	var2type;			// 변수2 이름에 대한 값의 타입
	
	vector<string>	var3name;					// 변수3 이름 (예: nom)
	vector<string>	var3desc;					// 변수3 이름에 대한 설명 (예: 규격)
	vector<short>	var3showFlag;				// 변수3 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var3value;			// 변수3 이름에 대한 값
	vector<API_AddParID>	var3type;			// 변수3 이름에 대한 값의 타입

	vector<string>	var4name;					// 변수4 이름 (예: nom)
	vector<string>	var4desc;					// 변수4 이름에 대한 설명 (예: 규격)
	vector<short>	var4showFlag;				// 변수4 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var4value;			// 변수4 이름에 대한 값
	vector<API_AddParID>	var4type;			// 변수4 이름에 대한 값의 타입

	vector<string>	var5name;					// 변수5 이름 (예: nom)
	vector<string>	var5desc;					// 변수5 이름에 대한 설명 (예: 규격)
	vector<short>	var5showFlag;				// 변수5 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var5value;			// 변수5 이름에 대한 값
	vector<API_AddParID>	var5type;			// 변수5 이름에 대한 값의 타입

	vector<string>	var6name;					// 변수6 이름 (예: nom)
	vector<string>	var6desc;					// 변수6 이름에 대한 설명 (예: 규격)
	vector<short>	var6showFlag;				// 변수6 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var6value;			// 변수6 이름에 대한 값
	vector<API_AddParID>	var6type;			// 변수6 이름에 대한 값의 타입

	vector<string>	var7name;					// 변수7 이름 (예: nom)
	vector<string>	var7desc;					// 변수7 이름에 대한 설명 (예: 규격)
	vector<short>	var7showFlag;				// 변수7 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var7value;			// 변수7 이름에 대한 값
	vector<API_AddParID>	var7type;			// 변수7 이름에 대한 값의 타입

	vector<string>	var8name;					// 변수8 이름 (예: nom)
	vector<string>	var8desc;					// 변수8 이름에 대한 설명 (예: 규격)
	vector<short>	var8showFlag;				// 변수8 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var8value;			// 변수8 이름에 대한 값
	vector<API_AddParID>	var8type;			// 변수8 이름에 대한 값의 타입

	vector<string>	var9name;					// 변수9 이름 (예: nom)
	vector<string>	var9desc;					// 변수9 이름에 대한 설명 (예: 규격)
	vector<short>	var9showFlag;				// 변수9 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var9value;			// 변수9 이름에 대한 값
	vector<API_AddParID>	var9type;			// 변수9 이름에 대한 값의 타입

	vector<string>	var10name;					// 변수10 이름 (예: nom)
	vector<string>	var10desc;					// 변수10 이름에 대한 설명 (예: 규격)
	vector<short>	var10showFlag;				// 변수10 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var10value;			// 변수10 이름에 대한 값
	vector<API_AddParID>	var10type;			// 변수10 이름에 대한 값의 타입

	vector<string>	var11name;					// 변수11 이름 (예: nom)
	vector<string>	var11desc;					// 변수11 이름에 대한 설명 (예: 규격)
	vector<short>	var11showFlag;				// 변수11 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var11value;			// 변수11 이름에 대한 값
	vector<API_AddParID>	var11type;			// 변수11 이름에 대한 값의 타입

	vector<string>	var12name;					// 변수12 이름 (예: nom)
	vector<string>	var12desc;					// 변수12 이름에 대한 설명 (예: 규격)
	vector<short>	var12showFlag;				// 변수12 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var12value;			// 변수12 이름에 대한 값
	vector<API_AddParID>	var12type;			// 변수12 이름에 대한 값의 타입

	vector<string>	var13name;					// 변수13 이름 (예: nom)
	vector<string>	var13desc;					// 변수13 이름에 대한 설명 (예: 규격)
	vector<short>	var13showFlag;				// 변수13 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var13value;			// 변수13 이름에 대한 값
	vector<API_AddParID>	var13type;			// 변수13 이름에 대한 값의 타입

	vector<string>	var14name;					// 변수14 이름 (예: nom)
	vector<string>	var14desc;					// 변수14 이름에 대한 설명 (예: 규격)
	vector<short>	var14showFlag;				// 변수14 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var14value;			// 변수14 이름에 대한 값
	vector<API_AddParID>	var14type;			// 변수14 이름에 대한 값의 타입

	vector<string>	var15name;					// 변수15 이름 (예: nom)
	vector<string>	var15desc;					// 변수15 이름에 대한 설명 (예: 규격)
	vector<short>	var15showFlag;				// 변수15 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var15value;			// 변수15 이름에 대한 값
	vector<API_AddParID>	var15type;			// 변수15 이름에 대한 값의 타입

	vector<string>	var16name;					// 변수16 이름 (예: nom)
	vector<string>	var16desc;					// 변수16 이름에 대한 설명 (예: 규격)
	vector<short>	var16showFlag;				// 변수16 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var16value;			// 변수16 이름에 대한 값
	vector<API_AddParID>	var16type;			// 변수16 이름에 대한 값의 타입

	vector<string>	var17name;					// 변수17 이름 (예: nom)
	vector<string>	var17desc;					// 변수17 이름에 대한 설명 (예: 규격)
	vector<short>	var17showFlag;				// 변수17 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var17value;			// 변수17 이름에 대한 값
	vector<API_AddParID>	var17type;			// 변수17 이름에 대한 값의 타입

	vector<string>	var18name;					// 변수18 이름 (예: nom)
	vector<string>	var18desc;					// 변수18 이름에 대한 설명 (예: 규격)
	vector<short>	var18showFlag;				// 변수18 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var18value;			// 변수18 이름에 대한 값
	vector<API_AddParID>	var18type;			// 변수18 이름에 대한 값의 타입

	vector<string>	var19name;					// 변수19 이름 (예: nom)
	vector<string>	var19desc;					// 변수19 이름에 대한 설명 (예: 규격)
	vector<short>	var19showFlag;				// 변수19 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var19value;			// 변수19 이름에 대한 값
	vector<API_AddParID>	var19type;			// 변수19 이름에 대한 값의 타입

	vector<string>	var20name;					// 변수20 이름 (예: nom)
	vector<string>	var20desc;					// 변수20 이름에 대한 설명 (예: 규격)
	vector<short>	var20showFlag;				// 변수20 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var20value;			// 변수20 이름에 대한 값
	vector<API_AddParID>	var20type;			// 변수20 이름에 대한 값의 타입

	vector<string>	var21name;					// 변수21 이름 (예: nom)
	vector<string>	var21desc;					// 변수21 이름에 대한 설명 (예: 규격)
	vector<short>	var21showFlag;				// 변수21 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var21value;			// 변수21 이름에 대한 값
	vector<API_AddParID>	var21type;			// 변수21 이름에 대한 값의 타입

	vector<string>	var22name;					// 변수22 이름 (예: nom)
	vector<string>	var22desc;					// 변수22 이름에 대한 설명 (예: 규격)
	vector<short>	var22showFlag;				// 변수22 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var22value;			// 변수22 이름에 대한 값
	vector<API_AddParID>	var22type;			// 변수22 이름에 대한 값의 타입

	vector<string>	var23name;					// 변수23 이름 (예: nom)
	vector<string>	var23desc;					// 변수23 이름에 대한 설명 (예: 규격)
	vector<short>	var23showFlag;				// 변수23 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var23value;			// 변수23 이름에 대한 값
	vector<API_AddParID>	var23type;			// 변수23 이름에 대한 값의 타입

	vector<string>	var24name;					// 변수24 이름 (예: nom)
	vector<string>	var24desc;					// 변수24 이름에 대한 설명 (예: 규격)
	vector<short>	var24showFlag;				// 변수24 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var24value;			// 변수24 이름에 대한 값
	vector<API_AddParID>	var24type;			// 변수24 이름에 대한 값의 타입

	vector<string>	var25name;					// 변수25 이름 (예: nom)
	vector<string>	var25desc;					// 변수25 이름에 대한 설명 (예: 규격)
	vector<short>	var25showFlag;				// 변수25 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var25value;			// 변수25 이름에 대한 값
	vector<API_AddParID>	var25type;			// 변수25 이름에 대한 값의 타입

	vector<string>	var26name;					// 변수26 이름 (예: nom)
	vector<string>	var26desc;					// 변수26 이름에 대한 설명 (예: 규격)
	vector<short>	var26showFlag;				// 변수26 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var26value;			// 변수26 이름에 대한 값
	vector<API_AddParID>	var26type;			// 변수26 이름에 대한 값의 타입

	vector<string>	var27name;					// 변수27 이름 (예: nom)
	vector<string>	var27desc;					// 변수27 이름에 대한 설명 (예: 규격)
	vector<short>	var27showFlag;				// 변수27 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var27value;			// 변수27 이름에 대한 값
	vector<API_AddParID>	var27type;			// 변수27 이름에 대한 값의 타입

	vector<string>	var28name;					// 변수28 이름 (예: nom)
	vector<string>	var28desc;					// 변수28 이름에 대한 설명 (예: 규격)
	vector<short>	var28showFlag;				// 변수28 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var28value;			// 변수28 이름에 대한 값
	vector<API_AddParID>	var28type;			// 변수28 이름에 대한 값의 타입

	vector<string>	var29name;					// 변수29 이름 (예: nom)
	vector<string>	var29desc;					// 변수29 이름에 대한 설명 (예: 규격)
	vector<short>	var29showFlag;				// 변수29 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var29value;			// 변수29 이름에 대한 값
	vector<API_AddParID>	var29type;			// 변수29 이름에 대한 값의 타입

	vector<string>	var30name;					// 변수30 이름 (예: nom)
	vector<string>	var30desc;					// 변수30 이름에 대한 설명 (예: 규격)
	vector<short>	var30showFlag;				// 변수30 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var30value;			// 변수30 이름에 대한 값
	vector<API_AddParID>	var30type;			// 변수30 이름에 대한 값의 타입

	vector<string>	var31name;					// 변수31 이름 (예: nom)
	vector<string>	var31desc;					// 변수31 이름에 대한 설명 (예: 규격)
	vector<short>	var31showFlag;				// 변수31 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var31value;			// 변수31 이름에 대한 값
	vector<API_AddParID>	var31type;			// 변수31 이름에 대한 값의 타입

	vector<string>	var32name;					// 변수32 이름 (예: nom)
	vector<string>	var32desc;					// 변수32 이름에 대한 설명 (예: 규격)
	vector<short>	var32showFlag;				// 변수32 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var32value;			// 변수32 이름에 대한 값
	vector<API_AddParID>	var32type;			// 변수32 이름에 대한 값의 타입

	vector<string>	var33name;					// 변수33 이름 (예: nom)
	vector<string>	var33desc;					// 변수33 이름에 대한 설명 (예: 규격)
	vector<short>	var33showFlag;				// 변수33 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var33value;			// 변수33 이름에 대한 값
	vector<API_AddParID>	var33type;			// 변수33 이름에 대한 값의 타입

	vector<string>	var34name;					// 변수34 이름 (예: nom)
	vector<string>	var34desc;					// 변수34 이름에 대한 설명 (예: 규격)
	vector<short>	var34showFlag;				// 변수34 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var34value;			// 변수34 이름에 대한 값
	vector<API_AddParID>	var34type;			// 변수34 이름에 대한 값의 타입

	vector<string>	var35name;					// 변수35 이름 (예: nom)
	vector<string>	var35desc;					// 변수35 이름에 대한 설명 (예: 규격)
	vector<short>	var35showFlag;				// 변수35 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var35value;			// 변수35 이름에 대한 값
	vector<API_AddParID>	var35type;			// 변수35 이름에 대한 값의 타입

	vector<string>	var36name;					// 변수36 이름 (예: nom)
	vector<string>	var36desc;					// 변수36 이름에 대한 설명 (예: 규격)
	vector<short>	var36showFlag;				// 변수36 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var36value;			// 변수36 이름에 대한 값
	vector<API_AddParID>	var36type;			// 변수36 이름에 대한 값의 타입

	vector<string>	var37name;					// 변수37 이름 (예: nom)
	vector<string>	var37desc;					// 변수37 이름에 대한 설명 (예: 규격)
	vector<short>	var37showFlag;				// 변수37 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var37value;			// 변수37 이름에 대한 값
	vector<API_AddParID>	var37type;			// 변수37 이름에 대한 값의 타입

	vector<string>	var38name;					// 변수38 이름 (예: nom)
	vector<string>	var38desc;					// 변수38 이름에 대한 설명 (예: 규격)
	vector<short>	var38showFlag;				// 변수38 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var38value;			// 변수38 이름에 대한 값
	vector<API_AddParID>	var38type;			// 변수38 이름에 대한 값의 타입

	vector<string>	var39name;					// 변수39 이름 (예: nom)
	vector<string>	var39desc;					// 변수39 이름에 대한 설명 (예: 규격)
	vector<short>	var39showFlag;				// 변수39 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var39value;			// 변수39 이름에 대한 값
	vector<API_AddParID>	var39type;			// 변수39 이름에 대한 값의 타입

	vector<string>	var40name;					// 변수40 이름 (예: nom)
	vector<string>	var40desc;					// 변수40 이름에 대한 설명 (예: 규격)
	vector<short>	var40showFlag;				// 변수40 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var40value;			// 변수40 이름에 대한 값
	vector<API_AddParID>	var40type;			// 변수40 이름에 대한 값의 타입

	vector<string>	var41name;					// 변수41 이름 (예: nom)
	vector<string>	var41desc;					// 변수41 이름에 대한 설명 (예: 규격)
	vector<short>	var41showFlag;				// 변수41 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var41value;			// 변수41 이름에 대한 값
	vector<API_AddParID>	var41type;			// 변수41 이름에 대한 값의 타입

	vector<string>	var42name;					// 변수42 이름 (예: nom)
	vector<string>	var42desc;					// 변수42 이름에 대한 설명 (예: 규격)
	vector<short>	var42showFlag;				// 변수42 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var42value;			// 변수42 이름에 대한 값
	vector<API_AddParID>	var42type;			// 변수42 이름에 대한 값의 타입

	vector<string>	var43name;					// 변수43 이름 (예: nom)
	vector<string>	var43desc;					// 변수43 이름에 대한 설명 (예: 규격)
	vector<short>	var43showFlag;				// 변수43 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var43value;			// 변수43 이름에 대한 값
	vector<API_AddParID>	var43type;			// 변수43 이름에 대한 값의 타입

	vector<string>	var44name;					// 변수44 이름 (예: nom)
	vector<string>	var44desc;					// 변수44 이름에 대한 설명 (예: 규격)
	vector<short>	var44showFlag;				// 변수44 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var44value;			// 변수44 이름에 대한 값
	vector<API_AddParID>	var44type;			// 변수44 이름에 대한 값의 타입

	vector<string>	var45name;					// 변수45 이름 (예: nom)
	vector<string>	var45desc;					// 변수45 이름에 대한 설명 (예: 규격)
	vector<short>	var45showFlag;				// 변수45 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var45value;			// 변수45 이름에 대한 값
	vector<API_AddParID>	var45type;			// 변수45 이름에 대한 값의 타입

	vector<string>	var46name;					// 변수46 이름 (예: nom)
	vector<string>	var46desc;					// 변수46 이름에 대한 설명 (예: 규격)
	vector<short>	var46showFlag;				// 변수46 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var46value;			// 변수46 이름에 대한 값
	vector<API_AddParID>	var46type;			// 변수46 이름에 대한 값의 타입

	vector<string>	var47name;					// 변수47 이름 (예: nom)
	vector<string>	var47desc;					// 변수47 이름에 대한 설명 (예: 규격)
	vector<short>	var47showFlag;				// 변수47 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var47value;			// 변수47 이름에 대한 값
	vector<API_AddParID>	var47type;			// 변수47 이름에 대한 값의 타입

	vector<string>	var48name;					// 변수48 이름 (예: nom)
	vector<string>	var48desc;					// 변수48 이름에 대한 설명 (예: 규격)
	vector<short>	var48showFlag;				// 변수48 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var48value;			// 변수48 이름에 대한 값
	vector<API_AddParID>	var48type;			// 변수48 이름에 대한 값의 타입

	vector<string>	var49name;					// 변수49 이름 (예: nom)
	vector<string>	var49desc;					// 변수49 이름에 대한 설명 (예: 규격)
	vector<short>	var49showFlag;				// 변수49 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var49value;			// 변수49 이름에 대한 값
	vector<API_AddParID>	var49type;			// 변수49 이름에 대한 값의 타입

	vector<string>	var50name;					// 변수50 이름 (예: nom)
	vector<string>	var50desc;					// 변수50 이름에 대한 설명 (예: 규격)
	vector<short>	var50showFlag;				// 변수50 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	var50value;			// 변수50 이름에 대한 값
	vector<API_AddParID>	var50type;			// 변수50 이름에 대한 값의 타입

	vector<vector<short>>	combinationCount;	// 여러 변수들의 조합에 대한 개수

	vector<short>	nCounts;					// 서로 다른 변수 조합 개수
	short nKnownObjects;						// 지정된 객체의 개수

	// 객체별 정보 (Beam 타입)
	vector<int>		beamLength;					// 보 길이
	vector<short>	beamCount;					// 해당 보 길이에 대한 개수
	short nCountsBeam;							// 보 종류별 개수

	// 알 수 없는 객체
	short nUnknownObjects;						// 지정되지 않은 객체의 개수
};

// 보이는 레이어 상의 객체별 명칭, 존재 여부, 보이기 여부
struct VisibleObjectInfo
{
	// Object 타입
	short	nKinds;				// 객체 종류 개수
	char	varName [100][50];	// 1열: 변수명
	char	objName [100][128];	// 2열: 객체명
	bool	bExist [100];		// 존재 여부
	bool	bShow [100];		// 표시 여부
	short	itmIdx [100];		// 다이얼로그 내 인덱스

	// 알려지지 않은 Object 타입의 객체
	bool	bShow_Unknown;
	long	nUnknownObjects;

	// 나머지 타입
	bool	bExist_Walls;
	bool	bShow_Walls;
	short	itmIdx_Walls;

	bool	bExist_Columns;
	bool	bShow_Columns;
	short	itmIdx_Columns;

	bool	bExist_Beams;
	bool	bShow_Beams;
	short	itmIdx_Beams;

	bool	bExist_Slabs;
	bool	bShow_Slabs;
	short	itmIdx_Slabs;

	bool	bExist_Roofs;
	bool	bShow_Roofs;
	short	itmIdx_Roofs;

	bool	bExist_Meshes;
	bool	bShow_Meshes;
	short	itmIdx_Meshes;

	bool	bExist_Morphs;
	bool	bShow_Morphs;
	short	itmIdx_Morphs;

	bool	bExist_Shells;
	bool	bShow_Shells;
	short	itmIdx_Shells;

	// 존재하는 항목 전체 개수
	short	nItems;
};

void		initArray (double arr [], short arrSize);											// 배열 초기화 함수
int			compare (const void* first, const void* second);									// 오름차순으로 정렬할 때 사용하는 비교함수 (퀵소트)
ColumnInfo	findColumn (ColumnPos* columnPos, short iHor, short iVer, short floorInd);			// 가로주열, 세로주열, 층 정보를 이용하여 기둥 찾기
GSErrCode	exportGridElementInfo (void);														// 부재(기둥,보,슬래브)들의 정보를 추출하고 정리해서 엑셀 파일로 내보내기
short		DGCALLBACK inputThresholdHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그] 기둥 간 최소 간격 거리를 사용자에게 입력 받음 (기본값: 2000 mm)
GSErrCode	exportSelectedElementInfo (void);													// 선택한 부재 정보 내보내기 (Single 모드)
GSErrCode	exportElementInfoOnVisibleLayers (void);											// 선택한 부재 정보 내보내기 (Multi 모드)
GSErrCode	filterSelection (void);																// 부재별 선택 후 보여주기
short		DGCALLBACK filterSelectionHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그] 다이얼로그에서 보이는 레이어 상에 있는 객체들의 종류를 보여주고, 체크한 종류의 객체들만 선택 후 보여줌

#endif