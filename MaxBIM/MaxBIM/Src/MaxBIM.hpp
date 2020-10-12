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
#endif

#include "ACAPinc.h"
#include "Location.hpp"
#include "DGModule.hpp"
#include "UC.h"
#include "DG.h"
#include "APICommon.h"

#define TRUE	1
#define FALSE	0


// 객체 타입
enum	libPartObjType {
	NONE,			// 없음
	INCORNER,		// 인코너판넬v1.0
	EUROFORM,		// 유로폼v2.0
	FILLERSPACER,	// 휠러스페이서v1.0
	PLYWOOD,		// 합판v1.0
	WOOD			// 목재v1.0
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
	std::string		dir_s;			// 설치방향 : *세우기, 눕히기, 뒤집기
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

// 목재 정보
struct Wood
{
	/*
	std::string		w_ins;			// 설치방향 : *벽세우기, 바닥눕히기, 바닥덮기
	*/
	double			w_w;			// 두께
	double			w_h;			// 너비
	double			w_leng;			// 길이
	double			w_ang;			// 각도
};

#endif //__MAXBIM_HPP__
