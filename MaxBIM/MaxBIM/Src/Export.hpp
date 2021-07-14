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

// 선택한 부재들의 요약 정보
class SummaryOfObjectInfo
{
public:
	SummaryOfObjectInfo ();		// 생성자

public:
	int vectorSize;

	// 키
	vector<string>	nameKey;		// 객체를 구분할 수 있는 값(문자열)이 들어 있는 변수 이름 (예: sup_type)
	vector<string>	nameVal;		// 객체의 이름 (예: KS프로파일)

	vector<short>	nInfo;			// 표시할 정보 필드 개수

	// 객체별 정보 (Object 타입)
	vector<string>	varName [200];					// 변수 이름 (예: nom)
	vector<string>	varDesc [200];					// 변수 이름에 대한 설명 (예: 규격)
	vector<short>	varShowFlag [200];				// 변수 항목 표시 여부 (0: 그대로 표시, n: n번 변수가 양수이면 표시, -n: n번 변수가 음수이면 표시)
	vector<vector<string>>	varValue [200];			// 변수 이름에 대한 값
	vector<API_AddParID>	varType [200];			// 변수 이름에 대한 값의 타입

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