#ifndef	__UTILITY_FUNCTIONS__
#define __UTILITY_FUNCTIONS__

#define _USE_MATH_DEFINES
#include <math.h>
#include "MaxBIM.hpp"

// 각도 변환
double	DegreeToRad (double degree);																		// degree 각도를 radian 각도로 변환
double	RadToDegree (double rad);																			// radian 각도를 degree 각도로 변환

// 거리 측정
double	GetDistance (const double begX, const double begY, const double endX, const double endY);			// 2차원에서 2점 간의 거리를 알려줌
double	GetDistance (const API_Coord begPoint, API_Coord endPoint);											// 2차원에서 2점 간의 거리를 알려줌
double	GetDistance (const double begX, const double begY, const double begZ, const double endX, const double endY, const double endZ);		// 3차원에서 2점 간의 거리를 알려줌
double	GetDistance (const API_Coord3D begPoint, API_Coord3D endPoint);										// 3차원에서 2점 간의 거리를 알려줌
double	distOfPointBetweenLine (API_Coord p, API_Coord a, API_Coord b);										// 선분 AB와 점 P와의 거리를 구하는 함수

// 비교하기
long	compareDoubles (const double a, const double b);													// 어떤 수가 더 큰지 비교함
long	compareRanges (double aMin, double aMax, double bMin, double bMax);									// a와 b의 각 값 범위의 관계를 알려줌

// 교환하기
void	exchangeDoubles (double* a, double* b);																// a와 b 값을 교환함

// 기하 (일반)
long	findDirection (const double begX, const double begY, const double endX, const double endY);			// 시작점에서 끝점으로 향하는 벡터의 방향을 확인
API_Coord	IntersectionPoint1 (const API_Coord* p1, const API_Coord* p2, const API_Coord* p3, const API_Coord* p4);	// (p1, p2)를 이은 직선과 (p3, p4)를 이은 직선의 교차점을 구하는 함수
API_Coord	IntersectionPoint2 (double m1, double b1, double m2, double b2);											// y = m1*x + b1, y = m2*x + b2 두 직선의 교차점을 구하는 함수
API_Coord	IntersectionPoint3 (double a1, double b1, double c1, double a2, double b2, double c2);						// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 두 직선의 교차점을 구하는 함수
bool		isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint);											// aPoint가 bPoint와 같은 점인지 확인함
short		moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2);							// curPoint에 가까운 점이 p1, p2 중 어떤 점입니까?

// 기하 (슬래브 관련)
bool		isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd);	// aPoint가 pointList에 보관이 되었는지 확인함
bool		isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint);				// nextPoint가 curPoint의 다음 점입니까?
API_Coord3D	getUnrotatedPoint (API_Coord3D rotatedPoint, API_Coord3D axisPoint, double ang);				// 회전이 적용되지 않았을 때의 위치 (배치되어야 할 본래 위치를 리턴), 각도는 Degree
API_Coord	getUnrotatedPoint (API_Coord rotatedPoint, API_Coord axisPoint, double ang);					// 회전이 적용되지 않았을 때의 위치 (배치되어야 할 본래 위치를 리턴), 각도는 Degree

// 산술
double	round (double x, int digit);																		// 반올림

// 문자열
std::string	format_string(const std::string fmt, ...);														// std::string 변수 값에 formatted string을 입력 받음
short	isStringDouble (char *str);																			// 문자열 s가 숫자로 된 문자열인지 알려줌 (숫자는 1, 문자열은 0)

// 객체 배치
GSErrCode	placeCoordinateLabel (double xPos, double yPos, double zPos, bool bComment = false, std::string comment = "", short layerInd = 1, short floorInd = 0);		// 좌표 라벨을 배치함

// 라이브러리 변수 접근 (Getter/Setter)
bool		setParameterByName (API_ElementMemo* memo, char* pName, double value);			// pName 파라미터의 값을 value로 설정함 (실수형) - 성공하면 true, 실패하면 false
bool		setParameterByName (API_ElementMemo* memo, char* pName, char* value);			// pName 파라미터의 값을 value로 설정함 (문자열) - 성공하면 true, 실패하면 false
double		getParameterValueByName (API_ElementMemo* memo, char* pName, double* value);	// pName 파라미터의 값을 가져옴 - 실수형
const char*	getParameterStringByName (API_ElementMemo* memo, char* pName, char* value);		// pName 파라미터의 값을 가져옴 - 문자열

// 기하 (이동)
void		moveIn3D (char direction, double ang, double offset, API_Coord3D* curPos);							// X, Y, Z축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
void		moveIn3D (char direction, double ang, double offset, double* curX, double* curY, double* curZ);		// X, Y, Z축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
void		moveIn3D (double offset, double* curZ);																// Z축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
void		moveIn2D (char direction, double ang, double offset, API_Coord* curPos);							// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
void		moveIn2D (char direction, double ang, double offset, double* curX, double* curY);					// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)

#endif
