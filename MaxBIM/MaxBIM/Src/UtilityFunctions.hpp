#ifndef	__UTILITY_FUNCTIONS__
#define __UTILITY_FUNCTIONS__

#define _USE_MATH_DEFINES
#include <math.h>
#include "MaxBIM.hpp"

double	DegreeToRad (double degree);																		// degree ������ radian ������ ��ȯ
double	RadToDegree (double rad);																			// radian ������ degree ������ ��ȯ
double	GetDistance (const double begX, const double begY, const double endX, const double endY);			// 2�������� 2�� ���� �Ÿ��� �˷���
double	GetDistance (const double begX, const double begY, const double begZ, const double endX, const double endY, const double endZ);		// 3�������� 2�� ���� �Ÿ��� �˷���
double	GetDistance (const API_Coord3D begPoint, API_Coord3D endPoint);										// 3�������� 2�� ���� �Ÿ��� �˷���
long	compareDoubles (const double a, const double b);													// � ���� �� ū�� ����
long	compareRanges (double aMin, double aMax, double bMin, double bMax);									// a�� b�� �� �� ������ ���踦 �˷���
void	exchangeDoubles (double* a, double* b);																// a�� b ���� ��ȯ��
long	findDirection (const double begX, const double begY, const double endX, const double endY);			// ���������� �������� ���ϴ� ������ ������ Ȯ��
double	distOfPointBetweenLine (API_Coord p, API_Coord a, API_Coord b);										// ���� AB�� �� P���� �Ÿ��� ���ϴ� �Լ�
API_Coord	IntersectionPoint1 (const API_Coord* p1, const API_Coord* p2, const API_Coord* p3, const API_Coord* p4);	// (p1, p2)�� ���� ������ (p3, p4)�� ���� ������ �������� ���ϴ� �Լ�
API_Coord	IntersectionPoint2 (double m1, double b1, double m2, double b2);											// y = m1*x + b1, y = m2*x + b2 �� ������ �������� ���ϴ� �Լ�
API_Coord	IntersectionPoint3 (double a1, double b1, double c1, double a2, double b2, double c2);						// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 �� ������ �������� ���ϴ� �Լ�
bool		isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint);												// aPoint�� bPoint�� ���� ������ Ȯ����
bool		isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd);		// aPoint�� pointList�� ������ �Ǿ����� Ȯ����
bool		isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint);					// nextPoint�� curPoint�� ���� ���Դϱ�?
short		moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2);								// curPoint�� ����� ���� p1, p2 �� � ���Դϱ�?
API_Coord3D	getUnrotatedPoint (API_Coord3D rotatedPoint, API_Coord3D axisPoint, double ang);					// ȸ���� ������� �ʾ��� ���� ��ġ (��ġ�Ǿ�� �� ���� ��ġ�� ����), ������ Degree

std::string	format_string(const std::string fmt, ...);														// std::string ���� ���� formatted string�� �Է� ����
GSErrCode	placeCoordinateLabel (double xPos, double yPos, double zPos, bool bComment, std::string comment, short layerInd, short floorInd);		// ��ǥ ���� ��ġ��

#endif
