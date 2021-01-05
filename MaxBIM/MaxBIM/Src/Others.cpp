#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Others.hpp"

using namespace othersDG;

LayerNameSystem	layerInfo;
LayerNameSystem	selectedInfo;


// 레이어 정보 초기화
void	initLayerInfo (LayerNameSystem *layerInfo)
{
	short xx;

	// 공사 구분
	layerInfo->bType_01_S = false;
	layerInfo->bType_02_A = false;
	layerInfo->bType_03_M = false;
	layerInfo->bType_04_E = false;
	layerInfo->bType_05_T = false;
	layerInfo->bType_06_F = false;
	layerInfo->bType_07_Q = false;
	layerInfo->bType_08_L = false;
	layerInfo->bType_09_C = false;
	layerInfo->bType_10_K = false;

	layerInfo->iType_01_S = 0;
	layerInfo->iType_02_A = 0;
	layerInfo->iType_03_M = 0;
	layerInfo->iType_04_E = 0;
	layerInfo->iType_05_T = 0;
	layerInfo->iType_06_F = 0;
	layerInfo->iType_07_Q = 0;
	layerInfo->iType_08_L = 0;
	layerInfo->iType_09_C = 0;
	layerInfo->iType_10_K = 0;

	// 동 구분
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo->bDong_1xx [xx] = false;
		layerInfo->bDong_2xx [xx] = false;
		layerInfo->bDong_3xx [xx] = false;
		layerInfo->bDong_4xx [xx] = false;
		layerInfo->bDong_5xx [xx] = false;
		layerInfo->bDong_6xx [xx] = false;
		layerInfo->bDong_7xx [xx] = false;
		layerInfo->bDong_8xx [xx] = false;
		layerInfo->bDong_9xx [xx] = false;
		layerInfo->bDong_10xx [xx] = false;
		layerInfo->bDong_11xx [xx] = false;
		layerInfo->bDong_12xx [xx] = false;
		layerInfo->bDong_13xx [xx] = false;
		layerInfo->bDong_14xx [xx] = false;
		layerInfo->bDong_15xx [xx] = false;
		layerInfo->bDong_SHOP = false;
		layerInfo->bDong_SECU = false;

		layerInfo->iDong_1xx [xx] = 0;
		layerInfo->iDong_2xx [xx] = 0;
		layerInfo->iDong_3xx [xx] = 0;
		layerInfo->iDong_4xx [xx] = 0;
		layerInfo->iDong_5xx [xx] = 0;
		layerInfo->iDong_6xx [xx] = 0;
		layerInfo->iDong_7xx [xx] = 0;
		layerInfo->iDong_8xx [xx] = 0;
		layerInfo->iDong_9xx [xx] = 0;
		layerInfo->iDong_10xx [xx] = 0;
		layerInfo->iDong_11xx [xx] = 0;
		layerInfo->iDong_12xx [xx] = 0;
		layerInfo->iDong_13xx [xx] = 0;
		layerInfo->iDong_14xx [xx] = 0;
		layerInfo->iDong_15xx [xx] = 0;
		layerInfo->iDong_SHOP = 0;
		layerInfo->iDong_SECU = 0;
	}

	// 층 구분
	for (xx = 0 ; xx < 10 ; ++xx) {
		layerInfo->bFloor_Bxx [xx] = false;
		layerInfo->bFloor_PHxx [xx] = false;

		layerInfo->iFloor_Bxx [xx] = 0;
		layerInfo->iFloor_PHxx [xx] = 0;
	}
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo->bFloor_Fxx [xx] = false;

		layerInfo->iFloor_Fxx [xx] = 0;
	}

	// CJ 구간
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo->bCJ [xx] = false;

		layerInfo->iCJ [xx] = 0;
	}

	// CJ 속 시공순서
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo->bOrderInCJ [xx] = false;

		layerInfo->iOrderInCJ [xx] = 0;
	}

	// 부재 및 객체 구분
	// 01-S 구조
	layerInfo->bType_01_S_STAR = false;	// 계단
	layerInfo->bType_01_S_COLU = false;	// 기둥
	layerInfo->bType_01_S_FOOT = false;	// 기초
	layerInfo->bType_01_S_WALL = false;	// 벽체
	layerInfo->bType_01_S_BEAM = false;	// 보
	layerInfo->bType_01_S_SLAB = false;	// 슬래브
	layerInfo->bType_01_S_CLST = false;	// 철골기둥
	layerInfo->bType_01_S_BMST = false;	// 철골보
	layerInfo->bType_01_S_RAMP = false;	// 램프
	layerInfo->bType_01_S_CWAL = false;	// 합벽
	layerInfo->bType_01_S_WTWL = false;	// 방수턱
	layerInfo->bType_01_S_CSTN = false;	// 연석
	layerInfo->bType_01_S_MPAD = false;	// 장비패드
	layerInfo->bType_01_S_GADN = false;	// 화단
	layerInfo->bType_01_S_PARA = false;	// 파라펫
	layerInfo->bType_01_S_CLPC = false;	// PC기둥
	layerInfo->bType_01_S_BMPC = false;	// 보PC
	layerInfo->bType_01_S_BMWL = false;	// 보벽체
	layerInfo->bType_01_S_STST = false;	// 철골계단

	layerInfo->iType_01_S_STAR = 0;	// 계단
	layerInfo->iType_01_S_COLU = 0;	// 기둥
	layerInfo->iType_01_S_FOOT = 0;	// 기초
	layerInfo->iType_01_S_WALL = 0;	// 벽체
	layerInfo->iType_01_S_BEAM = 0;	// 보
	layerInfo->iType_01_S_SLAB = 0;	// 슬래브
	layerInfo->iType_01_S_CLST = 0;	// 철골기둥
	layerInfo->iType_01_S_BMST = 0;	// 철골보
	layerInfo->iType_01_S_RAMP = 0;	// 램프
	layerInfo->iType_01_S_CWAL = 0;	// 합벽
	layerInfo->iType_01_S_WTWL = 0;	// 방수턱
	layerInfo->iType_01_S_CSTN = 0;	// 연석
	layerInfo->iType_01_S_MPAD = 0;	// 장비패드
	layerInfo->iType_01_S_GADN = 0;	// 화단
	layerInfo->iType_01_S_PARA = 0;	// 파라펫
	layerInfo->iType_01_S_CLPC = 0;	// PC기둥
	layerInfo->iType_01_S_BMPC = 0;	// 보PC
	layerInfo->iType_01_S_BMWL = 0;	// 보벽체
	layerInfo->iType_01_S_STST = 0;	// 철골계단

	// 02-A 건축마감
	layerInfo->bType_02_A_FURN = false;	// 가구
	layerInfo->bType_02_A_INSU = false;	// 단열재
	layerInfo->bType_02_A_PAIN = false;	// 도장
	layerInfo->bType_02_A_MOLD = false;	// 몰딩
	layerInfo->bType_02_A_MORT = false;	// 몰탈
	layerInfo->bType_02_A_WATE = false;	// 방수
	layerInfo->bType_02_A_BRIC = false;	// 벽돌
	layerInfo->bType_02_A_PAPE = false;	// 벽지
	layerInfo->bType_02_A_BLOC = false;	// 블록
	layerInfo->bType_02_A_GYPS = false;	// 석고보드
	layerInfo->bType_02_A_STON = false;	// 석재
	layerInfo->bType_02_A_INTE = false;	// 수장
	layerInfo->bType_02_A_GLAS = false;	// 유리
	layerInfo->bType_02_A_HARD = false;	// 철물
	layerInfo->bType_02_A_TILE = false;	// 타일
	layerInfo->bType_02_A_PANE = false;	// 판넬
	layerInfo->bType_02_A_PLYW = false;	// 합판
	layerInfo->bType_02_A_PCON = false;	// 무근콘크리트
	
	layerInfo->iType_02_A_FURN = 0;	// 가구
	layerInfo->iType_02_A_INSU = 0;	// 단열재
	layerInfo->iType_02_A_PAIN = 0;	// 도장
	layerInfo->iType_02_A_MOLD = 0;	// 몰딩
	layerInfo->iType_02_A_MORT = 0;	// 몰탈
	layerInfo->iType_02_A_WATE = 0;	// 방수
	layerInfo->iType_02_A_BRIC = 0;	// 벽돌
	layerInfo->iType_02_A_PAPE = 0;	// 벽지
	layerInfo->iType_02_A_BLOC = 0;	// 블록
	layerInfo->iType_02_A_GYPS = 0;	// 석고보드
	layerInfo->iType_02_A_STON = 0;	// 석재
	layerInfo->iType_02_A_INTE = 0;	// 수장
	layerInfo->iType_02_A_GLAS = 0;	// 유리
	layerInfo->iType_02_A_HARD = 0;	// 철물
	layerInfo->iType_02_A_TILE = 0;	// 타일
	layerInfo->iType_02_A_PANE = 0;	// 판넬
	layerInfo->iType_02_A_PLYW = 0;	// 합판
	layerInfo->iType_02_A_PCON = 0;	// 무근콘크리트

	// 05-T 가설재
	layerInfo->bType_05_T_TIMB = false;	// 각재
	layerInfo->bType_05_T_BIMJ = false;	// 각파이프행거
	layerInfo->bType_05_T_BDCM = false;	// 기둥밴드
	layerInfo->bType_05_T_DMGA = false;	// 다마가
	layerInfo->bType_05_T_RIBL = false;	// 리브라스
	layerInfo->bType_05_T_BMSP = false;	// 보받침필러
	layerInfo->bType_05_T_BSTA = false;	// 비계계단
	layerInfo->bType_05_T_SPIP = false;	// 사각파이프
	layerInfo->bType_05_T_SUPT = false;	// 서포트
	layerInfo->bType_05_T_SYSU = false;	// 시스템서포트
	layerInfo->bType_05_T_OUTA = false;	// 아웃코너앵글
	layerInfo->bType_05_T_OUTP = false;	// 아웃코너판넬
	layerInfo->bType_05_T_AFOM = false;	// 알폼
	layerInfo->bType_05_T_CPIP = false;	// 원형파이프
	layerInfo->bType_05_T_UGON = false;	// 유공발판
	layerInfo->bType_05_T_UFOM = false;	// 유로폼
	layerInfo->bType_05_T_INCO = false;	// 인코너판넬
	layerInfo->bType_05_T_JOIB = false;	// 조인트바
	layerInfo->bType_05_T_EFOM = false;	// 종이거푸집
	layerInfo->bType_05_T_JSUPT = false;	// 잭서포트
	layerInfo->bType_05_T_WTST = false;	// 지수판
	layerInfo->bType_05_T_CLAM = false;	// 클램프
	layerInfo->bType_05_T_LUMB = false;	// 토류판
	layerInfo->bType_05_T_TRUS = false;	// 트러스
	layerInfo->bType_05_T_TBBM = false;	// 팀버빔
	layerInfo->bType_05_T_BCWF = false;	// 합벽지지대
	layerInfo->bType_05_T_PLYW = false;	// 합판
	layerInfo->bType_05_T_FISP = false;	// 휠러스페이서
	layerInfo->bType_05_T_STSE = false;	// 계단제작폼
	layerInfo->bType_05_T_SLSE = false;	// 슬래브제작폼
	layerInfo->bType_05_T_RAIL = false;	// 목심레일
	
	layerInfo->iType_05_T_TIMB = 0;	// 각재
	layerInfo->iType_05_T_BIMJ = 0;	// 각파이프행거
	layerInfo->iType_05_T_BDCM = 0;	// 기둥밴드
	layerInfo->iType_05_T_DMGA = 0;	// 다마가
	layerInfo->iType_05_T_RIBL = 0;	// 리브라스
	layerInfo->iType_05_T_BMSP = 0;	// 보받침필러
	layerInfo->iType_05_T_BSTA = 0;	// 비계계단
	layerInfo->iType_05_T_SPIP = 0;	// 사각파이프
	layerInfo->iType_05_T_SUPT = 0;	// 서포트
	layerInfo->iType_05_T_SYSU = 0;	// 시스템서포트
	layerInfo->iType_05_T_OUTA = 0;	// 아웃코너앵글
	layerInfo->iType_05_T_OUTP = 0;	// 아웃코너판넬
	layerInfo->iType_05_T_AFOM = 0;	// 알폼
	layerInfo->iType_05_T_CPIP = 0;	// 원형파이프
	layerInfo->iType_05_T_UGON = 0;	// 유공발판
	layerInfo->iType_05_T_UFOM = 0;	// 유로폼
	layerInfo->iType_05_T_INCO = 0;	// 인코너판넬
	layerInfo->iType_05_T_JOIB = 0;	// 조인트바
	layerInfo->iType_05_T_EFOM = 0;	// 종이거푸집
	layerInfo->iType_05_T_JSUPT = 0;	// 잭서포트
	layerInfo->iType_05_T_WTST = 0;	// 지수판
	layerInfo->iType_05_T_CLAM = 0;	// 클램프
	layerInfo->iType_05_T_LUMB = 0;	// 토류판
	layerInfo->iType_05_T_TRUS = 0;	// 트러스
	layerInfo->iType_05_T_TBBM = 0;	// 팀버빔
	layerInfo->iType_05_T_BCWF = 0;	// 합벽지지대
	layerInfo->iType_05_T_PLYW = 0;	// 합판
	layerInfo->iType_05_T_FISP = 0;	// 휠러스페이서
	layerInfo->iType_05_T_STSE = 0;	// 계단제작폼
	layerInfo->iType_05_T_SLSE = 0;	// 슬래브제작폼
	layerInfo->iType_05_T_RAIL = 0;	// 목심레일

	// 06-F 가시설
	layerInfo->bType_06_F_STRU = false;	// 수평H형강
	layerInfo->bType_06_F_HFIL = false;	// 수직H형강
	layerInfo->bType_06_F_SJAK = false;	// 스크류잭
	layerInfo->bType_06_F_PJAK = false;	// 프리로딩잭
	layerInfo->bType_06_F_BRKT = false;	// 브라켓
	layerInfo->bType_06_F_PBKT = false;	// 피스 브라켓
	layerInfo->bType_06_F_CIP = false;	// 흙막이 벽체
	layerInfo->bType_06_F_LAND = false;	// 대지
	layerInfo->bType_06_F_ANGL = false;	// 앵글
	layerInfo->bType_06_F_ERAC = false;	// 지반앵커
	layerInfo->bType_06_F_LUMB = false;	// 토류판
	layerInfo->bType_06_F_BPAN = false;	// 복공판
	layerInfo->bType_06_F_WALE = false;	// 띠장
	layerInfo->bType_06_F_PILE = false;	// 파일

	layerInfo->iType_06_F_STRU = 0;	// 수평H형강
	layerInfo->iType_06_F_HFIL = 0;	// 수직H형강
	layerInfo->iType_06_F_SJAK = 0;	// 스크류잭
	layerInfo->iType_06_F_PJAK = 0;	// 프리로딩잭
	layerInfo->iType_06_F_BRKT = 0;	// 브라켓
	layerInfo->iType_06_F_PBKT = 0;	// 피스 브라켓
	layerInfo->iType_06_F_CIP = 0;	// 흙막이 벽체
	layerInfo->iType_06_F_LAND = 0;	// 대지
	layerInfo->iType_06_F_ANGL = 0;	// 앵글
	layerInfo->iType_06_F_ERAC = 0;	// 지반앵커
	layerInfo->iType_06_F_LUMB = 0;	// 토류판
	layerInfo->iType_06_F_BPAN = 0;	// 복공판
	layerInfo->iType_06_F_WALE = 0;	// 띠장
	layerInfo->iType_06_F_PILE = 0;	// 파일
}

// 레이어 쉽게 선택하기
GSErrCode	showLayersEasily (void)
{
	GSErrCode	err = NoError;
	
	API_Attribute	attrib;
	short			xx, i;
	short			nLayers;

	bool	success;
	char	*token;
	char	tempStr [5];
	char	tok1 [5];
	char	tok2 [5];
	char	tok3 [5];
	char	tok4 [5];
	char	tok5 [5];
	char	tok6 [5];
	char	tok7 [5];
	char	constructionCode [5];
	int		DongNumber;
	int		CJNumber;
	int		OrderInCJNumber;

	short	result;

	
	// 레이어 정보 초기화
	initLayerInfo (&layerInfo);
	initLayerInfo (&selectedInfo);

	// 프로젝트 내 레이어 개수를 알아냄
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	for (xx = 1; xx <= nLayers && err == NoError ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			strcpy (tok1, "");
			strcpy (tok2, "");
			strcpy (tok3, "");
			strcpy (tok4, "");
			strcpy (tok5, "");
			strcpy (tok6, "");
			strcpy (tok7, "");
			i = 1;
			success = 0;
			// 정규 코드: 01-S-(0101)-9B1-(01)-(01)-WALL  단, 괄호 안은 선택사항
			// 레이어 이름을 "-" 문자 기준으로 쪼개기
			token = strtok (attrib.layer.head.name, "-");
			while (token != NULL) {
				// 내용 및 길이 확인
				// 1차 (일련번호) - 필수 (2글자, 숫자)
				if (i == 1) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 2) {
						strcpy (tok1, tempStr);
						success = true;
					} else {
						i=8;
						success = false;
					}
				}
				// 2차 (공사 구분) - 필수 (1글자, 문자)
				if (i == 2) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 1) {
						strcpy (tok2, tempStr);
						success = true;
					} else {
						i=8;
						success = false;
					}
				}
				// 3차 (동 구분) - 선택 (4글자)
				if (i == 3) {
					strcpy (tempStr, token);
					// 동 구분일 경우,
					if (strlen (tempStr) == 4) {
						strcpy (tok3, tempStr);
						success = true;
					// 층 구분일 경우
					} else if (strlen (tempStr) == 3) {
						strcpy (tok4, tempStr);
						i=4;
						success = true;
					} else {
						i=8;
						success = false;
					}
				}
				// 4차 (층 구분) - 필수 (3글자)
				if (i == 4) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 3) {
						strcpy (tok4, tempStr);
						success = true;
					} else {
						success = false;
					}
				}
				// 5차 (CJ 구간) - 선택 (2글자, 숫자)
				if (i == 5) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 2) {
						strcpy (tok5, tempStr);
						success = true;
					} else if (strlen (tempStr) > 3) {
						strcpy (tok7, tempStr);
						i=8;
						success = true;
					} else {
						i=6;
						success = true;
					}
				}
				// 6차 (CJ 속 시공순서) - 선택 (2글자, 숫자)
				if (i == 6) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 2) {
						strcpy (tok6, tempStr);
						success = true;
					} else if (strlen (tempStr) >= 3) {
						strcpy (tok7, tempStr);
						i=8;
						success = true;
					} else {
						success = false;
					}
				}
				// 7차 (부재 및 객체 구분) - 필수 (3글자 이상)
				if (i == 7) {
					strcpy (tempStr, token);
					if (strlen (tempStr) >= 3) {
						strcpy (tok7, tempStr);
						success = true;
					} else {
						success = false;
					}
				}
				i++;
				token = strtok (NULL, "-");
			}

			// 7단계까지 성공적으로 완료되면 구조체에 적용
			if (success == true) {
				// 일련 번호와 공사 구분 문자를 먼저 합침
				sprintf (constructionCode, "%s-%s", tok1, tok2);
				
				// 1,2단계. 공사 구분 확인
				if (strncmp (constructionCode, "01-S", 4) == 0)	layerInfo.bType_01_S = true;
				if (strncmp (constructionCode, "02-A", 4) == 0)	layerInfo.bType_02_A = true;
				if (strncmp (constructionCode, "03-M", 4) == 0)	layerInfo.bType_03_M = true;
				if (strncmp (constructionCode, "04-E", 4) == 0)	layerInfo.bType_04_E = true;
				if (strncmp (constructionCode, "05-T", 4) == 0)	layerInfo.bType_05_T = true;
				if (strncmp (constructionCode, "06-F", 4) == 0)	layerInfo.bType_06_F = true;
				if (strncmp (constructionCode, "07-Q", 4) == 0)	layerInfo.bType_07_Q = true;
				if (strncmp (constructionCode, "08-L", 4) == 0)	layerInfo.bType_08_L = true;
				if (strncmp (constructionCode, "09-C", 4) == 0)	layerInfo.bType_09_C = true;
				if (strncmp (constructionCode, "10-K", 4) == 0)	layerInfo.bType_10_K = true;

				// 3단계. 동 구분
				DongNumber = atoi (tok3);
				if (DongNumber == 0) {
					if (strncmp (tok4, "SHOP", 4) == 0)	layerInfo.bDong_SHOP = true;
					if (strncmp (tok4, "SECU", 4) == 0)	layerInfo.bDong_SECU = true;
				} else {
					if ((DongNumber >= 100) && (DongNumber <= 199))
						layerInfo.bDong_1xx [DongNumber - 100] = true;
					if ((DongNumber >= 200) && (DongNumber <= 299))
						layerInfo.bDong_2xx [DongNumber - 200] = true;
					if ((DongNumber >= 300) && (DongNumber <= 399))
						layerInfo.bDong_3xx [DongNumber - 300] = true;
					if ((DongNumber >= 400) && (DongNumber <= 499))
						layerInfo.bDong_4xx [DongNumber - 400] = true;
					if ((DongNumber >= 500) && (DongNumber <= 599))
						layerInfo.bDong_5xx [DongNumber - 500] = true;
					if ((DongNumber >= 600) && (DongNumber <= 699))
						layerInfo.bDong_6xx [DongNumber - 600] = true;
					if ((DongNumber >= 700) && (DongNumber <= 799))
						layerInfo.bDong_7xx [DongNumber - 700] = true;
					if ((DongNumber >= 800) && (DongNumber <= 899))
						layerInfo.bDong_8xx [DongNumber - 800] = true;
					if ((DongNumber >= 900) && (DongNumber <= 999))
						layerInfo.bDong_9xx [DongNumber - 900] = true;
					if ((DongNumber >= 1000) && (DongNumber <= 1099))
						layerInfo.bDong_10xx [DongNumber - 1000] = true;
					if ((DongNumber >= 1100) && (DongNumber <= 1199))
						layerInfo.bDong_11xx [DongNumber - 1100] = true;
					if ((DongNumber >= 1200) && (DongNumber <= 1299))
						layerInfo.bDong_12xx [DongNumber - 1200] = true;
					if ((DongNumber >= 1300) && (DongNumber <= 1399))
						layerInfo.bDong_13xx [DongNumber - 1300] = true;
					if ((DongNumber >= 1400) && (DongNumber <= 1499))
						layerInfo.bDong_14xx [DongNumber - 1400] = true;
					if ((DongNumber >= 1500) && (DongNumber <= 1599))
						layerInfo.bDong_15xx [DongNumber - 1500] = true;
				}

				// 4단계. 층 구분
				if (strncmp (tok4, "9B1", 3) == 0)	layerInfo.bFloor_Bxx [1] = true;
				if (strncmp (tok4, "8B2", 3) == 0)	layerInfo.bFloor_Bxx [2] = true;
				if (strncmp (tok4, "7B3", 3) == 0)	layerInfo.bFloor_Bxx [3] = true;
				if (strncmp (tok4, "6B4", 3) == 0)	layerInfo.bFloor_Bxx [4] = true;
				if (strncmp (tok4, "5B5", 3) == 0)	layerInfo.bFloor_Bxx [5] = true;
				if (strncmp (tok4, "4B6", 3) == 0)	layerInfo.bFloor_Bxx [6] = true;
				if (strncmp (tok4, "3B7", 3) == 0)	layerInfo.bFloor_Bxx [7] = true;
				if (strncmp (tok4, "2B8", 3) == 0)	layerInfo.bFloor_Bxx [8] = true;
				if (strncmp (tok4, "1B9", 3) == 0)	layerInfo.bFloor_Bxx [9] = true;

				if (strncmp (tok4, "F01", 3) == 0)	layerInfo.bFloor_Fxx [1] = true;
				if (strncmp (tok4, "F02", 3) == 0)	layerInfo.bFloor_Fxx [2] = true;
				if (strncmp (tok4, "F03", 3) == 0)	layerInfo.bFloor_Fxx [3] = true;
				if (strncmp (tok4, "F04", 3) == 0)	layerInfo.bFloor_Fxx [4] = true;
				if (strncmp (tok4, "F05", 3) == 0)	layerInfo.bFloor_Fxx [5] = true;
				if (strncmp (tok4, "F06", 3) == 0)	layerInfo.bFloor_Fxx [6] = true;
				if (strncmp (tok4, "F07", 3) == 0)	layerInfo.bFloor_Fxx [7] = true;
				if (strncmp (tok4, "F08", 3) == 0)	layerInfo.bFloor_Fxx [8] = true;
				if (strncmp (tok4, "F09", 3) == 0)	layerInfo.bFloor_Fxx [9] = true;
				if (strncmp (tok4, "F10", 3) == 0)	layerInfo.bFloor_Fxx [10] = true;
				if (strncmp (tok4, "F11", 3) == 0)	layerInfo.bFloor_Fxx [11] = true;
				if (strncmp (tok4, "F12", 3) == 0)	layerInfo.bFloor_Fxx [12] = true;
				if (strncmp (tok4, "F13", 3) == 0)	layerInfo.bFloor_Fxx [13] = true;
				if (strncmp (tok4, "F14", 3) == 0)	layerInfo.bFloor_Fxx [14] = true;
				if (strncmp (tok4, "F15", 3) == 0)	layerInfo.bFloor_Fxx [15] = true;
				if (strncmp (tok4, "F16", 3) == 0)	layerInfo.bFloor_Fxx [16] = true;
				if (strncmp (tok4, "F17", 3) == 0)	layerInfo.bFloor_Fxx [17] = true;
				if (strncmp (tok4, "F18", 3) == 0)	layerInfo.bFloor_Fxx [18] = true;
				if (strncmp (tok4, "F19", 3) == 0)	layerInfo.bFloor_Fxx [19] = true;
				if (strncmp (tok4, "F20", 3) == 0)	layerInfo.bFloor_Fxx [20] = true;
				if (strncmp (tok4, "F21", 3) == 0)	layerInfo.bFloor_Fxx [21] = true;
				if (strncmp (tok4, "F22", 3) == 0)	layerInfo.bFloor_Fxx [22] = true;
				if (strncmp (tok4, "F23", 3) == 0)	layerInfo.bFloor_Fxx [23] = true;
				if (strncmp (tok4, "F24", 3) == 0)	layerInfo.bFloor_Fxx [24] = true;
				if (strncmp (tok4, "F25", 3) == 0)	layerInfo.bFloor_Fxx [25] = true;
				if (strncmp (tok4, "F26", 3) == 0)	layerInfo.bFloor_Fxx [26] = true;
				if (strncmp (tok4, "F27", 3) == 0)	layerInfo.bFloor_Fxx [27] = true;
				if (strncmp (tok4, "F28", 3) == 0)	layerInfo.bFloor_Fxx [28] = true;
				if (strncmp (tok4, "F29", 3) == 0)	layerInfo.bFloor_Fxx [29] = true;
				if (strncmp (tok4, "F30", 3) == 0)	layerInfo.bFloor_Fxx [30] = true;
				if (strncmp (tok4, "F31", 3) == 0)	layerInfo.bFloor_Fxx [31] = true;
				if (strncmp (tok4, "F32", 3) == 0)	layerInfo.bFloor_Fxx [32] = true;
				if (strncmp (tok4, "F33", 3) == 0)	layerInfo.bFloor_Fxx [33] = true;
				if (strncmp (tok4, "F34", 3) == 0)	layerInfo.bFloor_Fxx [34] = true;
				if (strncmp (tok4, "F35", 3) == 0)	layerInfo.bFloor_Fxx [35] = true;
				if (strncmp (tok4, "F36", 3) == 0)	layerInfo.bFloor_Fxx [36] = true;
				if (strncmp (tok4, "F37", 3) == 0)	layerInfo.bFloor_Fxx [37] = true;
				if (strncmp (tok4, "F38", 3) == 0)	layerInfo.bFloor_Fxx [38] = true;
				if (strncmp (tok4, "F39", 3) == 0)	layerInfo.bFloor_Fxx [39] = true;
				if (strncmp (tok4, "F40", 3) == 0)	layerInfo.bFloor_Fxx [40] = true;
				if (strncmp (tok4, "F41", 3) == 0)	layerInfo.bFloor_Fxx [41] = true;
				if (strncmp (tok4, "F42", 3) == 0)	layerInfo.bFloor_Fxx [42] = true;
				if (strncmp (tok4, "F43", 3) == 0)	layerInfo.bFloor_Fxx [43] = true;
				if (strncmp (tok4, "F44", 3) == 0)	layerInfo.bFloor_Fxx [44] = true;
				if (strncmp (tok4, "F45", 3) == 0)	layerInfo.bFloor_Fxx [45] = true;
				if (strncmp (tok4, "F46", 3) == 0)	layerInfo.bFloor_Fxx [46] = true;
				if (strncmp (tok4, "F47", 3) == 0)	layerInfo.bFloor_Fxx [47] = true;
				if (strncmp (tok4, "F48", 3) == 0)	layerInfo.bFloor_Fxx [48] = true;
				if (strncmp (tok4, "F49", 3) == 0)	layerInfo.bFloor_Fxx [49] = true;
				if (strncmp (tok4, "F50", 3) == 0)	layerInfo.bFloor_Fxx [50] = true;
				if (strncmp (tok4, "F51", 3) == 0)	layerInfo.bFloor_Fxx [51] = true;
				if (strncmp (tok4, "F52", 3) == 0)	layerInfo.bFloor_Fxx [52] = true;
				if (strncmp (tok4, "F53", 3) == 0)	layerInfo.bFloor_Fxx [53] = true;
				if (strncmp (tok4, "F54", 3) == 0)	layerInfo.bFloor_Fxx [54] = true;
				if (strncmp (tok4, "F55", 3) == 0)	layerInfo.bFloor_Fxx [55] = true;
				if (strncmp (tok4, "F56", 3) == 0)	layerInfo.bFloor_Fxx [56] = true;
				if (strncmp (tok4, "F57", 3) == 0)	layerInfo.bFloor_Fxx [57] = true;
				if (strncmp (tok4, "F58", 3) == 0)	layerInfo.bFloor_Fxx [58] = true;
				if (strncmp (tok4, "F59", 3) == 0)	layerInfo.bFloor_Fxx [59] = true;
				if (strncmp (tok4, "F60", 3) == 0)	layerInfo.bFloor_Fxx [60] = true;
				if (strncmp (tok4, "F61", 3) == 0)	layerInfo.bFloor_Fxx [61] = true;
				if (strncmp (tok4, "F62", 3) == 0)	layerInfo.bFloor_Fxx [62] = true;
				if (strncmp (tok4, "F63", 3) == 0)	layerInfo.bFloor_Fxx [63] = true;
				if (strncmp (tok4, "F64", 3) == 0)	layerInfo.bFloor_Fxx [64] = true;
				if (strncmp (tok4, "F65", 3) == 0)	layerInfo.bFloor_Fxx [65] = true;
				if (strncmp (tok4, "F66", 3) == 0)	layerInfo.bFloor_Fxx [66] = true;
				if (strncmp (tok4, "F67", 3) == 0)	layerInfo.bFloor_Fxx [67] = true;
				if (strncmp (tok4, "F68", 3) == 0)	layerInfo.bFloor_Fxx [68] = true;
				if (strncmp (tok4, "F69", 3) == 0)	layerInfo.bFloor_Fxx [69] = true;
				if (strncmp (tok4, "F70", 3) == 0)	layerInfo.bFloor_Fxx [70] = true;
				if (strncmp (tok4, "F71", 3) == 0)	layerInfo.bFloor_Fxx [71] = true;
				if (strncmp (tok4, "F72", 3) == 0)	layerInfo.bFloor_Fxx [72] = true;
				if (strncmp (tok4, "F73", 3) == 0)	layerInfo.bFloor_Fxx [73] = true;
				if (strncmp (tok4, "F74", 3) == 0)	layerInfo.bFloor_Fxx [74] = true;
				if (strncmp (tok4, "F75", 3) == 0)	layerInfo.bFloor_Fxx [75] = true;
				if (strncmp (tok4, "F76", 3) == 0)	layerInfo.bFloor_Fxx [76] = true;
				if (strncmp (tok4, "F77", 3) == 0)	layerInfo.bFloor_Fxx [77] = true;
				if (strncmp (tok4, "F78", 3) == 0)	layerInfo.bFloor_Fxx [78] = true;
				if (strncmp (tok4, "F79", 3) == 0)	layerInfo.bFloor_Fxx [79] = true;
				if (strncmp (tok4, "F80", 3) == 0)	layerInfo.bFloor_Fxx [80] = true;
				if (strncmp (tok4, "F81", 3) == 0)	layerInfo.bFloor_Fxx [81] = true;
				if (strncmp (tok4, "F82", 3) == 0)	layerInfo.bFloor_Fxx [82] = true;
				if (strncmp (tok4, "F83", 3) == 0)	layerInfo.bFloor_Fxx [83] = true;
				if (strncmp (tok4, "F84", 3) == 0)	layerInfo.bFloor_Fxx [84] = true;
				if (strncmp (tok4, "F85", 3) == 0)	layerInfo.bFloor_Fxx [85] = true;
				if (strncmp (tok4, "F86", 3) == 0)	layerInfo.bFloor_Fxx [86] = true;
				if (strncmp (tok4, "F87", 3) == 0)	layerInfo.bFloor_Fxx [87] = true;
				if (strncmp (tok4, "F88", 3) == 0)	layerInfo.bFloor_Fxx [88] = true;
				if (strncmp (tok4, "F89", 3) == 0)	layerInfo.bFloor_Fxx [89] = true;
				if (strncmp (tok4, "F90", 3) == 0)	layerInfo.bFloor_Fxx [90] = true;
				if (strncmp (tok4, "F91", 3) == 0)	layerInfo.bFloor_Fxx [91] = true;
				if (strncmp (tok4, "F92", 3) == 0)	layerInfo.bFloor_Fxx [92] = true;
				if (strncmp (tok4, "F93", 3) == 0)	layerInfo.bFloor_Fxx [93] = true;
				if (strncmp (tok4, "F94", 3) == 0)	layerInfo.bFloor_Fxx [94] = true;
				if (strncmp (tok4, "F95", 3) == 0)	layerInfo.bFloor_Fxx [95] = true;
				if (strncmp (tok4, "F96", 3) == 0)	layerInfo.bFloor_Fxx [96] = true;
				if (strncmp (tok4, "F97", 3) == 0)	layerInfo.bFloor_Fxx [97] = true;
				if (strncmp (tok4, "F98", 3) == 0)	layerInfo.bFloor_Fxx [98] = true;
				if (strncmp (tok4, "F99", 3) == 0)	layerInfo.bFloor_Fxx [99] = true;

				if (strncmp (tok4, "PH1", 3) == 0)	layerInfo.bFloor_PHxx [1] = true;
				if (strncmp (tok4, "PH2", 3) == 0)	layerInfo.bFloor_PHxx [2] = true;
				if (strncmp (tok4, "PH3", 3) == 0)	layerInfo.bFloor_PHxx [3] = true;
				if (strncmp (tok4, "PH4", 3) == 0)	layerInfo.bFloor_PHxx [4] = true;
				if (strncmp (tok4, "PH5", 3) == 0)	layerInfo.bFloor_PHxx [5] = true;
				if (strncmp (tok4, "PH6", 3) == 0)	layerInfo.bFloor_PHxx [6] = true;
				if (strncmp (tok4, "PH7", 3) == 0)	layerInfo.bFloor_PHxx [7] = true;
				if (strncmp (tok4, "PH8", 3) == 0)	layerInfo.bFloor_PHxx [8] = true;
				if (strncmp (tok4, "PH9", 3) == 0)	layerInfo.bFloor_PHxx [9] = true;

				// 5단계. CJ 구간
				CJNumber = atoi (tok5);
				if (CJNumber != 0)	layerInfo.bCJ [CJNumber] = true;

				// 6단계. CJ 속 시공순서
				OrderInCJNumber = atoi (tok6);
				if (OrderInCJNumber != 0)	layerInfo.bOrderInCJ [OrderInCJNumber] = true;

				// 7단계. 부재 및 객체 구분
				// 01-S 구조
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "STAR", 4) == 0))	layerInfo.bType_01_S_STAR = true;	// 계단
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "COLU", 4) == 0))	layerInfo.bType_01_S_COLU = true;	// 기둥
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "FOOT", 4) == 0))	layerInfo.bType_01_S_FOOT = true;	// 기초
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "WALL", 4) == 0))	layerInfo.bType_01_S_WALL = true;	// 벽체
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BEAM", 4) == 0))	layerInfo.bType_01_S_BEAM = true;	// 보
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "SLAB", 4) == 0))	layerInfo.bType_01_S_SLAB = true;	// 슬래브
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CLST", 4) == 0))	layerInfo.bType_01_S_CLST = true;	// 철골기둥
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BMST", 4) == 0))	layerInfo.bType_01_S_BMST = true;	// 철골보
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "RAMP", 4) == 0))	layerInfo.bType_01_S_RAMP = true;	// 램프
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CWAL", 4) == 0))	layerInfo.bType_01_S_CWAL = true;	// 합벽
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "WTWL", 4) == 0))	layerInfo.bType_01_S_WTWL = true;	// 방수턱
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CSTN", 4) == 0))	layerInfo.bType_01_S_CSTN = true;	// 연석
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "MPAD", 4) == 0))	layerInfo.bType_01_S_MPAD = true;	// 장비패드
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "GADN", 4) == 0))	layerInfo.bType_01_S_GADN = true;	// 화단
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "PARA", 4) == 0))	layerInfo.bType_01_S_PARA = true;	// 파라펫
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CLPC", 4) == 0))	layerInfo.bType_01_S_CLPC = true;	// PC기둥
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BMPC", 4) == 0))	layerInfo.bType_01_S_BMPC = true;	// 보PC
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BMWL", 4) == 0))	layerInfo.bType_01_S_BMWL = true;	// 보벽체
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "STST", 4) == 0))	layerInfo.bType_01_S_STST = true;	// 철골계단

				// 02-A 건축마감
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "FURN", 4) == 0))	layerInfo.bType_02_A_FURN = true;	// 가구
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "INSU", 4) == 0))	layerInfo.bType_02_A_INSU = true;	// 단열재
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PAIN", 4) == 0))	layerInfo.bType_02_A_PAIN = true;	// 도전
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "MOLD", 4) == 0))	layerInfo.bType_02_A_MOLD = true;	// 몰딩
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "MORT", 4) == 0))	layerInfo.bType_02_A_MORT = true;	// 몰탈
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "WATE", 4) == 0))	layerInfo.bType_02_A_WATE = true;	// 방수
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "BRIC", 4) == 0))	layerInfo.bType_02_A_BRIC = true;	// 벽돌
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PAPE", 4) == 0))	layerInfo.bType_02_A_PAPE = true;	// 벽지
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "BLOC", 4) == 0))	layerInfo.bType_02_A_BLOC = true;	// 블록
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "GYPS", 4) == 0))	layerInfo.bType_02_A_GYPS = true;	// 석고보드
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "STON", 4) == 0))	layerInfo.bType_02_A_STON = true;	// 석재
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "INTE", 4) == 0))	layerInfo.bType_02_A_INTE = true;	// 수장
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "GLAS", 4) == 0))	layerInfo.bType_02_A_GLAS = true;	// 유리
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "HARD", 4) == 0))	layerInfo.bType_02_A_HARD = true;	// 철물
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "TILE", 4) == 0))	layerInfo.bType_02_A_TILE = true;	// 타일
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PANE", 4) == 0))	layerInfo.bType_02_A_PANE = true;	// 판넬
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PLYW", 4) == 0))	layerInfo.bType_02_A_PLYW = true;	// 합판
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PCON", 4) == 0))	layerInfo.bType_02_A_PCON = true;	// 무근콘크리트

				// 05-T 가설재
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "TUNB", 4) == 0))	layerInfo.bType_05_T_TIMB = true;	// 각재
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BIMJ", 4) == 0))	layerInfo.bType_05_T_BIMJ = true;	// 각파이프행거
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BDCM", 4) == 0))	layerInfo.bType_05_T_BDCM = true;	// 기둥밴드
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "DMGA", 4) == 0))	layerInfo.bType_05_T_DMGA = true;	// 다마가
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "RIBL", 4) == 0))	layerInfo.bType_05_T_RIBL = true;	// 리브라스
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BMSP", 4) == 0))	layerInfo.bType_05_T_BMSP = true;	// 보받침필러
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BSTA", 4) == 0))	layerInfo.bType_05_T_BSTA = true;	// 비계계단
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SPIP", 4) == 0))	layerInfo.bType_05_T_SPIP = true;	// 사각파이프
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SUPT", 4) == 0))	layerInfo.bType_05_T_SUPT = true;	// 서포트
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SYSU", 4) == 0))	layerInfo.bType_05_T_SYSU = true;	// 시스템서포트
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "OUTA", 4) == 0))	layerInfo.bType_05_T_OUTA = true;	// 아웃코너앵글
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "OUTP", 4) == 0))	layerInfo.bType_05_T_OUTP = true;	// 아웃코너판넬
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "AFOM", 4) == 0))	layerInfo.bType_05_T_AFOM = true;	// 알폼
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "CPIP", 4) == 0))	layerInfo.bType_05_T_CPIP = true;	// 원형파이프
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "UGON", 4) == 0))	layerInfo.bType_05_T_UGON = true;	// 유공발판
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "UFOM", 4) == 0))	layerInfo.bType_05_T_UFOM = true;	// 유로폼
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "INCO", 4) == 0))	layerInfo.bType_05_T_INCO = true;	// 인코너판넬
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "JOIB", 4) == 0))	layerInfo.bType_05_T_JOIB = true;	// 조인트바
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "EFOM", 4) == 0))	layerInfo.bType_05_T_EFOM = true;	// 종이거푸집
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "JSUPT", 4) == 0))	layerInfo.bType_05_T_JSUPT = true;	// 잭서포트
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "WTST", 4) == 0))	layerInfo.bType_05_T_WTST = true;	// 지수판
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "CLAM", 4) == 0))	layerInfo.bType_05_T_CLAM = true;	// 클램프
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "LUMB", 4) == 0))	layerInfo.bType_05_T_LUMB = true;	// 토류판
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "TRUS", 4) == 0))	layerInfo.bType_05_T_TRUS = true;	// 트러스
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "TBBM", 4) == 0))	layerInfo.bType_05_T_TBBM = true;	// 팀버빔
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BCWF", 4) == 0))	layerInfo.bType_05_T_BCWF = true;	// 합벽지지대
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "PLYW", 4) == 0))	layerInfo.bType_05_T_PLYW = true;	// 합판
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "FISP", 4) == 0))	layerInfo.bType_05_T_FISP = true;	// 휠러스페이서
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "STSE", 4) == 0))	layerInfo.bType_05_T_STSE = true;	// 계단제작폼
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SLSE", 4) == 0))	layerInfo.bType_05_T_SLSE = true;	// 슬래브제작폼
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "RAIL", 4) == 0))	layerInfo.bType_05_T_RAIL = true;	// 목심레일

				// 06-F 가시설
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "STRU", 4) == 0))	layerInfo.bType_06_F_STRU = true;	// 수평H형강
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "HFIL", 4) == 0))	layerInfo.bType_06_F_HFIL = true;	// 수직H형강
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "SJAK", 4) == 0))	layerInfo.bType_06_F_SJAK = true;	// 스크류잭
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "PJAK", 4) == 0))	layerInfo.bType_06_F_PJAK = true;	// 프리로딩잭
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "BRKT", 4) == 0))	layerInfo.bType_06_F_BRKT = true;	// 브라켓
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "PBKT", 4) == 0))	layerInfo.bType_06_F_PBKT = true;	// 피스 브라켓
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "CIP", 4) == 0))	layerInfo.bType_06_F_CIP = true;	// 흙막이 벽체
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "LAND", 4) == 0))	layerInfo.bType_06_F_LAND = true;	// 대지
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "ANGL", 4) == 0))	layerInfo.bType_06_F_ANGL = true;	// 앵글
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "ERAC", 4) == 0))	layerInfo.bType_06_F_ERAC = true;	// 지반앵커
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "LUMB", 4) == 0))	layerInfo.bType_06_F_LUMB = true;	// 도류판
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "BPAN", 4) == 0))	layerInfo.bType_06_F_BPAN = true;	// 복공판
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "WALE", 4) == 0))	layerInfo.bType_06_F_WALE = true;	// 띠장
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "PILE", 4) == 0))	layerInfo.bType_06_F_PILE = true;	// 파일
			}
		}
		if (err == APIERR_DELETED)
			err = NoError;
	}

	// [다이얼로그 박스] 레이어 보여주기
	result = DGBlankModalDialog (700, 450, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerShowHandler, 0);

	// 모든 레이어 숨기기
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	
	if (result == DG_OK) {
		for (xx = 1; xx <= nLayers ; ++xx) {
			attrib.header.index = xx;
			err = ACAPI_Attribute_Get (&attrib);
			if (err == NoError) {
				//if ((attrib.layer.head.flags & APILay_Hidden) == false) {
					attrib.layer.head.flags |= APILay_Hidden;
					ACAPI_Attribute_Modify (&attrib, NULL);
				//}
			}
		}
	}

	short	z;
	char	code1 [10][5];		// 공사 코드
	short	LenCode1;
	char	code2 [1600][5];	// 동 코드
	short	LenCode2;
	char	code3 [120][5];		// 층 코드
	short	LenCode3;
	char	code4 [100][5];		// CJ 코드
	short	LenCode4;
	char	code5 [100][5];		// CJ 속 시공순서 코드
	short	LenCode5;
	char	code6 [90][5];		// 부재 코드
	short	LenCode6;

	char	fullLayerName [30];
	short	x1, x2, x3, x4, x5, x6;

	// 1. 공사 코드 문자열 만들기
	z = 0;
	if (selectedInfo.bType_01_S == true)	strcpy (code1 [z++], "01-S");
	if (selectedInfo.bType_02_A == true)	strcpy (code1 [z++], "02-A");
	if (selectedInfo.bType_03_M == true)	strcpy (code1 [z++], "03-M");
	if (selectedInfo.bType_04_E == true)	strcpy (code1 [z++], "04-E");
	if (selectedInfo.bType_05_T == true)	strcpy (code1 [z++], "05-T");
	if (selectedInfo.bType_06_F == true)	strcpy (code1 [z++], "06-F");
	if (selectedInfo.bType_07_Q == true)	strcpy (code1 [z++], "07-Q");
	if (selectedInfo.bType_08_L == true)	strcpy (code1 [z++], "08-L");
	if (selectedInfo.bType_09_C == true)	strcpy (code1 [z++], "09-C");
	if (selectedInfo.bType_10_K == true)	strcpy (code1 [z++], "10-K");
	LenCode1 = z;

	// 2. 동 코드 문자열 만들기
	z = 0;
	strcpy (code2 [z++], "????");
	for (xx = 1 ; xx < 100 ; ++xx) {
		if (selectedInfo.bDong_1xx [xx] == true)	sprintf (code2 [z++], "%04d", 100 + xx);
		if (selectedInfo.bDong_2xx [xx] == true)	sprintf (code2 [z++], "%04d", 200 + xx);
		if (selectedInfo.bDong_3xx [xx] == true)	sprintf (code2 [z++], "%04d", 300 + xx);
		if (selectedInfo.bDong_4xx [xx] == true)	sprintf (code2 [z++], "%04d", 400 + xx);
		if (selectedInfo.bDong_5xx [xx] == true)	sprintf (code2 [z++], "%04d", 500 + xx);
		if (selectedInfo.bDong_6xx [xx] == true)	sprintf (code2 [z++], "%04d", 600 + xx);
		if (selectedInfo.bDong_7xx [xx] == true)	sprintf (code2 [z++], "%04d", 700 + xx);
		if (selectedInfo.bDong_8xx [xx] == true)	sprintf (code2 [z++], "%04d", 800 + xx);
		if (selectedInfo.bDong_9xx [xx] == true)	sprintf (code2 [z++], "%04d", 900 + xx);
		if (selectedInfo.bDong_10xx [xx] == true)	sprintf (code2 [z++], "%04d", 1000 + xx);
		if (selectedInfo.bDong_11xx [xx] == true)	sprintf (code2 [z++], "%04d", 1100 + xx);
		if (selectedInfo.bDong_12xx [xx] == true)	sprintf (code2 [z++], "%04d", 1200 + xx);
		if (selectedInfo.bDong_13xx [xx] == true)	sprintf (code2 [z++], "%04d", 1300 + xx);
		if (selectedInfo.bDong_14xx [xx] == true)	sprintf (code2 [z++], "%04d", 1400 + xx);
		if (selectedInfo.bDong_15xx [xx] == true)	sprintf (code2 [z++], "%04d", 1500 + xx);
	}
	if (selectedInfo.bDong_SHOP == true)	strcpy (code2 [z++], "SHOP");
	if (selectedInfo.bDong_SECU == true)	strcpy (code2 [z++], "SECU");
	LenCode2 = z;

	// 3. 층 코드 문자열 만들기
	z = 0;
	if (selectedInfo.bFloor_Bxx [1] == true)	strcpy (code3 [z++], "9B1");
	if (selectedInfo.bFloor_Bxx [2] == true)	strcpy (code3 [z++], "8B2");
	if (selectedInfo.bFloor_Bxx [3] == true)	strcpy (code3 [z++], "7B3");
	if (selectedInfo.bFloor_Bxx [4] == true)	strcpy (code3 [z++], "6B4");
	if (selectedInfo.bFloor_Bxx [5] == true)	strcpy (code3 [z++], "5B5");
	if (selectedInfo.bFloor_Bxx [6] == true)	strcpy (code3 [z++], "4B6");
	if (selectedInfo.bFloor_Bxx [7] == true)	strcpy (code3 [z++], "3B7");
	if (selectedInfo.bFloor_Bxx [8] == true)	strcpy (code3 [z++], "2B8");
	if (selectedInfo.bFloor_Bxx [9] == true)	strcpy (code3 [z++], "1B9");
	for (xx = 1 ; xx < 100 ; ++xx) {
		if (selectedInfo.bFloor_Fxx [xx] == true)	sprintf (code3 [z++], "F%02d", xx);
	}
	for (xx = 1 ; xx < 10 ; ++xx) {
		if (selectedInfo.bFloor_PHxx [xx] == true)	sprintf (code3 [z++], "PH%1d", xx);
	}
	LenCode3 = z;

	// 4. CJ 코드 문자열 만들기
	z = 0;
	strcpy (code4 [z++], "??");
	for (xx = 1 ; xx < 100 ; ++xx) {
		if (selectedInfo.bCJ [xx] == true)	sprintf (code4 [z++], "%02d", xx);
	}
	LenCode4 = z;

	// 5. CJ 속 시공순서 문자열 만들기
	z = 0;
	strcpy (code5 [z++], "??");
	for (xx = 1 ; xx < 100 ; ++xx) {
		if (selectedInfo.bOrderInCJ [xx] == true)	sprintf (code5 [z++], "%02d", xx);
	}
	LenCode5 = z;

	// 6. 부재 코드 문자열 만들기
	z = 0;
	if (selectedInfo.bType_01_S_STAR == true)	strcpy (code6 [z++], "STAR");
	if (selectedInfo.bType_01_S_COLU == true)	strcpy (code6 [z++], "COLU");
	if (selectedInfo.bType_01_S_FOOT == true)	strcpy (code6 [z++], "FOOT");
	if (selectedInfo.bType_01_S_WALL == true)	strcpy (code6 [z++], "WALL");
	if (selectedInfo.bType_01_S_BEAM == true)	strcpy (code6 [z++], "BEAM");
	if (selectedInfo.bType_01_S_SLAB == true)	strcpy (code6 [z++], "SLAB");
	if (selectedInfo.bType_01_S_CLST == true)	strcpy (code6 [z++], "CLST");
	if (selectedInfo.bType_01_S_BMST == true)	strcpy (code6 [z++], "BMST");
	if (selectedInfo.bType_01_S_RAMP == true)	strcpy (code6 [z++], "RAMP");
	if (selectedInfo.bType_01_S_CWAL == true)	strcpy (code6 [z++], "CWAL");
	if (selectedInfo.bType_01_S_WTWL == true)	strcpy (code6 [z++], "WTWL");
	if (selectedInfo.bType_01_S_CSTN == true)	strcpy (code6 [z++], "CSTN");
	if (selectedInfo.bType_01_S_MPAD == true)	strcpy (code6 [z++], "MPAD");
	if (selectedInfo.bType_01_S_GADN == true)	strcpy (code6 [z++], "GADN");
	if (selectedInfo.bType_01_S_PARA == true)	strcpy (code6 [z++], "PARA");
	if (selectedInfo.bType_01_S_CLPC == true)	strcpy (code6 [z++], "CLPC");
	if (selectedInfo.bType_01_S_BMPC == true)	strcpy (code6 [z++], "BMPC");
	if (selectedInfo.bType_01_S_BMWL == true)	strcpy (code6 [z++], "BMWL");
	if (selectedInfo.bType_01_S_STST == true)	strcpy (code6 [z++], "STST");

	if (selectedInfo.bType_02_A_FURN == true)	strcpy (code6 [z++], "FURN");
	if (selectedInfo.bType_02_A_INSU == true)	strcpy (code6 [z++], "INSU");
	if (selectedInfo.bType_02_A_PAIN == true)	strcpy (code6 [z++], "PAIN");
	if (selectedInfo.bType_02_A_MOLD == true)	strcpy (code6 [z++], "MOLD");
	if (selectedInfo.bType_02_A_MORT == true)	strcpy (code6 [z++], "MORT");
	if (selectedInfo.bType_02_A_WATE == true)	strcpy (code6 [z++], "WATE");
	if (selectedInfo.bType_02_A_BRIC == true)	strcpy (code6 [z++], "BRIC");
	if (selectedInfo.bType_02_A_PAPE == true)	strcpy (code6 [z++], "PAPE");
	if (selectedInfo.bType_02_A_BLOC == true)	strcpy (code6 [z++], "BLOC");
	if (selectedInfo.bType_02_A_GYPS == true)	strcpy (code6 [z++], "GYPS");
	if (selectedInfo.bType_02_A_STON == true)	strcpy (code6 [z++], "STON");
	if (selectedInfo.bType_02_A_INTE == true)	strcpy (code6 [z++], "INTE");
	if (selectedInfo.bType_02_A_GLAS == true)	strcpy (code6 [z++], "GLAS");
	if (selectedInfo.bType_02_A_HARD == true)	strcpy (code6 [z++], "HARD");
	if (selectedInfo.bType_02_A_TILE == true)	strcpy (code6 [z++], "TILE");
	if (selectedInfo.bType_02_A_PANE == true)	strcpy (code6 [z++], "PANE");
	if (selectedInfo.bType_02_A_PLYW == true)	strcpy (code6 [z++], "PLYW");
	if (selectedInfo.bType_02_A_PCON == true)	strcpy (code6 [z++], "PCON");

	if (selectedInfo.bType_05_T_TIMB == true)	strcpy (code6 [z++], "TIMB");
	if (selectedInfo.bType_05_T_BIMJ == true)	strcpy (code6 [z++], "BIMJ");
	if (selectedInfo.bType_05_T_BDCM == true)	strcpy (code6 [z++], "BDCM");
	if (selectedInfo.bType_05_T_DMGA == true)	strcpy (code6 [z++], "DMGA");
	if (selectedInfo.bType_05_T_RIBL == true)	strcpy (code6 [z++], "RIBL");
	if (selectedInfo.bType_05_T_BMSP == true)	strcpy (code6 [z++], "BMSP");
	if (selectedInfo.bType_05_T_BSTA == true)	strcpy (code6 [z++], "BSTA");
	if (selectedInfo.bType_05_T_SPIP == true)	strcpy (code6 [z++], "SPIP");
	if (selectedInfo.bType_05_T_SUPT == true)	strcpy (code6 [z++], "SUPT");
	if (selectedInfo.bType_05_T_SYSU == true)	strcpy (code6 [z++], "SYSU");
	if (selectedInfo.bType_05_T_OUTA == true)	strcpy (code6 [z++], "OUTA");
	if (selectedInfo.bType_05_T_OUTP == true)	strcpy (code6 [z++], "OUTP");
	if (selectedInfo.bType_05_T_AFOM == true)	strcpy (code6 [z++], "AFOM");
	if (selectedInfo.bType_05_T_CPIP == true)	strcpy (code6 [z++], "CPIP");
	if (selectedInfo.bType_05_T_UGON == true)	strcpy (code6 [z++], "UGON");
	if (selectedInfo.bType_05_T_UFOM == true)	strcpy (code6 [z++], "UFOM");
	if (selectedInfo.bType_05_T_INCO == true)	strcpy (code6 [z++], "INCO");
	if (selectedInfo.bType_05_T_JOIB == true)	strcpy (code6 [z++], "JOIB");
	if (selectedInfo.bType_05_T_EFOM == true)	strcpy (code6 [z++], "EFOM");
	if (selectedInfo.bType_05_T_JSUPT == true)	strcpy (code6 [z++], "JSUPT");
	if (selectedInfo.bType_05_T_WTST == true)	strcpy (code6 [z++], "WTST");
	if (selectedInfo.bType_05_T_CLAM == true)	strcpy (code6 [z++], "CLAM");
	if (selectedInfo.bType_05_T_LUMB == true)	strcpy (code6 [z++], "LUMB");
	if (selectedInfo.bType_05_T_TRUS == true)	strcpy (code6 [z++], "TRUS");
	if (selectedInfo.bType_05_T_TBBM == true)	strcpy (code6 [z++], "TBBM");
	if (selectedInfo.bType_05_T_BCWF == true)	strcpy (code6 [z++], "BCWF");
	if (selectedInfo.bType_05_T_PLYW == true)	strcpy (code6 [z++], "PLYW");
	if (selectedInfo.bType_05_T_FISP == true)	strcpy (code6 [z++], "FISP");
	if (selectedInfo.bType_05_T_STSE == true)	strcpy (code6 [z++], "STSE");
	if (selectedInfo.bType_05_T_SLSE == true)	strcpy (code6 [z++], "SLSE");
	if (selectedInfo.bType_05_T_RAIL == true)	strcpy (code6 [z++], "RAIL");

	if (selectedInfo.bType_06_F_STRU == true)	strcpy (code6 [z++], "STRU");
	if (selectedInfo.bType_06_F_HFIL == true)	strcpy (code6 [z++], "HFIL");
	if (selectedInfo.bType_06_F_SJAK == true)	strcpy (code6 [z++], "SJAK");
	if (selectedInfo.bType_06_F_PJAK == true)	strcpy (code6 [z++], "PJAK");
	if (selectedInfo.bType_06_F_BRKT == true)	strcpy (code6 [z++], "BRKT");
	if (selectedInfo.bType_06_F_PBKT == true)	strcpy (code6 [z++], "PBKT");
	if (selectedInfo.bType_06_F_CIP == true)	strcpy (code6 [z++], "CIP");
	if (selectedInfo.bType_06_F_LAND == true)	strcpy (code6 [z++], "LAND");
	if (selectedInfo.bType_06_F_ANGL == true)	strcpy (code6 [z++], "ANGL");
	if (selectedInfo.bType_06_F_ERAC == true)	strcpy (code6 [z++], "ERAC");
	if (selectedInfo.bType_06_F_LUMB == true)	strcpy (code6 [z++], "LUMB");
	if (selectedInfo.bType_06_F_BPAN == true)	strcpy (code6 [z++], "BPAN");
	if (selectedInfo.bType_06_F_WALE == true)	strcpy (code6 [z++], "WALE");
	if (selectedInfo.bType_06_F_PILE == true)	strcpy (code6 [z++], "PILE");
	LenCode6 = z;

	// 레이어 이름 조합하기
	for (x1 = 0 ; x1 < LenCode1 ; ++x1) {
		for (x2 = 0 ; x2 < LenCode2 ; ++x2) {
			for (x3 = 0 ; x3 < LenCode3 ; ++x3) {
				for (x4 = 0 ; x4 < LenCode4 ; ++x4) {
					for (x5 = 0 ; x5 < LenCode5 ; ++x5) {
						for (x6 = 0 ; x6 < LenCode6 ; ++x6) {
							// 공사 구분
							strcpy (fullLayerName, "");
							strcpy (fullLayerName, code1 [x1]);

							// 동 구분
							if (LenCode2 > 1) {
								strcat (fullLayerName, "-");
								strcat (fullLayerName, code2 [x2]);
							}

							// 층 구분
							strcat (fullLayerName, "-");
							strcat (fullLayerName, code3 [x3]);

							// CJ 구간
							if (LenCode4 > 1) {
								strcat (fullLayerName, "-");
								strcat (fullLayerName, code4 [x4]);
							}

							// CJ 속 시공순서
							if (LenCode5 > 1) {
								strcat (fullLayerName, "-");
								strcat (fullLayerName, code5 [x5]);
							}

							// 부재
							strcat (fullLayerName, "-");
							strcat (fullLayerName, code6 [x6]);

							// !!!
							//ACAPI_WriteReport (fullLayerName, true);

							// 조합한 레이어 이름 검색하기
							BNZeroMemory (&attrib, sizeof (API_Attribute));
							attrib.header.typeID = API_LayerID;
							CHCopyC (fullLayerName, attrib.header.name);
							err = ACAPI_Attribute_Get (&attrib);

							// 해당 레이어 보여주기
							if ((attrib.layer.head.flags & APILay_Hidden) == true) {
								attrib.layer.head.flags ^= APILay_Hidden;
								ACAPI_Attribute_Modify (&attrib, NULL);
							}
						}
					}
				}
			}
		}
	}
	
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return err;
}

// [다이얼로그 박스] 레이어 보여주기
short DGCALLBACK layerShowHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx;
	char	tempStr [20];
	short	dialogSizeX, dialogSizeY;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "레이어 쉽게 선택하기");

			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 20, 40, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 50, 40, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			// 라벨: 공사 구분
			itmPosX = 40;
			itmPosY = 25;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "공사 구분");
			DGShowItem (dialogID, itmIdx);

			// 체크박스: 공사 구분 버튼
			itmPosX = 150;
			itmPosY = 20;
			if (layerInfo.bType_01_S) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "01-S 구조");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "02-A 건축마감");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_03_M) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "03-M 기계설비");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_03_M = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_04_E) {	
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "04-E 전기설비");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_04_E = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "05-T 가설재");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "06-F 가시설");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_07_Q) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "07-Q 물량합판");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_07_Q = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_08_L) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "08-L 조경");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_08_L = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_09_C) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "09-C 토목");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_09_C = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_10_K) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "10-K 건설장비");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_10_K = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// 라벨: 동 구분
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "동 구분");
			DGShowItem (dialogID, itmIdx);

			// 체크박스: 동 구분 버튼
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_1xx [xx]) {
					sprintf (tempStr, "%d동", 100 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_1xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_2xx [xx]) {
					sprintf (tempStr, "%d동", 200 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_2xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_3xx [xx]) {
					sprintf (tempStr, "%d동", 300 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_3xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_4xx [xx]) {
					sprintf (tempStr, "%d동", 400 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_4xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_5xx [xx]) {
					sprintf (tempStr, "%d동", 500 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_5xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_6xx [xx]) {
					sprintf (tempStr, "%d동", 600 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_6xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_7xx [xx]) {
					sprintf (tempStr, "%d동", 700 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_7xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_8xx [xx]) {
					sprintf (tempStr, "%d동", 800 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_8xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_9xx [xx]) {
					sprintf (tempStr, "%d동", 900 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_9xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_10xx [xx]) {
					sprintf (tempStr, "%d동", 1000 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_10xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_11xx [xx]) {
					sprintf (tempStr, "%d동", 1100 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_11xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_12xx [xx]) {
					sprintf (tempStr, "%d동", 1200 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_12xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_13xx [xx]) {
					sprintf (tempStr, "%d동", 1300 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_13xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_14xx [xx]) {
					sprintf (tempStr, "%d동", 1400 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_14xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_15xx [xx]) {
					sprintf (tempStr, "%d동", 1500 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_15xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			if (layerInfo.bDong_SHOP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "근린공원");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iDong_SHOP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bDong_SECU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "경비실");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iDong_SECU = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// 라벨: 층 구분
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "층 구분");
			DGShowItem (dialogID, itmIdx);

			// 체크박스: 층 구분 버튼
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 9 ; xx >= 1 ; --xx) {
				if (layerInfo.bFloor_Bxx [xx]) {
					sprintf (tempStr, "지하%d층", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iFloor_Bxx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bFloor_Fxx [xx]) {
					sprintf (tempStr, "지상%d층", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iFloor_Fxx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 10 ; ++xx) {
				if (layerInfo.bFloor_PHxx [xx]) {
					sprintf (tempStr, "PH%d층", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iFloor_PHxx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}

			itmPosY += 30;

			// 라벨: CJ 구간
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "CJ 구간");
			DGShowItem (dialogID, itmIdx);

			// 체크박스: CJ 구간 버튼
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bCJ [xx]) {
					sprintf (tempStr, "%02d", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iCJ [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}

			itmPosY += 30;

			// 라벨: CJ 속 시공순서
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "시공순서");
			DGShowItem (dialogID, itmIdx);

			// 체크박스: CJ 속 시공순서 버튼
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bOrderInCJ [xx]) {
					sprintf (tempStr, "%02d", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iOrderInCJ [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}

			itmPosY += 30;

			// 라벨: 객체(구조)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "OBJ-구조");
			DGShowItem (dialogID, itmIdx);

			// 체크박스: 객체(구조)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_01_S_STAR) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "계단");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_STAR = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_COLU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "기둥");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_COLU = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_FOOT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "기초");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_FOOT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_WALL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "벽체");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_WALL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_BEAM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "보");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_BEAM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_SLAB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "슬래브");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_SLAB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_CLST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "철골기둥");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_CLST = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_BMST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "철골보");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_BMST = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_RAMP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "램프");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_RAMP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_CWAL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "합벽");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_CWAL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_WTWL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "방수턱");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_WTWL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_CSTN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "연석");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_CSTN = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_MPAD) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "장비패드");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_MPAD = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_GADN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "화단");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_GADN = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_PARA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "파라펫");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_PARA = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_CLPC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "PC기둥");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_CLPC = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_BMPC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "보PC");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_BMPC = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_BMWL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "보벽체");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_BMWL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_STST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "철골계단");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_STST = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// 라벨: 객체(건축마감)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "OBJ-건축마감");
			DGShowItem (dialogID, itmIdx);

			// 체크박스: 객체(건축마감)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_02_A_FURN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "가구");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_FURN = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_INSU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "단열재");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_INSU = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_PAIN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "도장");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_PAIN = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_MOLD) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "몰딩");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_MOLD = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_MORT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "몰탈");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_MORT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_WATE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "방수");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_WATE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_BRIC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "벽돌");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_BRIC = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_PAPE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "벽지");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_PAPE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_BLOC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "블록");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_BLOC = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_GYPS) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "석고보드");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_GYPS = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_STON) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "석재");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_STON = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_INTE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "수장");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_INTE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_GLAS) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "유리");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_GLAS = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_HARD) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "철물");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_HARD = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_TILE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "타일");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_TILE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_PANE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "판넬");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_PANE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_PLYW) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "합판");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_PLYW = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_PCON) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "무근콘크리트");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_PCON = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// 라벨: 객체(가설재)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "OBJ-가설재");
			DGShowItem (dialogID, itmIdx);

			// 체크박스: 객체(가설재)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_05_T_TIMB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "각재");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_TIMB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_BIMJ) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "각파이프행거");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_BIMJ = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_BDCM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "기둥밴드");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_BDCM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_DMGA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "다마가");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_DMGA = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_RIBL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "리브라스");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_RIBL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_BMSP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "보받침필러");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_BMSP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_BSTA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "비계계단");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_BSTA = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_SPIP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "사각파이프");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_SPIP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_SUPT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "서포트");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_SUPT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_SYSU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "시스템서포트");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_SYSU = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_OUTA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "아웃코너앵글");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_OUTA = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_OUTP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "아웃코너판넬");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_OUTP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_AFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "알폼");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_AFOM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_CPIP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "원형파이프");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_CPIP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_UGON) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "유공발판");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_UGON = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_UFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "유로폼");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_UFOM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_INCO) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "인코너판넬");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_INCO = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_JOIB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "조인트바");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_JOIB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_EFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "종이거푸집");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_EFOM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_JSUPT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "잭서포트");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_JSUPT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_WTST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "지수판");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_WTST = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_CLAM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "클램프");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_CLAM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_LUMB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "토류판");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_LUMB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_TRUS) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "트러스");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_TRUS = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_TBBM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "팀버빔");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_TBBM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_BCWF) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "합벽지지대");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_BCWF = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_PLYW) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "합판");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_PLYW = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_FISP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "휠러스페이서");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_FISP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_STSE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "계단제작폼");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_STSE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_SLSE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "슬래브제작폼");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_SLSE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_RAIL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "목심레일");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_RAIL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// 라벨: 객체(가시설)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "OBJ-가시설");
			DGShowItem (dialogID, itmIdx);

			// 체크박스: 객체(가시설)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_06_F_STRU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "수평H형강");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_STRU = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_HFIL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "수직H형강");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_HFIL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_SJAK) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "스크류잭");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_SJAK = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_PJAK) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "프리로딩잭");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_PJAK = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_BRKT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "브라켓");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_BRKT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_PBKT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "피스 브라켓");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_PBKT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_CIP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "흙막이 벽체");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_CIP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_LAND) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "대지");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_LAND = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_ANGL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "앵글");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_ANGL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_ERAC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "지반앵커");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_ERAC = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_LUMB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "토류판");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_LUMB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_BPAN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "복공판");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_BPAN = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_WALE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "띠장");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_WALE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_PILE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "파일");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_PILE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			dialogSizeX = 700;
			dialogSizeY = itmPosY + 150;
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;
		
		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 공사 구분
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S) == TRUE) ?	selectedInfo.bType_01_S = true : selectedInfo.bType_01_S = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A) == TRUE) ?	selectedInfo.bType_02_A = true : selectedInfo.bType_02_A = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_03_M) == TRUE) ?	selectedInfo.bType_03_M = true : selectedInfo.bType_03_M = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_04_E) == TRUE) ?	selectedInfo.bType_04_E = true : selectedInfo.bType_04_E = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T) == TRUE) ?	selectedInfo.bType_05_T = true : selectedInfo.bType_05_T = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F) == TRUE) ?	selectedInfo.bType_06_F = true : selectedInfo.bType_06_F = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_07_Q) == TRUE) ?	selectedInfo.bType_07_Q = true : selectedInfo.bType_07_Q = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_08_L) == TRUE) ?	selectedInfo.bType_08_L = true : selectedInfo.bType_08_L = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_09_C) == TRUE) ?	selectedInfo.bType_09_C = true : selectedInfo.bType_09_C = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_10_K) == TRUE) ?	selectedInfo.bType_10_K = true : selectedInfo.bType_10_K = false;
					
					// 동 구분
					for (xx = 1 ; xx < 100 ; ++xx) {
						(DGGetItemValLong (dialogID, layerInfo.iDong_1xx [xx]) == TRUE) ?	selectedInfo.bDong_1xx [xx] = true : selectedInfo.bDong_1xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_2xx [xx]) == TRUE) ?	selectedInfo.bDong_2xx [xx] = true : selectedInfo.bDong_2xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_3xx [xx]) == TRUE) ?	selectedInfo.bDong_3xx [xx] = true : selectedInfo.bDong_3xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_4xx [xx]) == TRUE) ?	selectedInfo.bDong_4xx [xx] = true : selectedInfo.bDong_4xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_5xx [xx]) == TRUE) ?	selectedInfo.bDong_5xx [xx] = true : selectedInfo.bDong_5xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_6xx [xx]) == TRUE) ?	selectedInfo.bDong_6xx [xx] = true : selectedInfo.bDong_6xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_7xx [xx]) == TRUE) ?	selectedInfo.bDong_7xx [xx] = true : selectedInfo.bDong_7xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_8xx [xx]) == TRUE) ?	selectedInfo.bDong_8xx [xx] = true : selectedInfo.bDong_8xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_9xx [xx]) == TRUE) ?	selectedInfo.bDong_9xx [xx] = true : selectedInfo.bDong_9xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_10xx [xx]) == TRUE) ?	selectedInfo.bDong_10xx [xx] = true : selectedInfo.bDong_10xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_11xx [xx]) == TRUE) ?	selectedInfo.bDong_11xx [xx] = true : selectedInfo.bDong_11xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_12xx [xx]) == TRUE) ?	selectedInfo.bDong_12xx [xx] = true : selectedInfo.bDong_12xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_13xx [xx]) == TRUE) ?	selectedInfo.bDong_13xx [xx] = true : selectedInfo.bDong_13xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_14xx [xx]) == TRUE) ?	selectedInfo.bDong_14xx [xx] = true : selectedInfo.bDong_14xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_15xx [xx]) == TRUE) ?	selectedInfo.bDong_15xx [xx] = true : selectedInfo.bDong_15xx [xx] = false;
					}

					(DGGetItemValLong (dialogID, layerInfo.iDong_SHOP) == TRUE) ?	selectedInfo.bDong_SHOP = true : selectedInfo.bDong_SHOP = false;
					(DGGetItemValLong (dialogID, layerInfo.iDong_SECU) == TRUE) ?	selectedInfo.bDong_SECU = true : selectedInfo.bDong_SECU = false;

					// 층 구분
					for (xx = 9 ; xx >= 1 ; --xx) {
						(DGGetItemValLong (dialogID, layerInfo.iFloor_Bxx [xx]) == TRUE) ?	selectedInfo.bFloor_Bxx [xx] = true : selectedInfo.bFloor_Bxx [xx] = false;
					}
					for (xx = 1 ; xx < 100 ; ++xx) {
						(DGGetItemValLong (dialogID, layerInfo.iFloor_Fxx [xx]) == TRUE) ?	selectedInfo.bFloor_Fxx [xx] = true : selectedInfo.bFloor_Fxx [xx] = false;
					}
					for (xx = 1 ; xx < 10 ; ++xx) {
						(DGGetItemValLong (dialogID, layerInfo.iFloor_PHxx [xx]) == TRUE) ?	selectedInfo.bFloor_PHxx [xx] = true : selectedInfo.bFloor_PHxx [xx] = false;
					}

					// CJ, CJ 속 시공순서
					for (xx = 1 ; xx < 100 ; ++xx) {
						(DGGetItemValLong (dialogID, layerInfo.iCJ [xx]) == TRUE) ?			selectedInfo.bCJ [xx] = true : selectedInfo.bCJ [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iOrderInCJ [xx]) == TRUE) ?	selectedInfo.bOrderInCJ [xx] = true : selectedInfo.bOrderInCJ [xx] = false;
					}

					// 01-S 구조
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_STAR) == TRUE) ?	selectedInfo.bType_01_S_STAR = true : selectedInfo.bType_01_S_STAR = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_COLU) == TRUE) ?	selectedInfo.bType_01_S_COLU = true : selectedInfo.bType_01_S_COLU = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_FOOT) == TRUE) ?	selectedInfo.bType_01_S_FOOT = true : selectedInfo.bType_01_S_FOOT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_WALL) == TRUE) ?	selectedInfo.bType_01_S_WALL = true : selectedInfo.bType_01_S_WALL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_BEAM) == TRUE) ?	selectedInfo.bType_01_S_BEAM = true : selectedInfo.bType_01_S_BEAM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_SLAB) == TRUE) ?	selectedInfo.bType_01_S_SLAB = true : selectedInfo.bType_01_S_SLAB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_CLST) == TRUE) ?	selectedInfo.bType_01_S_CLST = true : selectedInfo.bType_01_S_CLST = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_BMST) == TRUE) ?	selectedInfo.bType_01_S_BMST = true : selectedInfo.bType_01_S_BMST = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_RAMP) == TRUE) ?	selectedInfo.bType_01_S_RAMP = true : selectedInfo.bType_01_S_RAMP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_CWAL) == TRUE) ?	selectedInfo.bType_01_S_CWAL = true : selectedInfo.bType_01_S_CWAL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_WTWL) == TRUE) ?	selectedInfo.bType_01_S_WTWL = true : selectedInfo.bType_01_S_WTWL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_CSTN) == TRUE) ?	selectedInfo.bType_01_S_CSTN = true : selectedInfo.bType_01_S_CSTN = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_MPAD) == TRUE) ?	selectedInfo.bType_01_S_MPAD = true : selectedInfo.bType_01_S_MPAD = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_GADN) == TRUE) ?	selectedInfo.bType_01_S_GADN = true : selectedInfo.bType_01_S_GADN = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_PARA) == TRUE) ?	selectedInfo.bType_01_S_PARA = true : selectedInfo.bType_01_S_PARA = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_CLPC) == TRUE) ?	selectedInfo.bType_01_S_CLPC = true : selectedInfo.bType_01_S_CLPC = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_BMPC) == TRUE) ?	selectedInfo.bType_01_S_BMPC = true : selectedInfo.bType_01_S_BMPC = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_BMWL) == TRUE) ?	selectedInfo.bType_01_S_BMWL = true : selectedInfo.bType_01_S_BMWL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_STST) == TRUE) ?	selectedInfo.bType_01_S_STST = true : selectedInfo.bType_01_S_STST = false;

					// 02-A 건축마감
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_FURN) == TRUE) ?	selectedInfo.bType_02_A_FURN = true : selectedInfo.bType_02_A_FURN = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_INSU) == TRUE) ?	selectedInfo.bType_02_A_INSU = true : selectedInfo.bType_02_A_INSU = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_PAIN) == TRUE) ?	selectedInfo.bType_02_A_PAIN = true : selectedInfo.bType_02_A_PAIN = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_MOLD) == TRUE) ?	selectedInfo.bType_02_A_MOLD = true : selectedInfo.bType_02_A_MOLD = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_MORT) == TRUE) ?	selectedInfo.bType_02_A_MORT = true : selectedInfo.bType_02_A_MORT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_WATE) == TRUE) ?	selectedInfo.bType_02_A_WATE = true : selectedInfo.bType_02_A_WATE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_BRIC) == TRUE) ?	selectedInfo.bType_02_A_BRIC = true : selectedInfo.bType_02_A_BRIC = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_PAPE) == TRUE) ?	selectedInfo.bType_02_A_PAPE = true : selectedInfo.bType_02_A_PAPE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_BLOC) == TRUE) ?	selectedInfo.bType_02_A_BLOC = true : selectedInfo.bType_02_A_BLOC = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_GYPS) == TRUE) ?	selectedInfo.bType_02_A_GYPS = true : selectedInfo.bType_02_A_GYPS = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_STON) == TRUE) ?	selectedInfo.bType_02_A_STON = true : selectedInfo.bType_02_A_STON = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_INTE) == TRUE) ?	selectedInfo.bType_02_A_INTE = true : selectedInfo.bType_02_A_INTE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_GLAS) == TRUE) ?	selectedInfo.bType_02_A_GLAS = true : selectedInfo.bType_02_A_GLAS = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_HARD) == TRUE) ?	selectedInfo.bType_02_A_HARD = true : selectedInfo.bType_02_A_HARD = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_TILE) == TRUE) ?	selectedInfo.bType_02_A_TILE = true : selectedInfo.bType_02_A_TILE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_PANE) == TRUE) ?	selectedInfo.bType_02_A_PANE = true : selectedInfo.bType_02_A_PANE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_PLYW) == TRUE) ?	selectedInfo.bType_02_A_PLYW = true : selectedInfo.bType_02_A_PLYW = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_PCON) == TRUE) ?	selectedInfo.bType_02_A_PCON = true : selectedInfo.bType_02_A_PCON = false;
					
					// 05-T 가설재
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_TIMB) == TRUE) ?	selectedInfo.bType_05_T_TIMB = true : selectedInfo.bType_05_T_TIMB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_BIMJ) == TRUE) ?	selectedInfo.bType_05_T_BIMJ = true : selectedInfo.bType_05_T_BIMJ = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_BDCM) == TRUE) ?	selectedInfo.bType_05_T_BDCM = true : selectedInfo.bType_05_T_BDCM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_DMGA) == TRUE) ?	selectedInfo.bType_05_T_DMGA = true : selectedInfo.bType_05_T_DMGA = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_RIBL) == TRUE) ?	selectedInfo.bType_05_T_RIBL = true : selectedInfo.bType_05_T_RIBL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_BMSP) == TRUE) ?	selectedInfo.bType_05_T_BMSP = true : selectedInfo.bType_05_T_BMSP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_BSTA) == TRUE) ?	selectedInfo.bType_05_T_BSTA = true : selectedInfo.bType_05_T_BSTA = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_SPIP) == TRUE) ?	selectedInfo.bType_05_T_SPIP = true : selectedInfo.bType_05_T_SPIP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_SUPT) == TRUE) ?	selectedInfo.bType_05_T_SUPT = true : selectedInfo.bType_05_T_SUPT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_SYSU) == TRUE) ?	selectedInfo.bType_05_T_SYSU = true : selectedInfo.bType_05_T_SYSU = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_OUTA) == TRUE) ?	selectedInfo.bType_05_T_OUTA = true : selectedInfo.bType_05_T_OUTA = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_OUTP) == TRUE) ?	selectedInfo.bType_05_T_OUTP = true : selectedInfo.bType_05_T_OUTP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_AFOM) == TRUE) ?	selectedInfo.bType_05_T_AFOM = true : selectedInfo.bType_05_T_AFOM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_CPIP) == TRUE) ?	selectedInfo.bType_05_T_CPIP = true : selectedInfo.bType_05_T_CPIP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_UGON) == TRUE) ?	selectedInfo.bType_05_T_UGON = true : selectedInfo.bType_05_T_UGON = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_UFOM) == TRUE) ?	selectedInfo.bType_05_T_UFOM = true : selectedInfo.bType_05_T_UFOM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_INCO) == TRUE) ?	selectedInfo.bType_05_T_INCO = true : selectedInfo.bType_05_T_INCO = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_JOIB) == TRUE) ?	selectedInfo.bType_05_T_JOIB = true : selectedInfo.bType_05_T_JOIB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_EFOM) == TRUE) ?	selectedInfo.bType_05_T_EFOM = true : selectedInfo.bType_05_T_EFOM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_JSUPT) == TRUE) ?	selectedInfo.bType_05_T_JSUPT = true : selectedInfo.bType_05_T_JSUPT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_WTST) == TRUE) ?	selectedInfo.bType_05_T_WTST = true : selectedInfo.bType_05_T_WTST = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_CLAM) == TRUE) ?	selectedInfo.bType_05_T_CLAM = true : selectedInfo.bType_05_T_CLAM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_LUMB) == TRUE) ?	selectedInfo.bType_05_T_LUMB = true : selectedInfo.bType_05_T_LUMB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_TRUS) == TRUE) ?	selectedInfo.bType_05_T_TRUS = true : selectedInfo.bType_05_T_TRUS = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_TBBM) == TRUE) ?	selectedInfo.bType_05_T_TBBM = true : selectedInfo.bType_05_T_TBBM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_BCWF) == TRUE) ?	selectedInfo.bType_05_T_BCWF = true : selectedInfo.bType_05_T_BCWF = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_PLYW) == TRUE) ?	selectedInfo.bType_05_T_PLYW = true : selectedInfo.bType_05_T_PLYW = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_FISP) == TRUE) ?	selectedInfo.bType_05_T_FISP = true : selectedInfo.bType_05_T_FISP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_STSE) == TRUE) ?	selectedInfo.bType_05_T_STSE = true : selectedInfo.bType_05_T_STSE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_SLSE) == TRUE) ?	selectedInfo.bType_05_T_SLSE = true : selectedInfo.bType_05_T_SLSE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_RAIL) == TRUE) ?	selectedInfo.bType_05_T_RAIL = true : selectedInfo.bType_05_T_RAIL = false;

					// 06-F 가시설
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_STRU) == TRUE) ?	selectedInfo.bType_06_F_STRU = true : selectedInfo.bType_06_F_STRU = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_HFIL) == TRUE) ?	selectedInfo.bType_06_F_HFIL = true : selectedInfo.bType_06_F_HFIL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_SJAK) == TRUE) ?	selectedInfo.bType_06_F_SJAK = true : selectedInfo.bType_06_F_SJAK = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_PJAK) == TRUE) ?	selectedInfo.bType_06_F_PJAK = true : selectedInfo.bType_06_F_PJAK = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_BRKT) == TRUE) ?	selectedInfo.bType_06_F_BRKT = true : selectedInfo.bType_06_F_BRKT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_PBKT) == TRUE) ?	selectedInfo.bType_06_F_PBKT = true : selectedInfo.bType_06_F_PBKT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_CIP) == TRUE) ?	selectedInfo.bType_06_F_CIP = true : selectedInfo.bType_06_F_CIP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_LAND) == TRUE) ?	selectedInfo.bType_06_F_LAND = true : selectedInfo.bType_06_F_LAND = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_ANGL) == TRUE) ?	selectedInfo.bType_06_F_ANGL = true : selectedInfo.bType_06_F_ANGL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_ERAC) == TRUE) ?	selectedInfo.bType_06_F_ERAC = true : selectedInfo.bType_06_F_ERAC = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_LUMB) == TRUE) ?	selectedInfo.bType_06_F_LUMB = true : selectedInfo.bType_06_F_LUMB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_BPAN) == TRUE) ?	selectedInfo.bType_06_F_BPAN = true : selectedInfo.bType_06_F_BPAN = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_WALE) == TRUE) ?	selectedInfo.bType_06_F_WALE = true : selectedInfo.bType_06_F_WALE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_PILE) == TRUE) ?	selectedInfo.bType_06_F_PILE = true : selectedInfo.bType_06_F_PILE = false;

					break;

				case DG_CANCEL:
					break;
			}
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}