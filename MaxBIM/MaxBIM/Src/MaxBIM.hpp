/**
 * @file Contains the includes and definitions necessary for the Add-On to
 *       function.
 */

#if !defined (__MAXBIM_HPP__)
#define __MAXBIM_HPP__

#ifdef _WIN32
	#pragma warning (push, 3)
	#include	<Win32Interface.hpp>
	#pragma warning (pop)

	#ifndef WINDOWS
		#define WINDOWS
	#endif
#endif

#ifdef macintosh
	#include <CoreServices/CoreServices.h>
#endif

#ifndef ACExtension
	#define	ACExtension
#endif

#ifdef WINDOWS
	#pragma warning (disable: 4068)
#endif

#include "ACAPinc.h"
#include "Location.hpp"
#include "DGModule.hpp"
#include "UC.h"
#include "DG.h"
#include "APICommon.h"

#ifdef WINDOWS
	#pragma warning (default: 4068)
#endif

#pragma warning (disable: 4239)
#pragma warning (disable: 4127)

#define TRUE	1
#define FALSE	0

#endif //__MAXBIM_HPP__


// 변수 선언
enum	idxItems_1;
enum	idxItems_2;
enum	libPartObjType;
struct	InfoWall;
struct	InfoMorph;
struct	InterfereBeam;
struct	BacksideColumn;
struct	Euroform;
struct	FillerSpacer;
struct	IncornerPanel;
struct	Plywood;
struct	Cell;
struct	PlacingZone;

// 함수 선언
double	degreeToRad (double degree);																	// degree 각도를 radian 각도로 변환
double	RadToDegree (double rad);																		// radian 각도를 degree 각도로 변환
double	GetDistance (const double begX, const double begY, const double endX, const double endY);		// 2차원에서 2점 간의 거리를 알려줌
long	compareDoubles (const double a, const double b);												// 어떤 수가 더 큰지 비교함
long	compareRanges (const double aMin, const double aMax, const double bMin, const double bMax);		// a와 b의 각 값 범위의 관계를 알려줌
void	exchangeDoubles (double* a, double* b);															// a와 b 값을 교환함
long	findDirection (const double begX, const double begY, const double endX, const double endY);		// 시작점에서 끝점으로 향하는 벡터의 방향을 확인
void	initCells (PlacingZone* placingZone);															// Cell 배열을 초기화함
void	firstPlacingSettings (PlacingZone* placingZone);												// 1차 배치: 인코너, 유로폼
void	copyPlacingZoneSymmetric (PlacingZone* src_zone, PlacingZone* dst_zone, InfoWall* infoWall);	// 원본 벽면 영역 정보를 대칭하는 반대쪽에도 복사함
void	alignPlacingZone (PlacingZone* target_zone);													// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
std::string	format_string(const std::string fmt, ...);													// std::string 변수 값에 formatted string을 입력 받음

// 유로폼/인코너 배치 관련 함수
GSErrCode	placeEuroformOnWall (void);		// 1번 메뉴: 유로폼/인코너 등을 배치하는 통합 루틴
short DGCALLBACK placerHandlerPrimary (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK placerHandlerSecondary (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK placerHandlerThird (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그

API_Guid	placeLibPart (Cell objInfo);											// 해당 셀 정보를 기반으로 라이브러리 배치
double		getCellPositionLeftBottomX (PlacingZone *src_zone, short idx);			// [0]행 - 해당 셀의 좌하단 좌표X 위치를 리턴
void		setCellPositionLeftBottomZ (PlacingZone *src_zone, double new_hei);		// [0]행 - 전체 셀의 최하단 좌표Z 위치를 설정


// 다이얼로그 항목 인덱스
enum	idxItems_1 {
	LABEL_INCORNER				= 3,
	CHECKBOX_SET_LEFT_INCORNER	= 4,
	EDITCONTROL_LEFT_INCORNER	= 5,
	CHECKBOX_SET_RIGHT_INCORNER	= 6,
	EDITCONTROL_RIGHT_INCORNER	= 7,
	SEPARATOR_1					= 8,

	LABEL_PLACING_EUROFORM		= 9,
	LABEL_EUROFORM_WIDTH		= 10,
	POPUP_EUROFORM_WIDTH		= 11,
	LABEL_EUROFORM_HEIGHT		= 12,
	POPUP_EUROFORM_HEIGHT		= 13,
	LABEL_EUROFORM_ORIENTATION	= 14,
	POPUP_EUROFORM_ORIENTATION	= 15,
	SEPARATOR_2					= 16,

	ICON_LAYER					= 17,
	USERCONTROL_LAYER			= 18
};

enum	idxItems_2 {
	LABEL_REMAIN_HORIZONTAL_LENGTH			= 3,
	EDITCONTROL_REMAIN_HORIZONTAL_LENGTH	= 4,
	GROUPBOX_GRID_EUROFORM_FILLERSPACER		= 5,
	PUSHBUTTON_CONFIRM_REMAIN_LENGTH		= 6,

	// 이후에는 그리드 버튼이 배치됨
	GRIDBUTTON_IDX_START					= 7
};

enum	idxItem_3 {
	LABEL_OBJ_TYPE						= 3,
	POPUP_OBJ_TYPE						= 4,

	// 인코너, 휠러스페이서, 합판의 경우
	LABEL_WIDTH							= 5,
	EDITCONTROL_WIDTH					= 6,
	LABEL_HEIGHT						= 7,
	EDITCONTROL_HEIGHT					= 8,
	LABEL_ORIENTATION					= 9,
	RADIO_ORIENTATION_1_PLYWOOD			= 10,
	RADIO_ORIENTATION_2_PLYWOOD			= 11,

	// 유로폼의 경우
	CHECKBOX_SET_STANDARD				= 12,
	LABEL_EUROFORM_WIDTH_OPTIONS		= 13,
	POPUP_EUROFORM_WIDTH_OPTIONS		= 14,
	EDITCONTROL_EUROFORM_WIDTH_OPTIONS	= 15,
	LABEL_EUROFORM_HEIGHT_OPTIONS		= 16,
	POPUP_EUROFORM_HEIGHT_OPTIONS		= 17,
	EDITCONTROL_EUROFORM_HEIGHT_OPTIONS	= 18,
	LABEL_EUROFORM_ORIENTATION_OPTIONS	= 19,
	RADIO_ORIENTATION_1_EUROFORM		= 20,
	RADIO_ORIENTATION_2_EUROFORM		= 21
};

// 객체 번호
enum	libPartObjType {
	NONE,			// 없음
	INCORNER,		// 인코너판넬v1.0
	EUROFORM,		// 유로폼v2.0
	FILLERSPACER,	// 휠러스페이서v1.0
	PLYWOOD			// 합판v1.0
};


// 벽 관련 정보
struct InfoWall
{
	double	wallThk;			// 벽 두께
	short	floorInd;			// 층 인덱스

	double	begX;				// 시작점 X
	double	begY;				// 시작점 Y
	double	endX;				// 끝점 X
	double	endY;				// 끝점 Y
};

// 모프 관련 정보
struct InfoMorph
{
	API_Guid	guid;		// 모프의 GUID

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	rightTopX;		// 우상단 좌표 X
	double	rightTopY;		// 우상단 좌표 Y
	double	rightTopZ;		// 우상단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Degree, 회전축: Z축)
};

// 간섭 보 정보
struct InterfereBeam
{
	double	leftBottomX;
	double	leftBottomZ;

	double	horLen;
	double	verLen;
};

// 후면기둥 정보
struct BacksideColumn
{
	double	leftBottomX;
	double	leftBottomZ;

	double	horLen;
	double	verLen;
};

// 유로폼 정보
struct Euroform
{
	bool			eu_stan_onoff;	// 규격폼 On/Off
	double			eu_wid;			// 너비 (규격) : *600, 500, 450, 400, 300, 200
	double			eu_hei;			// 높이 (규격) : *1200, 900, 600
	double			eu_wid2;		// 너비 (비규격) : 50 ~ 900
	double			eu_hei2;		// 높이 (비규격) : 50 ~ 1500
	bool			u_ins_wall;		// 설치방향 : 벽세우기(true), 벽눕히기(false)
	/*
	std::string		u_ins;			// 설치방향 : *벽세우기, 벽눕히기
	double			ang_x;			// 회전X : 벽(90), 천장(0), 바닥(180)
	double			ang_y;			// 회전Y
	*/
};

// 휠러스페이서 정보
struct FillerSpacer
{
	double			f_thk;			// 두께 : 10 ~ 50 (*20)
	double			f_leng;			// 길이 : 150 ~ 2400
	/*
	double			f_ang;			// 각도 : 90
	double			f_rota;			// 회전 : 0
	*/
};

// 인코너판넬 정보
struct IncornerPanel
{
	double			wid_s;			// 가로(빨강) : 80 ~ 500 (*100)
	double			leng_s;			// 세로(파랑) : 80 ~ 500 (*100)
	double			hei_s;			// 높이 : 50 ~ 1500
	/*
	double			dir_s;			// 설치방향 : *세우기, 눕히기, 뒤집기
	*/
};

// 합판 정보
struct Plywood
{
	/*
	std::string		p_stan;			// 규격 : *3x6 [910x1820], 4x8 [1220x2440], 비규격, 비정형
	std::string		w_dir;			// 설치방향 : *벽세우기, 벽눕히기, 바닥깔기, 바닥덮기
	std::string		p_thk;			// 두께 : 2.7T, 4.8T, 8.5T, *11.5T, 14.5T
	*/
	double			p_wid;			// 가로
	double			p_leng;			// 세로
	bool			w_dir_wall;		// 설치방향 : 벽세우기(true), 벽눕히기(false)
	/*
	double			p_ang;			// 각도 : 0
	bool			sogak;			// 제작틀 *On/Off
	std::string		prof;			// 목재종류 : *소각, 중각, 대각
	*/
};

// 그리드 각 셀 정보
struct Cell
{
	short		objType;	// enum libPartObjType 참조

	API_Guid	guid;		// 객체의 GUID

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	union {
		Euroform		form;
		FillerSpacer	fillersp;
		IncornerPanel	incorner;
		Plywood			plywood;
	} libPart;
};

// 벽면 영역 정보 (가장 중요한 정보)
struct PlacingZone
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	// 간섭보 (0개 이상)
	short	nInterfereBeams;	// 간섭보 개수
	InterfereBeam	beams [30];

	// 후면기둥 (0개 이상)
	short	nBacksideColumn;	// 후면기둥 개수
	BacksideColumn	columns [30];

	// 검토할 사항 (1. 기본 채우기)
	double	remain_hor;				// 가로 방향 남은 길이
	double	remain_hor_updated;		// 가로 방향 남은 길이 (업데이트 후)
	double	remain_ver;				// 세로 방향 남은 길이
	double	remain_ver_wo_beams;	// 간섭보의 영향을 받지 않는 세로 길이

	bool	bLIncorner;				// 인코너 왼쪽 배치
	double	lenLIncorner;			// 인코너 왼쪽 너비
	bool	bRIncorner;				// 인코너 오른쪽 배치
	double	lenRIncorner;			// 인코너 오른쪽 너비

	std::string		eu_wid;			// 유로폼 너비
	std::string		eu_hei;			// 유로폼 높이
	std::string		eu_ori;			// 유로폼 방향
	double	eu_wid_numeric;			// 유로폼 너비 (실수형)
	double	eu_hei_numeric;			// 유로폼 높이 (실수형)
	short	eu_count_hor;			// 가로 방향 유로폼 개수
	short	eu_count_ver;			// 세로 방향 유로폼 개수

	// 검토할 사항 (2. 배치된 객체 정보를 그리드로 관리)
	// 인코너[0] | 예비[홀수] | 폼[짝수] | ... | 인코너[n-1]
	Cell	cells [50][100];		// 주로 [0][n] 만 사용됨, 다른 행들은 guid 저장용
	short	nCells;
};
