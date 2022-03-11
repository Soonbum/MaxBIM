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

	enum	BEAMTABLEFORM_DIRECTION {
		HORIZONTAL_DIRECTION,
		VERTICAL_DIRECTION
	};

	enum	BEAMTABLEFORM_OBJECT_TYPE {
		NONE,
		EUROFORM,
		PLYWOOD
	};

	enum	ATTACH_POSITION {
		NO_POSITION,
		LEFT_SIDE,
		RIGHT_SIDE,
		BOTTOM_SIDE
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
	void clear ();				// 레코드 내용 지우기
	int	quantityPlus1 (vector<string> record);	// 객체의 레코드 수량 1 증가 (있으면 증가, 없으면 신규 추가)

public:
	// objectInfo.csv 파일 정보
	vector<string>				keyName;	// 객체를 구분할 수 있는 값(문자열)이 들어 있는 변수 이름 (예: u_comp)
	vector<string>				keyDesc;	// 객체의 이름 (예: 유로폼)
	vector<int>					nInfo;		// 표시할 정보 필드 개수
	vector<vector<string>>		varName;	// 변수의 이름 (예: eu_stan_onoff)		-- 내부에 또 다른 vector<string>가 삽입되며, 그것의 길이는 해당 객체의 nInfo와 같음
	vector<vector<string>>		varDesc;	// 변수의 이름에 대한 설명 (예: 규격폼)	-- 내부에 또 다른 vector<string>가 삽입되며, 그것의 길이는 해당 객체의 nInfo와 같음

	// 객체의 변수 값들의 조합별 수량
	vector<vector<string>>		records;	// 객체의 이름, 변수 값들의 조합, 조합에 해당하는 객체 개수를 담고 있음 (필드 1개의 예시: 인코너판넬 | 100 | 100 | 1200 | 3)

	// 기타
	int nKnownObjects;						// 지정된 객체의 개수
	int nUnknownObjects;					// 지정되지 않은 객체의 개수

	// 객체별 정보 (Beam 타입)
	vector<int>		beamLength;				// 보 길이
	vector<int>		beamCount;				// 해당 보 길이에 대한 개수
	int				nCountsBeam;			// 보 종류별 개수
};

// 보이는 레이어 상의 객체별 명칭, 존재 여부, 보이기 여부
struct VisibleObjectInfo
{
	// Object 타입
	short	nKinds;				// 객체 종류 개수
	char	varName [100][64];	// 1열: 변수명
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

// 보 테이블폼 내 자재들에 대한 정보
struct objectInBeamTableform
{
	short	objType;			// EUROFORM, PLYWOOD
	short	attachPosition;		// LEFT_SIDE, RIGHT_SIDE, BOTTOM_SIDE
	API_Coord3D	origin;			// 원점 좌표
	API_Coord3D	minPos, maxPos;	// 최소점, 최대점 위치
	double	width;				// 객체 너비
	double	length;				// 객체 길이
};

// 보 테이블폼 물량표 작성을 위한 구조체에 사용되는 셀 정보
class BeamTableformCell
{
public:
	double	euroform_leftHeight;		// 왼쪽/아래쪽 높이 (유로폼)
	double	euroform_rightHeight;		// 오른쪽/위쪽 높이 (유로폼)
	double	euroform_bottomWidth;		// 하부 너비 (유로폼)

	double	plywoodOnly_leftHeight;		// 왼쪽/아래쪽 높이 (전체 합판)
	double	plywoodOnly_rightHeight;	// 오른쪽/위쪽 높이 (전체 합판)
	double	plywoodOnly_bottomWidth;	// 하부 너비 (전체 합판)

	double	length;						// 셀 길이

public:
	bool operator == (BeamTableformCell& other) {
		if (this->euroform_leftHeight > EPS) {
			if ((abs (this->euroform_leftHeight - other.euroform_leftHeight) < EPS) && (abs (this->euroform_rightHeight - other.euroform_rightHeight) < EPS) && (abs (this->euroform_bottomWidth - other.euroform_bottomWidth) < EPS) && (abs (this->length - other.length) < EPS))
				return true;
			else
				return false;
		} else {
			if ((abs (this->plywoodOnly_leftHeight - other.plywoodOnly_leftHeight) < EPS) && (abs (this->plywoodOnly_rightHeight - other.plywoodOnly_rightHeight) < EPS) && (abs (this->plywoodOnly_bottomWidth - other.plywoodOnly_bottomWidth) < EPS) && (abs (this->length - other.length) < EPS))
				return true;
			else
				return false;
		}
	}
	bool operator != (BeamTableformCell& other) {
		if (this->euroform_leftHeight > EPS) {
			if ((abs (this->euroform_leftHeight - other.euroform_leftHeight) < EPS) && (abs (this->euroform_rightHeight - other.euroform_rightHeight) < EPS) && (abs (this->euroform_bottomWidth - other.euroform_bottomWidth) < EPS) && (abs (this->length - other.length) < EPS))
				return false;
			else
				return true;
		} else {
			if ((abs (this->plywoodOnly_leftHeight - other.plywoodOnly_leftHeight) < EPS) && (abs (this->plywoodOnly_rightHeight - other.plywoodOnly_rightHeight) < EPS) && (abs (this->plywoodOnly_bottomWidth - other.plywoodOnly_bottomWidth) < EPS) && (abs (this->length - other.length) < EPS))
				return false;
			else
				return true;
		}
	}
};

// 레이어 이름 및 해당 레이어의 인덱스
class LayerList
{
public:
	short	layerInd;
	string	layerName;
};

// 보 테이블폼의 유로폼 셀 타입 정보
class BeamTableformEuroformCellType
{
public:
	short				nCells;
	BeamTableformCell	cells [50];
	vector<string>		layerNames [50];
};

// 보 테이블폼 물량표 작성을 위한 클래스
class BeamTableformInfo
{
public:
	short	iBeamDirection;		// 보 방향 (HORIZONTAL_DIRECTION, VERTICAL_DIRECTION)
	short	nCells;						// 셀 개수
	BeamTableformCell	cells [50];		// 셀 정보

public:
	BeamTableformInfo ();		// 보 테이블폼 물량표 작성을 위한 클래스 생성자 (초기화)
	void init ();				// 초기화
};

void		initArray (double arr [], short arrSize);											// 배열 초기화 함수
int			compare (const void* first, const void* second);									// 오름차순으로 정렬할 때 사용하는 비교함수 (퀵소트)
bool		comparePosX (const objectInBeamTableform& a, const objectInBeamTableform& b);		// vector 내 자재 정보 구조체 정렬을 위한 비교 함수 (좌표값 X 기준)
bool		comparePosY (const objectInBeamTableform& a, const objectInBeamTableform& b);		// vector 내 자재 정보 구조체 정렬을 위한 비교 함수 (좌표값 Y 기준)
bool		compareLayerName (const LayerList& a, const LayerList& b);							// vector 내 레이어 정보 구조체 정렬을 위한 비교 함수 (레이어 이름 기준)
bool		compareVectorStringByValue (const vector<string>& a, const vector<string>& b);		// vector 내 레코드 내 필드(숫자 값)를 기준으로 내림차순 정렬을 위한 비교 함수
bool		compareVectorStringByString (const vector<string>& a, const vector<string>& b);		// vector 내 레코드 내 필드(문자열)를 기준으로 내림차순 정렬을 위한 비교 함수
bool		compareVectorStringMultiField (const vector<string>& a, const vector<string>& b);	// vector 내 레코드 내 여러 필드들을 기준으로 내림차순 정렬을 위한 비교 함수

ColumnInfo	findColumn (ColumnPos* columnPos, short iHor, short iVer, short floorInd);			// 가로주열, 세로주열, 층 정보를 이용하여 기둥 찾기
GSErrCode	exportGridElementInfo (void);														// 부재(기둥,보,슬래브)들의 정보를 추출하고 정리해서 엑셀 파일로 내보내기
short		DGCALLBACK inputThresholdHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그] 기둥 간 최소 간격 거리를 사용자에게 입력 받음 (기본값: 2000 mm)

GSErrCode	exportSelectedElementInfo (void);													// 선택한 부재 정보 내보내기 (Single 모드)
GSErrCode	exportElementInfoOnVisibleLayers (void);											// 선택한 부재 정보 내보내기 (Multi 모드)

int			quantityPlusN (vector<vector<string>> *db, vector<string> record, int n);			// 객체의 레코드 수량 n 증가

GSErrCode	filterSelection (void);																// 부재별 선택 후 보여주기
short		DGCALLBACK filterSelectionHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그] 다이얼로그에서 보이는 레이어 상에 있는 객체들의 종류를 보여주고, 체크한 종류의 객체들만 선택 후 보여줌

GSErrCode	exportBeamTableformInformation (void);												// 보 테이블폼 물량 정보 내보내기

#endif