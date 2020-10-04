#ifndef	__UTILITY_FUNCTIONS__
#define __UTILITY_FUNCTIONS__
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include "MaxBIM.hpp"

double	DegreeToRad (double degree);																		// degree 각도를 radian 각도로 변환
double	RadToDegree (double rad);																			// radian 각도를 degree 각도로 변환
double	GetDistance (const double begX, const double begY, const double endX, const double endY);			// 2차원에서 2점 간의 거리를 알려줌
long	compareDoubles (const double a, const double b);													// 어떤 수가 더 큰지 비교함
long	compareRanges (double aMin, double aMax, double bMin, double bMax);									// a와 b의 각 값 범위의 관계를 알려줌
void	exchangeDoubles (double* a, double* b);																// a와 b 값을 교환함
long	findDirection (const double begX, const double begY, const double endX, const double endY);			// 시작점에서 끝점으로 향하는 벡터의 방향을 확인
API_Coord	IntersectionPoint1 (const API_Coord* p1, const API_Coord* p2, const API_Coord* p3, const API_Coord* p4);	// (p1, p2)를 이은 직선과 (p3, p4)를 이은 직선의 교차점을 구하는 함수
API_Coord	IntersectionPoint2 (double m1, double b1, double m2, double b2);											// y = m1*x + b1, y = m2*x + b2 두 직선의 교차점을 구하는 함수
API_Coord	IntersectionPoint3 (double a1, double b1, double c1, double a2, double b2, double c2);						// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 두 직선의 교차점을 구하는 함수

std::string	format_string(const std::string fmt, ...);														// std::string 변수 값에 formatted string을 입력 받음
GSErrCode	placeCoordinateLabel (double xPos, double yPos, double zPos, bool bComment, std::string comment, short layerInd, short floorInd);		// 좌표 라벨을 배치함
