#ifndef	__UTILITY_FUNCTIONS__
#define __UTILITY_FUNCTIONS__

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdarg.h>
#include "MaxBIM.hpp"

// ���� ��ȯ
double	DegreeToRad (double degree);																		// degree ������ radian ������ ��ȯ
double	RadToDegree (double rad);																			// radian ������ degree ������ ��ȯ

// �Ÿ� ����
double	GetDistance (const double begX, const double begY, const double endX, const double endY);			// 2�������� 2�� ���� �Ÿ��� �˷���
double	GetDistance (const API_Coord begPoint, API_Coord endPoint);											// 2�������� 2�� ���� �Ÿ��� �˷���
double	GetDistance (const double begX, const double begY, const double begZ, const double endX, const double endY, const double endZ);		// 3�������� 2�� ���� �Ÿ��� �˷���
double	GetDistance (const API_Coord3D begPoint, API_Coord3D endPoint);										// 3�������� 2�� ���� �Ÿ��� �˷���
double	distOfPointBetweenLine (API_Coord p, API_Coord a, API_Coord b);										// ���� AB�� �� P���� �Ÿ��� ���ϴ� �Լ�
double	distOfPointBetweenLine (API_Coord3D p, API_Coord a, API_Coord b);									// ���� AB�� �� P���� �Ÿ��� ���ϴ� �Լ�

// ���ϱ�
long	compareDoubles (const double a, const double b);													// � ���� �� ū�� ����
long	compareRanges (double aMin, double aMax, double bMin, double bMax);									// a�� b�� �� �� ������ ���踦 �˷���
int		my_strcmp (const char *str1, const char *str2);														// ���ڿ� ��

// ��ȯ�ϱ�
void	exchangeDoubles (double* a, double* b);																// a�� b ���� ��ȯ��

// ���� (�Ϲ�)
long	findDirection (const double begX, const double begY, const double endX, const double endY);			// ���������� �������� ���ϴ� ������ ������ Ȯ��
API_Coord	IntersectionPoint1 (const API_Coord* p1, const API_Coord* p2, const API_Coord* p3, const API_Coord* p4);	// (p1, p2)�� ���� ������ (p3, p4)�� ���� ������ �������� ���ϴ� �Լ�
API_Coord	IntersectionPoint2 (double m1, double b1, double m2, double b2);											// y = m1*x + b1, y = m2*x + b2 �� ������ �������� ���ϴ� �Լ�
API_Coord	IntersectionPoint3 (double a1, double b1, double c1, double a2, double b2, double c2);						// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 �� ������ �������� ���ϴ� �Լ�
bool		isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint);											// aPoint�� bPoint�� ���� ������ Ȯ����
short		moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2);							// curPoint�� ����� ���� p1, p2 �� � ���Դϱ�?

// ���� (����)
bool	inRange (double srcPoint, double targetMin, double targetMax);										// srcPoint ���� target ���� �ȿ� ��� �ִ°�?
double	overlapRange (double srcMin, double srcMax, double targetMin, double targetMax);					// src ������ target ������ ��ġ�� ���̸� ������

// ���� (������ ����)
bool		isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd);	// aPoint�� pointList�� ������ �Ǿ����� Ȯ����
bool		isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint);				// nextPoint�� curPoint�� ���� ���Դϱ�?
API_Coord3D	getUnrotatedPoint (API_Coord3D rotatedPoint, API_Coord3D axisPoint, double ang);				// ȸ���� ������� �ʾ��� ���� ��ġ (��ġ�Ǿ�� �� ���� ��ġ�� ����), ������ Degree
API_Coord	getUnrotatedPoint (API_Coord rotatedPoint, API_Coord axisPoint, double ang);					// ȸ���� ������� �ʾ��� ���� ��ġ (��ġ�Ǿ�� �� ���� ��ġ�� ����), ������ Degree

// ���
double	round (double x, int digit);																		// �ݿø�

// ���ڿ�
std::string	format_string (const std::string fmt, ...);														// std::string ���� ���� formatted string�� �Է� ����
short	isStringDouble (char *str);																			// ���ڿ� s�� ���ڷ� �� ���ڿ����� �˷��� (���ڴ� 1, ���ڿ��� 0)
short	isStringDouble (const char *str);																	// ���ڿ� s�� ���ڷ� �� ���ڿ����� �˷��� (���ڴ� 1, ���ڿ��� 0)

// ��ü ��ġ
GSErrCode	placeCoordinateLabel (double xPos, double yPos, double zPos, bool bComment = false, std::string comment = "", short layerInd = 1, short floorInd = 0);		// ��ǥ ���� ��ġ��

// Ŭ����: ���� ��ü ��ġ
class EasyObjectPlacement
{
private:
	const char*		paramName [50];
	API_AddParID	paramType [50];
	const char*		paramVal [50];
	short			nParams;

	short			layerInd;
	short			floorInd;
	const GS::uchar_t*	gsmName;
public:
	double			posX;
	double			posY;
	double			posZ;
	double			radAng;

	EasyObjectPlacement ();		// ������
	EasyObjectPlacement (const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX, double posY, double posZ, double radAng);	// ������
	void		init (const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX, double posY, double posZ, double radAng);	// �ʱ�ȭ �Լ�
	void		setGsmName (const GS::uchar_t* gsmName);	// GSM ���ϸ� ����
	void		setLayerInd (short layerInd);	// ���̾� �ε��� ���� (1���� ����)
	void		setFloorInd (short floorInd);	// �� �ε��� ���� (����, 0, ��� ����)
	void		setParameters (short nParams, const char* paramNameList [], API_AddParID* paramTypeList, const char* paramValList []);	// �Ķ���� ����
	API_Guid	placeObject (double posX, double posY, double posZ, double radAng);		// ��ü ��ġ
	API_Guid	placeObject (const GS::uchar_t* gsmName, short nParams, const char* paramNameList [], API_AddParID* paramTypeList, const char* paramValList [], short layerInd, short floorInd, double posX, double posY, double posZ, double radAng);	// ��ü ��ġ
	API_Guid	placeObject (short nParams, ...);	// ��ü ��ġ (�Ķ������ ���� �� �Ķ���� �̸�/Ÿ��/���� �Է�)
	API_Guid	placeObject (const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX, double posY, double posZ, double radAng, short nParams, ...);				// �ʱ�ȭ�� �ϴ� ���ÿ� ��ü ��ġ (���� �Ķ����: �Ķ������ ���� �� �Ķ���� �̸�/Ÿ��/���� �Է�)
	API_Guid	placeObjectMirrored (double posX, double posY, double posZ, double radAng);		// ��ü ��ġ
	API_Guid	placeObjectMirrored (const GS::uchar_t* gsmName, short nParams, const char* paramNameList [], API_AddParID* paramTypeList, const char* paramValList [], short layerInd, short floorInd, double posX, double posY, double posZ, double radAng);	// ��ü ��ġ
	API_Guid	placeObjectMirrored (short nParams, ...);	// ��ü ��ġ (�Ķ������ ���� �� �Ķ���� �̸�/Ÿ��/���� �Է�)
	API_Guid	placeObjectMirrored (const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX, double posY, double posZ, double radAng, short nParams, ...);		// �ʱ�ȭ�� �ϴ� ���ÿ� ��ü ��ġ (���� �Ķ����: �Ķ������ ���� �� �Ķ���� �̸�/Ÿ��/���� �Է�)

	// ���� �ȳ�
	/*
	EasyObjectPlacement objP;
	objP.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, cell.leftBottomX, cell.leftBottomY, cell.leftBottomZ, cell.ang);

	for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
		height = 0.0;
		for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
			height += placementInfo.height [yy];
			elemList.Push (objP.placeObject (5,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", round (placementInfo.width [xx]*1000, 0)).c_str (),
				"eu_hei", APIParT_CString, format_string ("%.0f", round (placementInfo.height [yy]*1000, 0)).c_str (),
				"u_ins", APIParT_CString, "�������",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			moveIn3D ('z', objP.radAng, placementInfo.height [yy], &objP.posX, &objP.posY, &objP.posZ);
		}
		moveIn3D ('x', objP.radAng, placementInfo.width [xx], &objP.posX, &objP.posY, &objP.posZ);
		moveIn3D ('z', objP.radAng, -height, &objP.posX, &objP.posY, &objP.posZ);
	}
	*/
};

// ���̺귯�� ���� ���� (Getter/Setter)
bool		setParameterByName (API_ElementMemo* memo, char* pName, double value);			// pName �Ķ������ ���� value�� ������ (�Ǽ���) - �����ϸ� true, �����ϸ� false
bool		setParameterByName (API_ElementMemo* memo, char* pName, char* value);			// pName �Ķ������ ���� value�� ������ (���ڿ�) - �����ϸ� true, �����ϸ� false
double		getParameterValueByName (API_ElementMemo* memo, char* pName);					// pName �Ķ������ ���� ������ - �Ǽ���
const char*	getParameterStringByName (API_ElementMemo* memo, char* pName);					// pName �Ķ������ ���� ������ - ���ڿ�
API_AddParID	getParameterTypeByName (API_ElementMemo* memo, char* pName);				// pName �Ķ������ Ÿ���� ������ - API_AddParID

// ���� (�̵�)
void		moveIn3D (char direction, double ang, double offset, API_Coord3D* curPos);							// X, Y, Z�� ������ �����ϰ�, �ش� �������� �Ÿ��� �̵��� ��ǥ�� ������ (���� ���, ����: radian)
void		moveIn3D (char direction, double ang, double offset, double* curX, double* curY, double* curZ);		// X, Y, Z�� ������ �����ϰ�, �ش� �������� �Ÿ��� �̵��� ��ǥ�� ������ (���� ���, ����: radian)
void		moveIn2D (char direction, double ang, double offset, API_Coord* curPos);							// X, Y�� ������ �����ϰ�, �ش� �������� �Ÿ��� �̵��� ��ǥ�� ������ (���� ���, ����: radian)
void		moveIn2D (char direction, double ang, double offset, double* curX, double* curY);					// X, Y�� ������ �����ϰ�, �ش� �������� �Ÿ��� �̵��� ��ǥ�� ������ (���� ���, ����: radian)

// ���̾�
short		findLayerIndex (const char* layerName);																	// ���̾� �̸����� ���̾� �ε��� ã��
short		makeTemporaryLayer (API_Guid structurualObject, const char* suffix, char* returnedLayerName = NULL);	// ��ü�� ���̾� �̸��� 01-S�� �����ϴ� ��� ���λ縦 05-T�� �ٲٰ�, ������ + ���̻� ���ڿ��� ���� ���̾� �̸��� ������ �� ���̾� �ε����� �̸��� ������

// ���� ����
GSErrCode	getGuidsOfSelection (GS::Array<API_Guid>* guidList, API_ElemTypeID elemType, long *nElem);		// ������ ��ҵ� �߿��� ��� ID�� elemType�� ��ü���� GUID�� ������, ������ ������ ������
double		getWorkLevel (short floorInd);																	// �ش� ���� �۾� �� ���� ������

#endif
