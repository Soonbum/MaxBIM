#ifndef	__UTILITY_FUNCTIONS__
#define __UTILITY_FUNCTIONS__
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include "MaxBIM.hpp"

double	DegreeToRad (double degree);																		// degree ������ radian ������ ��ȯ
double	RadToDegree (double rad);																			// radian ������ degree ������ ��ȯ
double	GetDistance (const double begX, const double begY, const double endX, const double endY);			// 2�������� 2�� ���� �Ÿ��� �˷���
long	compareDoubles (const double a, const double b);													// � ���� �� ū�� ����
long	compareRanges (double aMin, double aMax, double bMin, double bMax);									// a�� b�� �� �� ������ ���踦 �˷���
void	exchangeDoubles (double* a, double* b);																// a�� b ���� ��ȯ��
long	findDirection (const double begX, const double begY, const double endX, const double endY);			// ���������� �������� ���ϴ� ������ ������ Ȯ��
API_Coord	IntersectionPoint1 (const API_Coord* p1, const API_Coord* p2, const API_Coord* p3, const API_Coord* p4);	// (p1, p2)�� ���� ������ (p3, p4)�� ���� ������ �������� ���ϴ� �Լ�
API_Coord	IntersectionPoint2 (double m1, double b1, double m2, double b2);											// y = m1*x + b1, y = m2*x + b2 �� ������ �������� ���ϴ� �Լ�
API_Coord	IntersectionPoint3 (double a1, double b1, double c1, double a2, double b2, double c2);						// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 �� ������ �������� ���ϴ� �Լ�

std::string	format_string(const std::string fmt, ...);														// std::string ���� ���� formatted string�� �Է� ����
GSErrCode	placeCoordinateLabel (double xPos, double yPos, double zPos, bool bComment, std::string comment, short layerInd, short floorInd);		// ��ǥ ���� ��ġ��
