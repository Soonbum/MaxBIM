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


// ���� ����
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

// �Լ� ����
double	degreeToRad (double degree);																	// degree ������ radian ������ ��ȯ
double	RadToDegree (double rad);																		// radian ������ degree ������ ��ȯ
double	GetDistance (const double begX, const double begY, const double endX, const double endY);		// 2�������� 2�� ���� �Ÿ��� �˷���
long	compareDoubles (const double a, const double b);												// � ���� �� ū�� ����
long	compareRanges (const double aMin, const double aMax, const double bMin, const double bMax);		// a�� b�� �� �� ������ ���踦 �˷���
void	exchangeDoubles (double* a, double* b);															// a�� b ���� ��ȯ��
long	findDirection (const double begX, const double begY, const double endX, const double endY);		// ���������� �������� ���ϴ� ������ ������ Ȯ��
void	initCells (PlacingZone* placingZone);															// Cell �迭�� �ʱ�ȭ��
void	firstPlacingSettings (PlacingZone* placingZone);												// 1�� ��ġ: ���ڳ�, ������
void	copyPlacingZoneSymmetric (PlacingZone* src_zone, PlacingZone* dst_zone, InfoWall* infoWall);	// ���� ���� ���� ������ ��Ī�ϴ� �ݴ��ʿ��� ������
void	alignPlacingZone (PlacingZone* target_zone);													// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
std::string	format_string(const std::string fmt, ...);													// std::string ���� ���� formatted string�� �Է� ����

// ������/���ڳ� ��ġ ���� �Լ�
GSErrCode	placeEuroformOnWall (void);		// 1�� �޴�: ������/���ڳ� ���� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK placerHandlerPrimary (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK placerHandlerSecondary (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK placerHandlerThird (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�

API_Guid	placeLibPart (Cell objInfo);											// �ش� �� ������ ������� ���̺귯�� ��ġ
double		getCellPositionLeftBottomX (PlacingZone *src_zone, short idx);			// [0]�� - �ش� ���� ���ϴ� ��ǥX ��ġ�� ����
void		setCellPositionLeftBottomZ (PlacingZone *src_zone, double new_hei);		// [0]�� - ��ü ���� ���ϴ� ��ǥZ ��ġ�� ����


// ���̾�α� �׸� �ε���
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

	// ���Ŀ��� �׸��� ��ư�� ��ġ��
	GRIDBUTTON_IDX_START					= 7
};

enum	idxItem_3 {
	LABEL_OBJ_TYPE						= 3,
	POPUP_OBJ_TYPE						= 4,

	// ���ڳ�, �ٷ������̼�, ������ ���
	LABEL_WIDTH							= 5,
	EDITCONTROL_WIDTH					= 6,
	LABEL_HEIGHT						= 7,
	EDITCONTROL_HEIGHT					= 8,
	LABEL_ORIENTATION					= 9,
	RADIO_ORIENTATION_1_PLYWOOD			= 10,
	RADIO_ORIENTATION_2_PLYWOOD			= 11,

	// �������� ���
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

// ��ü ��ȣ
enum	libPartObjType {
	NONE,			// ����
	INCORNER,		// ���ڳ��ǳ�v1.0
	EUROFORM,		// ������v2.0
	FILLERSPACER,	// �ٷ������̼�v1.0
	PLYWOOD			// ����v1.0
};


// �� ���� ����
struct InfoWall
{
	double	wallThk;			// �� �β�
	short	floorInd;			// �� �ε���

	double	begX;				// ������ X
	double	begY;				// ������ Y
	double	endX;				// ���� X
	double	endY;				// ���� Y
};

// ���� ���� ����
struct InfoMorph
{
	API_Guid	guid;		// ������ GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	rightTopX;		// ���� ��ǥ X
	double	rightTopY;		// ���� ��ǥ Y
	double	rightTopZ;		// ���� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Degree, ȸ����: Z��)
};

// ���� �� ����
struct InterfereBeam
{
	double	leftBottomX;
	double	leftBottomZ;

	double	horLen;
	double	verLen;
};

// �ĸ��� ����
struct BacksideColumn
{
	double	leftBottomX;
	double	leftBottomZ;

	double	horLen;
	double	verLen;
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
	std::string		u_ins;			// ��ġ���� : *�������, ��������
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
	double			dir_s;			// ��ġ���� : *�����, ������, ������
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

// �׸��� �� �� ����
struct Cell
{
	short		objType;	// enum libPartObjType ����

	API_Guid	guid;		// ��ü�� GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	union {
		Euroform		form;
		FillerSpacer	fillersp;
		IncornerPanel	incorner;
		Plywood			plywood;
	} libPart;
};

// ���� ���� ���� (���� �߿��� ����)
struct PlacingZone
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	// ������ (0�� �̻�)
	short	nInterfereBeams;	// ������ ����
	InterfereBeam	beams [30];

	// �ĸ��� (0�� �̻�)
	short	nBacksideColumn;	// �ĸ��� ����
	BacksideColumn	columns [30];

	// ������ ���� (1. �⺻ ä���)
	double	remain_hor;				// ���� ���� ���� ����
	double	remain_hor_updated;		// ���� ���� ���� ���� (������Ʈ ��)
	double	remain_ver;				// ���� ���� ���� ����
	double	remain_ver_wo_beams;	// �������� ������ ���� �ʴ� ���� ����

	bool	bLIncorner;				// ���ڳ� ���� ��ġ
	double	lenLIncorner;			// ���ڳ� ���� �ʺ�
	bool	bRIncorner;				// ���ڳ� ������ ��ġ
	double	lenRIncorner;			// ���ڳ� ������ �ʺ�

	std::string		eu_wid;			// ������ �ʺ�
	std::string		eu_hei;			// ������ ����
	std::string		eu_ori;			// ������ ����
	double	eu_wid_numeric;			// ������ �ʺ� (�Ǽ���)
	double	eu_hei_numeric;			// ������ ���� (�Ǽ���)
	short	eu_count_hor;			// ���� ���� ������ ����
	short	eu_count_ver;			// ���� ���� ������ ����

	// ������ ���� (2. ��ġ�� ��ü ������ �׸���� ����)
	// ���ڳ�[0] | ����[Ȧ��] | ��[¦��] | ... | ���ڳ�[n-1]
	Cell	cells [50][100];		// �ַ� [0][n] �� ����, �ٸ� ����� guid �����
	short	nCells;
};
