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


// ��ü Ÿ��
enum	libPartObjType {
	NONE,			// ����
	INCORNER,		// ���ڳ��ǳ�v1.0
	EUROFORM,		// ������v2.0
	FILLERSPACER,	// �ٷ������̼�v1.0
	PLYWOOD,		// ����v1.0
	WOOD			// ����v1.0
};

// ������ ����
struct Euroform
{
	bool			eu_stan_onoff;	// �԰��� On/Off
	double			eu_wid;			// �ʺ� (�԰�) : *600, 500, 450, 400, 300, 200
	double			eu_hei;			// ���� (�԰�) : *1200, 900, 600
	double			eu_wid2;		// �ʺ� (��԰�) : 50 ~ 900
	double			eu_hei2;		// ���� (��԰�) : 50 ~ 1500
	bool			u_ins_wall;		// ��ġ���� : �������(true), ��������(false)
	/*
	double			ang_x;			// ȸ��X : ��(90), õ��(0), �ٴ�(180)
	double			ang_y;			// ȸ��Y
	*/
};

// �ٷ������̼� ����
struct FillerSpacer
{
	double			f_thk;			// �β� : 10 ~ 50 (*20)
	double			f_leng;			// ���� : 150 ~ 2400
	/*
	double			f_ang;			// ���� : 90
	double			f_rota;			// ȸ�� : 0
	*/
};

// ���ڳ��ǳ� ����
struct IncornerPanel
{
	double			wid_s;			// ����(����) : 80 ~ 500 (*100)
	double			leng_s;			// ����(�Ķ�) : 80 ~ 500 (*100)
	double			hei_s;			// ���� : 50 ~ 1500
	/*
	std::string		dir_s;			// ��ġ���� : *�����, ������, ������
	*/
};

// ���� ����
struct Plywood
{
	/*
	std::string		p_stan;			// �԰� : *3x6 [910x1820], 4x8 [1220x2440], ��԰�, ������
	std::string		w_dir;			// ��ġ���� : *�������, ��������, �ٴڱ��, �ٴڵ���
	std::string		p_thk;			// �β� : 2.7T, 4.8T, 8.5T, *11.5T, 14.5T
	*/
	double			p_wid;			// ����
	double			p_leng;			// ����
	bool			w_dir_wall;		// ��ġ���� : �������(true), ��������(false)
	/*
	double			p_ang;			// ���� : 0
	bool			sogak;			// ����Ʋ *On/Off
	std::string		prof;			// �������� : *�Ұ�, �߰�, �밢
	*/
};

// ���� ����
struct Wood
{
	/*
	std::string		w_ins;			// ��ġ���� : *�������, �ٴڴ�����, �ٴڵ���
	*/
	double			w_w;			// �β�
	double			w_h;			// �ʺ�
	double			w_leng;			// ����
	double			w_ang;			// ����
};

#endif //__MAXBIM_HPP__
