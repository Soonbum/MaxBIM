#ifndef	__OTHERS__
#define __OTHERS__

#include "MaxBIM.hpp"

namespace othersDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_others {
	};
}

// 레이어 코드 체계
struct LayerNameSystem
{
	// 정규 코드 예시: 01-S-(0101)-9B1-(01)-(01)-WALL  단, 괄호 안은 선택사항

	// 공사 구분 (필수)
	bool	bType_01_S;		// 구조 (01-S)
	bool	bType_02_A;		// 건축마감 (02-A)
	bool	bType_03_M;		// 기계설비 (03-M)
	bool	bType_04_E;		// 전기설비 (04-E)
	bool	bType_05_T;		// 가설재 (05-T)
	bool	bType_06_F;		// 가시설 (06-F)
	bool	bType_07_Q;		// 물량합판 (07-Q)
	bool	bType_08_L;		// 조경 (08-L)
	bool	bType_09_C;		// 토목 (09-C)
	bool	bType_10_K;		// 건설장비 (10-K)
	bool	bTypeAll;		// 모두

	// 동 구분 (선택)
	bool	bDong_1xx [100];	// 100 ~ 199동 (0101~0199)
	bool	bDong_2xx [100];	// 200 ~ 299동 (0200~0299)
	bool	bDong_3xx [100];	// 300 ~ 399동 (0300~0399)
	bool	bDong_4xx [100];	// 400 ~ 499동 (0400~0499)
	bool	bDong_5xx [100];	// 500 ~ 599동 (0500~0599)
	bool	bDong_6xx [100];	// 600 ~ 699동 (0600~0699)
	bool	bDong_7xx [100];	// 700 ~ 799동 (0700~0799)
	bool	bDong_8xx [100];	// 800 ~ 899동 (0800~0899)
	bool	bDong_9xx [100];	// 900 ~ 999동 (0900~0999)
	bool	bDong_10xx [100];	// 1000 ~ 1099동 (1000~1099)
	bool	bDong_11xx [100];	// 1100 ~ 1199동 (1100~1199)
	bool	bDong_12xx [100];	// 1200 ~ 1299동 (1200~1299)
	bool	bDong_13xx [100];	// 1300 ~ 1399동 (1300~1399)
	bool	bDong_14xx [100];	// 1400 ~ 1499동 (1400~1499)
	bool	bDong_15xx [100];	// 1500 ~ 1599동 (1500~1599)
	bool	bDong_SHOP;			// 근린공원 (SHOP)
	bool	bDong_SECU;			// 경비실 (SECU)
	bool	bDongAll;			// 모두 (생략한 경우도 포함)

	// 층 구분 (필수)
	bool	bFloor_Bxx [10];	// 지하층 (인덱스 0은 사용하지 않음) 9B1, 8B2, 7B3, 6B4, 5B5, 4B6, 3B7, 2B8, 1B9 (1B9~9B1에서 1번째 숫자는 정렬 차원에서 넣은 것으로 의미는 없음)
	bool	bFloor_B_All;		// 지하층 모두
	bool	bFloor_Fxx [100];	// 지상층 (인덱스 0은 사용하지 않음) F01 ~ F99
	bool	bFloor_F_All;		// 지상층 모두
	bool	bFloor_PHxx [10];	// 옥탑층 (인덱스 0은 사용하지 않음) PH1 ~ PH9
	bool	bFloor_PH_All;		// 옥탑층 모두

	// CJ 구간 (선택)
	bool	bCJ [100];			// 01~99 (인덱스 0은 사용하지 않음)
	bool	bCJ_All;			// CJ 구간 모두

	// CJ 속 시공순서 (선택)
	bool	bOrderInCJ [100];	// 01~99 (인덱스 0은 사용하지 않음)
	bool	bOrderInCJ_All;		// CJ 속 시공순서 모두

	// 부재 및 객체 구분 (필수)
	// 01-S 구조
	bool	bType_01_S_STAR;	// 계단
	bool	bType_01_S_COLU;	// 기둥
	bool	bType_01_S_FOOT;	// 기초
	bool	bType_01_S_WALL;	// 벽체
	bool	bType_01_S_BEAM;	// 보
	bool	bType_01_S_SALB;	// 슬래브
	bool	bType_01_S_CLST;	// 철골기둥
	bool	bType_01_S_BMST;	// 철골보
	bool	bType_01_S_RAMP;	// 램프
	bool	bType_01_S_CWAL;	// 합벽
	bool	bType_01_S_WTWL;	// 방수턱
	bool	bType_01_S_CSTN;	// 연석
	bool	bType_01_S_MPAD;	// 장비패드
	bool	bType_01_S_GADN;	// 화단
	bool	bType_01_S_PARA;	// 파라펫
	bool	bType_01_S_CLPC;	// PC기둥
	bool	bType_01_S_BMPC;	// 보PC
	bool	bType_01_S_BMWL;	// 보벽체
	bool	bType_01_S_STST;	// 철골계단

	// 02-A 건축마감
	bool	bType_02_A_FURN;	// 가구
	bool	bType_02_A_INSU;	// 단열재
	bool	bType_02_A_PAIN;	// 도장
	bool	bType_02_A_MOLD;	// 몰딩
	bool	bType_02_A_MORT;	// 몰탈
	bool	bType_02_A_WATE;	// 방수
	bool	bType_02_A_BRIC;	// 벽돌
	bool	bType_02_A_PAPE;	// 벽지
	bool	bType_02_A_BLOC;	// 블록
	bool	bType_02_A_GYPS;	// 석고보드
	bool	bType_02_A_STON;	// 석재
	bool	bType_02_A_INTE;	// 수장
	bool	bType_02_A_GLAS;	// 유리
	bool	bType_02_A_HARD;	// 철물
	bool	bType_02_A_TILE;	// 타일
	bool	bType_02_A_PANE;	// 판넬
	bool	bType_02_A_PLYW;	// 합판
	bool	bType_02_A_PCON;	// 무근콘크리트
	
	// 05-T 가설재
	bool	bType_05_T_TIMB;	// 각재
	bool	bType_05_T_BIMJ;	// 각파이프행거
	bool	bType_05_T_BDCM;	// 기둥밴드
	bool	bType_05_T_DMGA;	// 다마가
	bool	bType_05_T_RIBL;	// 리브라스
	bool	bType_05_T_BMSP;	// 보받침필러
	bool	bType_05_T_BSTA;	// 비계계단
	bool	bType_05_T_SPIP;	// 사각파이프
	bool	bType_05_T_SUPT;	// 서포트
	bool	bType_05_T_SYSU;	// 시스템서포트
	bool	bType_05_T_OUTA;	// 아웃코너앵글
	bool	bType_05_T_OUTP;	// 아웃코너판넬
	bool	bType_05_T_AFOM;	// 알폼
	bool	bType_05_T_CPIP;	// 원형파이프
	bool	bType_05_T_UGON;	// 유공발판
	bool	bType_05_T_UFOM;	// 유로폼
	bool	bType_05_T_INCO;	// 인코너판넬
	bool	bType_05_T_JOIB;	// 조인트바
	bool	bType_05_T_EFOM;	// 종이거푸집
	bool	bType_05_T_JSUPT;	// 잭서포트
	bool	bType_05_T_WTST;	// 지수판
	bool	bType_05_T_CLAM;	// 클램프
	bool	bType_05_T_LUMB;	// 토류판
	bool	bType_05_T_TRUS;	// 트러스
	bool	bType_05_T_TBBM;	// 텀버빔
	bool	bType_05_T_BCWF;	// 합벽지지대
	bool	bType_05_T_PLYW;	// 합판
	bool	bType_05_T_FISP;	// 휠러스페이서
	bool	bType_05_T_STSE;	// 계단제작폼
	bool	bType_05_T_SLSE;	// 슬래브제작폼
	bool	bType_05_T_RAIL;	// 목심레일
	
	// 06-F 가시설
	bool	bType_06_F_STRU;	// 수평H형강
	bool	bType_06_F_HFIL;	// 수직H형강
	bool	bType_06_F_SJAK;	// 스크류잭
	bool	bType_06_F_PJAK;	// 프리로딩잭
	bool	bType_06_F_BRKT;	// 브라켓
	bool	bType_06_F_PBKT;	// 피스 브라켓
	bool	bType_06_F_CIP;		// 흙막이 벽체
	bool	bType_06_F_LAND;	// 대지
	bool	bType_06_F_ANGL;	// 행글
	bool	bType_06_F_ERAC;	// 지반앵커
	bool	bType_06_F_LUMB;	// 토류판
	bool	bType_06_F_BPAN;	// 복공판
	bool	bType_06_F_WALE;	// 띠장
	bool	bType_06_F_PILE;	// 파일
};

GSErrCode	showLayersEasily (void);	// 레이어 쉽게 선택하기
//short DGCALLBACK aboutHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그 박스] MaxBIM 애드온 정보

#endif