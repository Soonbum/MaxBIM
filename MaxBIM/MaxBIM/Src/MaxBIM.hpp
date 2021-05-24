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
	#pragma warning (disable: 4239)
	#pragma warning (disable: 4127)
	#pragma warning (disable: 4701)
#endif

#include "ACAPinc.h"
#include "Location.hpp"
#include "DGModule.hpp"
#include "UC.h"
#include "DG.h"
#include "APICommon.h"
#include <string.h>
#include <vector>

#define TRUE	1
#define FALSE	0


// 벽 관련 정보
struct InfoWall
{
	double	wallThk;			// 벽 두께
	short	floorInd;			// 층 인덱스
	double	bottomOffset;		// 벽 하단 오프셋

	double	begX;				// 시작점 X
	double	begY;				// 시작점 Y
	double	endX;				// 끝점 X
	double	endY;				// 끝점 Y
};

// 슬래브 관련 정보
struct InfoSlab
{
	short	floorInd;			// 층 인덱스
	double	offsetFromTop;		// 슬래브 윗면과 레퍼런스 레벨과의 수직 거리
	double	thickness;			// 슬래브 두께
	double	level;				// 레퍼런스 레벨의 고도
};

// 보 관련 정보
struct InfoBeam
{
	API_Guid	guid;	// 보의 GUID

	short	floorInd;	// 층 인덱스
	double	height;		// 보 높이
	double	width;		// 보 너비
	double	offset;		// 보 중심으로부터 보의 레퍼런스 라인의 오프셋입니다.
	double	level;		// 바닥 레벨에 대한 보의 위쪽면 높이입니다.

	API_Coord	begC;	// 보 시작 좌표
	API_Coord	endC;	// 보 끝 좌표
};

// 기둥 관련 정보
struct InfoColumn
{
	API_Guid	guid;		// 기둥의 GUID
	short	floorInd;		// 층 인덱스

	bool	bRectangle;		// 직사각형 형태가 맞는가?
	short	coreAnchor;		// 코어의 앵커 포인트
	double	coreWidth;		// 기둥의 X 길이
	double	coreDepth;		// 기둥의 Y 길이
	double	venThick;		// 기둥 베니어 두께
	double	height;			// 기둥의 높이
	double	bottomOffset;	// 바닥 레벨에 대한 기둥 베이스 레벨
	double	topOffset;		// 만약 기둥이 윗층과 연결되어 있는 경우 윗층으로부터의 오프셋
	double	angle;			// 기둥 축을 중심으로 한 회전 각도 (단위: Radian)
	API_Coord	origoPos;	// 기둥의 위치
};

// 유로폼 정보
struct Euroform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	bool	eu_stan_onoff;	// 규격폼 On/Off
	double	eu_wid;			// 너비 (규격) : *600, 500, 450, 400, 300, 200
	double	eu_hei;			// 높이 (규격) : *1200, 900, 600
	double	eu_wid2;		// 너비 (비규격) : 50 ~ 900
	double	eu_hei2;		// 높이 (비규격) : 50 ~ 1500
	bool	u_ins_wall;		// 설치방향 : 벽세우기(true), 벽눕히기(false)
	double	ang_x;			// 회전X : 벽(90), 천장(0), 바닥(180)
	/*
	double	ang_y;			// 회전Y
	*/
	double	width;			// 너비 (규격폼/비규격폼 구분 없음)
	double	height;			// 높이 (규격폼/비규격폼 구분 없음)
};

// 휠러스페이서 정보
struct FillerSpacer
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	f_thk;			// 두께 : 10 ~ 50 (*20)
	double	f_leng;			// 길이 : 150 ~ 2400
	double	f_ang;			// 각도 : 90
	double	f_rota;			// 회전 : 0
};

// 인코너판넬 정보
struct IncornerPanel
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	wid_s;			// 가로(빨강) : 80 ~ 500 (*100)
	double	leng_s;			// 세로(파랑) : 80 ~ 500 (*100)
	double	hei_s;			// 높이 : 50 ~ 1500
	/*
	std::string		dir_s;			// 설치방향 : *세우기, 눕히기, 뒤집기
	*/
};

// 아웃코너판넬 정보
struct OutcornerPanel
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	wid_s;			// 가로(빨강) : 80 ~ 500 (*100)
	double	leng_s;			// 세로(파랑) : 80 ~ 500 (*100)
	double	hei_s;			// 높이 : 50 ~ 1500
	/*
	std::string		dir_s;			// 설치방향 : *세우기, 눕히기, 뒤집기
	*/
};

// 합판 정보
struct Plywood
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	/*
	std::string		p_stan;			// 규격 : *3x6 [910x1820], 4x8 [1220x2440], 비규격, 비정형
	std::string		w_dir;			// 설치방향 : *벽세우기, 벽눕히기, 바닥깔기, 바닥덮기
	std::string		p_thk;			// 두께 : 2.7T, 4.8T, 8.5T, *11.5T, 14.5T
	*/
	double	p_wid;			// 가로
	double	p_leng;			// 세로
	bool	w_dir_wall;		// 설치방향 : 벽세우기(true), 벽눕히기(false)
	short	w_dir;			// (확장) 설치방향 : 벽세우기(1), 벽눕히기(2), 바닥깔기(3), 바닥덮기(4)
	/*
	double			p_ang;			// 각도 : 0
	bool			sogak;			// 제작틀 *On/Off
	std::string		prof;			// 목재종류 : *소각, 중각, 대각
	*/
};

// 목재 정보
struct Wood
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	/*
	std::string		w_ins;			// 설치방향 : *벽세우기, 바닥눕히기, 바닥덮기
	*/
	double	w_w;			// 두께
	double	w_h;			// 너비
	double	w_leng;			// 길이
	double	w_ang;			// 각도
};

// 아웃코너앵글 정보
struct OutcornerAngle
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	a_leng;			// 길이
	double	a_ang;			// 각도
};

// 매직바 정보
struct MagicBar
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	ZZYZX;				// 높이

	double	angX;				// 회전 X
	double	angY;				// 회전 Y

	bool	bPlywood;			// 합판 On/Off
	double	plywoodWidth;		// 합판 너비
	double	plywoodOverhangH;	// 합판 오버행
	double	plywoodUnderhangH;	// 합판 언더행
};

// 매직인코너 정보
struct MagicIncorner
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	ZZYZX;				// 높이

	short	type;				// 타입 (50, 100)

	double	angX;				// 회전 X
	double	angY;				// 회전 Y

	bool	bPlywood;			// 합판 On/Off
	double	plywoodWidth;		// 합판 너비
	double	plywoodOverhangH;	// 합판 오버행
	double	plywoodUnderhangH;	// 합판 언더행
};

// 슬래브 테이블폼 정보
struct SlabTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	bool	direction;		// 설치방향 : 가로방향(true), 세로방향(false)
	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이

	char	type [20];		// 타입
};

// 벽 테이블폼 정보
struct WallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	width;			// 너비 (1800~2300, 50 간격)
	double	height;			// 높이 (1500~6000, 300 간격)
};

// 비계 파이프 정보
struct SquarePipe
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	length;			// 파이프 길이
	double	pipeAng;		// 각도 (수평: 0, 수직: 90)
};

// 핀볼트 세트 정보
struct PinBoltSet
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	bool	bPinBoltRot90;	// 핀볼트 90도 회전
	double	boltLen;		// 볼트 길이
	double	angX;			// X축 회전
	double	angY;			// Y축 회전
};

// 벽체 타이 정보
struct WallTie
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	boltLen;		// 볼트 길이
	double	pipeBeg;		// 파이프 시작점
	double	pipeEnd;		// 파이프 끝점
	double	clampBeg;		// 조임쇠 시작점
	double	clampEnd;		// 조임쇠 끝점
};

// 직교 클램프 정보
struct CrossClamp
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	angX;			// 본체 회전 (X)
	double	angY;			// 본체 회전 (Y)
};

// 헤드피스 정보
struct HeadpieceOfPushPullProps
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)
};

// 결합철물 정보
struct MetalFittings
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	angX;			// 본체 회전 (X)
	double	angY;			// 본체 회전 (Y)
};

// KS프로파일 정보
struct KSProfile
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	angX;			// 본체 회전 (X)
	double	angY;			// 본체 회전 (Y)

	char	type [10];		// 분류 (기둥, 보)
	char	shape [30];		// 형태 (C형강, H형강...)
	short	iAnchor;		// 앵커 포인트 (1~9)
	double	len;			// 길이
	char	nom [50];		// 규격
	long	mat;			// 재질
};

//  결합철물 (사각와셔활용) 정보
struct MetalFittingsWithRectWasher
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	angX;			// 본체 회전 (X)
	double	angY;			// 본체 회전 (Y)

	double	bolt_len;		// 볼트 길이
	double	bolt_dia;		// 볼트 직경
	bool	bWasher1;		// 와셔1 On/Off
	double	washer_pos1;	// 와셔1 위치
	bool	bWasher2;		// 와셔2 On/Off
	double	washer_pos2;	// 와셔2 위치
	double	washer_size;	// 와셔 크기
	char	nutType [15];	// 너트 타입
};

#endif //__MAXBIM_HPP__
