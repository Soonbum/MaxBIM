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
	vector<vector<string>>	var1value;			// 변수1 이름에 대한 값
	vector<API_AddParID>	var1type;			// 변수1 이름에 대한 값의 타입

	vector<string>	var2name;					// 변수2 이름 (예: nom)
	vector<string>	var2desc;					// 변수2 이름에 대한 설명 (예: 규격)
	vector<vector<string>>	var2value;			// 변수2 이름에 대한 값
	vector<API_AddParID>	var2type;			// 변수2 이름에 대한 값의 타입
	
	vector<string>	var3name;					// 변수3 이름 (예: nom)
	vector<string>	var3desc;					// 변수3 이름에 대한 설명 (예: 규격)
	vector<vector<string>>	var3value;			// 변수3 이름에 대한 값
	vector<API_AddParID>	var3type;			// 변수3 이름에 대한 값의 타입

	vector<string>	var4name;					// 변수4 이름 (예: nom)
	vector<string>	var4desc;					// 변수4 이름에 대한 설명 (예: 규격)
	vector<vector<string>>	var4value;			// 변수4 이름에 대한 값
	vector<API_AddParID>	var4type;			// 변수4 이름에 대한 값의 타입

	vector<string>	var5name;					// 변수5 이름 (예: nom)
	vector<string>	var5desc;					// 변수5 이름에 대한 설명 (예: 규격)
	vector<vector<string>>	var5value;			// 변수5 이름에 대한 값
	vector<API_AddParID>	var5type;			// 변수5 이름에 대한 값의 타입

	vector<string>	var6name;					// 변수6 이름 (예: nom)
	vector<string>	var6desc;					// 변수6 이름에 대한 설명 (예: 규격)
	vector<vector<string>>	var6value;			// 변수6 이름에 대한 값
	vector<API_AddParID>	var6type;			// 변수6 이름에 대한 값의 타입

	vector<string>	var7name;					// 변수7 이름 (예: nom)
	vector<string>	var7desc;					// 변수7 이름에 대한 설명 (예: 규격)
	vector<vector<string>>	var7value;			// 변수7 이름에 대한 값
	vector<API_AddParID>	var7type;			// 변수7 이름에 대한 값의 타입

	vector<string>	var8name;					// 변수8 이름 (예: nom)
	vector<string>	var8desc;					// 변수8 이름에 대한 설명 (예: 규격)
	vector<vector<string>>	var8value;			// 변수8 이름에 대한 값
	vector<API_AddParID>	var8type;			// 변수8 이름에 대한 값의 타입

	vector<string>	var9name;					// 변수9 이름 (예: nom)
	vector<string>	var9desc;					// 변수9 이름에 대한 설명 (예: 규격)
	vector<vector<string>>	var9value;			// 변수9 이름에 대한 값
	vector<API_AddParID>	var9type;			// 변수9 이름에 대한 값의 타입

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

// 선택한 부재들의 요약 정보
struct SummaryOfSelectedObjects
{
	// 유로폼
	int uformWidth [100];				// 유로폼 너비
	int uformHeight [100];				// 유로폼 높이
	int uformCount [100];				// 해당 유로폼 너비/높이 조합에 대한 개수
	int sizeOfUformKinds;				// 유로폼 종류별 개수

	// 스틸폼ㅎ
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

	// KS프로파일
	char KSProfileType [200][10];		// KS프로파일 타입 (기둥, 보)
	char KSProfileShape [200][30];		// KS프로파일 형태
	char KSProfileNom [200][50];		// KS프로파일 규격
	double KSProfileLen [200];			// KS프로파일 길이
	int KSProfileCount [200];			// 해당 KS프로파일 형태/규격/길이 조합에 대한 개수
	int sizeOfKSProfileKinds;			// KS프로파일 종류별 개수

	// PERI동바리 수직재
	char PERI_suppVerPostNom [100][10];	// PERI동바리 수직재 규격
	int PERI_suppVerPostLen [100];		// PERI동바리 수직재 현재 길이
	int PERI_suppVerPostCount [100];	// 해당 PERI동바리 수직재 규격/현재 길이 조합에 대한 개수
	int sizeOfPERI_suppVerPostKinds;	// PERI동바리 수직재 종류별 개수

	// PERI동바리 수평재
	char PERI_suppHorPostNom [20][10];	// PERI동바리 수평재 규격
	int PERI_suppHorPostCount [20];		// 해당 PERI동바리 수평재 규격 조합에 대한 개수
	int sizeOfPERI_suppHorPostKinds;	// PERI동바리 수평재 종류별 개수

	// GT24 거더
	char GT24GirderNom [30][10];		// GT24 거더 규격
	int GT24GirderCount [30];			// 해당 GT24 거더 규격에 대한 개수
	int sizeOfGT24Girder;				// GT24 거더 종류별 개수

	// 매직바
	int MagicBarLen [100];				// 매직바 길이
	int MagicBarCount [100];			// 해당 매직바 길이에 대한 개수
	int sizeOfMagicBar;					// 매직바 종류별 개수

	// 매직아웃코너
	char MagicOutcornerType [100][10];	// 매직아웃코너 규격
	int MagicOutcornerLen [100];		// 매직아웃코너 길이
	int MagicOutcornerCount [100];		// 해당 매직아웃코너 규격/길이 조합에 대한 개수
	int sizeOfMagicOutcorner;			// 매직아웃코너 종류별 개수

	// 매직인코너
	char MagicIncornerType [100][10];	// 매직인코너 규격
	int MagicIncornerLen [100];			// 매직인코너 길이
	int MagicIncornerCount [100];		// 해당 매직인코너 길이에 대한 개수
	int sizeOfMagicIncorner;			// 매직인코너 종류별 개수

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