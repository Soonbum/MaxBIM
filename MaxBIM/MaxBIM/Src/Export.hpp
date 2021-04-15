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
struct SummaryOfSelectedObjects
{
	// 유로폼
	int uformWidth [100];				// 유로폼 너비
	int uformHeight [100];				// 유로폼 높이
	int uformCount [100];				// 해당 유로폼 너비/높이 조합에 대한 개수
	int sizeOfUformKinds;				// 유로폼 종류별 개수

	// 스틸폼
	int steelformWidth [100];			// 스틸폼 너비
	int steelformHeight [100];			// 스틸폼 높이
	int steelformCount [100];			// 해당 스틸폼 너비/높이 조합에 대한 개수
	int	sizeOfSteelformKinds;			// 스틸폼 종류별 개수

	// 인코너판넬
	int incornerPanelHor [100];			// 인코너판넬 가로
	int incornerPanelVer [100];			// 인코너판넬 세로
	int incornerPanelHei [100];			// 인코너판넬 높이
	int incornerPanelCount [100];		// 해당 인코너판넬 가로/세로/높이 조합에 대한 개수
	int	sizeOfIncornerPanelKinds;		// 인코너판넬 종류별 개수

	// 아웃코너판넬
	int outcornerPanelHor [100];		// 아웃코너판넬 가로
	int outcornerPanelVer [100];		// 아웃코너판넬 세로
	int outcornerPanelHei [100];		// 아웃코너판넬 높이
	int outcornerPanelCount [100];		// 해당 아웃코너판넬 가로/세로/높이 조합에 대한 개수
	int	sizeOfOutcornerPanelKinds;		// 아웃코너판넬 종류별 개수

	// 아웃코너앵글
	int outcornerAngleLength [100];		// 아웃코너앵글 길이
	int outcornerAngleCount [100];		// 해당 아웃코너앵글 길이에 대한 개수
	int sizeOfOutcornerAngleKinds;		// 아웃코너앵글 종류별 개수

	// 목재
	int woodThk [100];					// 목재 두께
	int woodWidth [100];				// 목재 너비
	int woodLength [100];				// 목재 길이
	int woodCount [100];				// 해당 목재 두께/너비/길이 조합에 대한 개수
	int sizeOfWoodKinds;				// 목재 종류별 개수

	// 휠러스페이서
	int fsThk [100];					// 휠러스페이서 두께
	int fsLength [100];					// 휠러스페이서 길이
	int fsCount [100];					// 해당 휠러스페이서 두께/길이 조합에 대한 개수
	int sizeOfFsKinds;					// 휠러스페이서 종류별 개수

	// 사각파이프 (비계파이프, 직사각파이프)
	int sqrPipeHor [100];				// 사각파이프 단면 가로
	int sqrPipeVer [100];				// 사각파이프 단면 세로
	int sqrPipeLength [100];			// 사각파이프 길이
	int sqrPipeCount [100];				// 해당 사각파이프 가로/세로/길이 조합에 대한 개수
	int sizeOfSqrPipeKinds;				// 사각파이프 종류별 개수

	// 합판
	int plywoodHor [100];				// 가로
	int plywoodVer [100];				// 세로
	double plywoodThk [100];			// 두께
	int plywoodCount [100];				// 해당 합판 가로/세로/두께 조합에 대한 개수
	int sizeOfPlywoodKinds;				// 합판 종류별 개수

	// RS Push-Pull Props 헤드피스 (인양고리 포함)
	int nHeadpiece;

	// RS Push-Pull Props
	bool bPropsUpperSupp [100];			// 상부(메인) 지지대 여부
	char PropsNomUpperSupp [100][30];	// 상부(메인) 지지대 규격
	bool bPropsLowerSupp [100];			// 하부(보조) 지지대 여부
	char PropsNomLowerSupp [100][30];	// 하부(보조) 지지대 규격
	bool bPropsBase [100];				// 베이스 플레이트 여부
	int propsCount [100];				// 해당 Props 지지대 여부/규격, 베이스 플레이트 여부 조합에 대한 개수
	int sizeOfPropsKinds;				// Props 종류별 개수

	// 벽체 타이
	int nTie;

	// 직교클램프
	int nClamp;

	// 핀볼트세트
	int pinboltLen [100];				// 핀볼트 길이
	int pinboltCount [100];				// 해당 길이에 대한 개수
	int sizeOfPinboltKinds;				// 핀볼트 종류별 개수

	// 결합철물 (사각와셔활용)
	int nJoin;

	// 보 멍에제
	int beamYokeLength [10];			// 보 멍에제 길이
	int beamYokeCount [10];				// 해당 보 멍에제 길이에 대한 개수
	int sizeOfBeamYokeKinds;			// 보 멍에제 종류별 개수

	// 일반 요소
	// 보
	int beamLength [100];				// 보 길이
	int beamCount [100];				// 해당 보 길이에 대한 개수
	int sizeOfBeamKinds;				// 보 종류별 개수

	// 알 수 없는 객체
	int nUnknownObjects;				// 지정되지 않은 객체의 개수
};

void		initArray (double arr [], short arrSize);											// 배열 초기화 함수
int			compare (const void* first, const void* second);									// 오름차순으로 정렬할 때 사용하는 비교함수 (퀵소트)
ColumnInfo	findColumn (ColumnPos* columnPos, short iHor, short iVer, short floorInd);			// 가로주열, 세로주열, 층 정보를 이용하여 기둥 찾기
GSErrCode	exportGridElementInfo (void);														// 부재(기둥,보,슬래브)들의 정보를 추출하고 정리해서 엑셀 파일로 내보내기
short		DGCALLBACK inputThresholdHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그] 기둥 간 최소 간격 거리를 사용자에게 입력 받음 (기본값: 2000 mm)
GSErrCode	exportSelectedElementInfo (void);													// 선택한 부재 정보 내보내기

#endif