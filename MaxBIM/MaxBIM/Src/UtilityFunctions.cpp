#include "UtilityFunctions.hpp"


////////////////////////////////////////////////// 각도 변환
// degree 각도를 radian 각도로 변환
double	DegreeToRad (double degree)
{
	return degree * M_PI / 180;
}

// radian 각도를 degree 각도로 변환
double	RadToDegree (double rad)
{
	return rad * 180 / M_PI;
}

////////////////////////////////////////////////// 거리 측정
// 2차원에서 2점 간의 거리를 알려줌
double	GetDistance (const double begX, const double begY, const double endX, const double endY)
{
	return sqrt ( (begX - endX) * (begX - endX) + (begY - endY) * (begY - endY) );
}

// 2차원에서 2점 간의 거리를 알려줌
double	GetDistance (const API_Coord begPoint, API_Coord endPoint)
{
	return sqrt ( (begPoint.x - endPoint.x) * (begPoint.x - endPoint.x) + (begPoint.y - endPoint.y) * (begPoint.y - endPoint.y) );
}

// 3차원에서 2점 간의 거리를 알려줌
double	GetDistance (const double begX, const double begY, const double begZ, const double endX, const double endY, const double endZ)
{
	return sqrt ( (begX - endX) * (begX - endX) + (begY - endY) * (begY - endY) + (begZ - endZ) * (begZ - endZ) );
}

// 3차원에서 2점 간의 거리를 알려줌
double	GetDistance (const API_Coord3D begPoint, API_Coord3D endPoint)
{
	return sqrt ( (begPoint.x - endPoint.x) * (begPoint.x - endPoint.x) + (begPoint.y - endPoint.y) * (begPoint.y - endPoint.y) + (begPoint.z - endPoint.z) * (begPoint.z - endPoint.z) );
}

// 선분 AB와 점 P와의 거리를 구하는 함수
double	distOfPointBetweenLine (API_Coord p, API_Coord a, API_Coord b)
{
	double	area;
	double	dist_ab;

	area = abs ( (a.x - p.x) * (b.y - p.y) - (a.y - p.y) * (b.x - p.x) );
	dist_ab = sqrt ( (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) );

	return	area / dist_ab;
}

////////////////////////////////////////////////// 비교하기
// 어떤 수가 더 큰지 비교함 : 오류(-100), A<B(-1), A==B(0), A>B(+1)
long	compareDoubles (const double a, const double b)
{
	// 같으면 0
	if ( (abs (a - b) < EPS) && (abs (b - a) < EPS) )
		return 0;

	// a > b이면 1
	if ( ((a - b) > 0) && abs (a - b) > EPS )
		return 1;

	// a < b이면 -1
	if ( ((a - b) < 0) && abs (a - b) > EPS )
		return -1;

	return -100;
}

// a와 b의 각 값 범위의 관계를 알려줌 (0~13 중 하나)
long	compareRanges (double aMin, double aMax, double bMin, double bMax)
{
	/*
	 *			 Min  Max
	 *  A   :	[10]-[20]
	 *  		 Min  Max
	 *  B
	 *	0	:	오류
	 *	1	:	[ 0]-[ 5]
	 *	2	:	[ 5]-[10]
	 *	3	:	[ 5]-[15]
	 *	4	:	[10]-[15]
	 *	5	:	[12]-[18]	IN
	 *	6	:	[15]-[20]
	 *	7	:	[15]-[25]
	 *	8	:	[20]-[25]
	 *	9	:	[25]-[30]
	 *	10	:	[ 5]-[20]
	 *	11	:	[10]-[25]
	 *	12	:	[10]-[20]
	 *	13	:	[ 5]-[25]
	 */
	
	long	result = 0;
	long	ab, aB, Ab, AB;
	bool	invertedA = false, invertedB = false;

	// a, b의 min, max 범위가 반대이면 뒤집어줌
	if ( aMin > aMax ) {
		exchangeDoubles (&aMin, &aMax);
		invertedA = true;
	}
	if ( bMin > bMax ) {
		exchangeDoubles (&bMin, &bMax);
		invertedB = true;
	}

	ab = compareDoubles (aMin, bMin);
	aB = compareDoubles (aMin, bMax);
	Ab = compareDoubles (aMax, bMin);
	AB = compareDoubles (aMax, bMax);

	if ( (ab  > 0) && (aB  > 0) && (Ab  > 0) && (AB  > 0) )		return 1;
	if ( (ab  > 0) && (aB == 0) && (Ab  > 0) && (AB  > 0) )		return 2;
	if ( (ab  > 0) && (aB  < 0) && (Ab  > 0) && (AB  > 0) )		return 3;
	if ( (ab == 0) && (aB  < 0) && (Ab  > 0) && (AB  > 0) )		return 4;
	if ( (ab  < 0) && (aB  < 0) && (Ab  > 0) && (AB  > 0) )		return 5;
	if ( (ab  < 0) && (aB  < 0) && (Ab  > 0) && (AB == 0) )		return 6;
	if ( (ab  < 0) && (aB  < 0) && (Ab  > 0) && (AB  < 0) )		return 7;
	if ( (ab  < 0) && (aB  < 0) && (Ab == 0) && (AB  < 0) )		return 8;
	if ( (ab  < 0) && (aB  < 0) && (Ab  < 0) && (AB  < 0) )		return 9;
	if ( (ab  > 0) && (aB  < 0) && (Ab  > 0) && (AB == 0) )		return 10;
	if ( (ab == 0) && (aB  < 0) && (Ab  > 0) && (AB  < 0) )		return 11;
	if ( (ab == 0) && (aB  < 0) && (Ab  > 0) && (AB == 0) )		return 12;
	if ( (ab  > 0) && (aB  < 0) && (Ab  > 0) && (AB  < 0) )		return 13;

	// 뒤집어준 값은 원래 상태로 복구
	if (invertedA == true)	exchangeDoubles (&aMin, &aMax);
	if (invertedB == true)	exchangeDoubles (&bMin, &bMax);

	return	result;
}

////////////////////////////////////////////////// 교환하기
// a와 b 값을 교환함
void	exchangeDoubles (double* a, double* b)
{
	double buffer;

	buffer = *a;
	*a = *b;
	*b = buffer;

	return;
}

////////////////////////////////////////////////// 기하 (일반)
// 시작점에서 끝점으로 향하는 벡터의 방향을 확인
long	findDirection (const double begX, const double begY, const double endX, const double endY)
{
	double deltaX = endX - begX;
	double deltaX_abs = abs (deltaX);
	double deltaY = endY - begY;
	double deltaY_abs = abs (deltaY);

	/*
	if ( deltaX == 0 && deltaY == 0 )	return 0;
	if ( deltaX >  0 && deltaY == 0 )	return 1;
	if ( deltaX >  0 && deltaY >  0 )	return 2;
	if ( deltaX == 0 && deltaY >  0 )	return 3;
	if ( deltaX <  0 && deltaY >  0 )	return 4;
	if ( deltaX <  0 && deltaY == 0 )	return 5;
	if ( deltaX <  0 && deltaY <  0 )	return 6;
	if ( deltaX == 0 && deltaY <  0 )	return 7;
	if ( deltaX >  0 && deltaY <  0 )	return 8;
	*/

	if ((deltaX_abs	< EPS)	&&	(deltaY_abs	< EPS))		return 0;	// Point
	if ((deltaX		> EPS)	&&	(deltaY_abs	< EPS))		return 1;	// East-ward
	if ((deltaX		> EPS)	&&	(deltaY		> EPS))		return 2;	// Northeast-ward
	if ((deltaX_abs	< EPS)	&&	(deltaY		> EPS))		return 3;	// North-ward
	if ((deltaX		< -EPS)	&&	(deltaY		> EPS))		return 4;	// Northwest-ward
	if ((deltaX		< -EPS)	&&	(deltaY_abs	< EPS))		return 5;	// West-ward
	if ((deltaX		< -EPS)	&&	(deltaY		< -EPS))	return 6;	// Southwest-ward
	if ((deltaX_abs	< EPS)	&&	(deltaY		< -EPS))	return 7;	// South-ward
	if ((deltaX		> EPS)	&&	(deltaY		< -EPS))	return 8;	// Southeast-ward

	return -1;
}

// (p1, p2)를 이은 직선과 (p3, p4)를 이은 직선의 교차점을 구하는 함수
// Function to get intersection point with line connecting points (p1, p2) and another line (p3, p4).
API_Coord	IntersectionPoint1 (const API_Coord* p1, const API_Coord* p2, const API_Coord* p3, const API_Coord* p4)
{
	API_Coord ret;
	ret.x = ((p1->x*p2->y - p1->y*p2->x)*(p3->x - p4->x) - (p1->x - p2->x)*(p3->x*p4->y - p3->y*p4->x))/( (p1->x - p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x - p4->x) );
	ret.y = ((p1->x*p2->y - p1->y*p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x*p4->y - p3->y*p4->x)) / ( (p1->x - p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x - p4->x) );
	return	ret;
}

// y = m1*x + b1, y = m2*x + b2 두 직선의 교차점을 구하는 함수
// Function to get intersection point of two lines y = m1*x + b1, y = m2*x + b2
API_Coord	IntersectionPoint2 (double m1, double b1, double m2, double b2)
{
	API_Coord ret;
	ret.x = (b2 - b1) / (m1 - m2);
	ret.y = m1*(b2 - b1)/(m1 - m2) + b1;    
	return	ret;
}

// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 두 직선의 교차점을 구하는 함수
// Function to get intersection point of two lines a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0
API_Coord	IntersectionPoint3 (double a1, double b1, double c1, double a2, double b2, double c2)
{
	API_Coord ret;
	ret.x = (b1*c2 - b2*c1)/(a1*b2 - a2*b1);
	ret.y = -a1/b1*(b1*c2-b2*c1)/(a1*b2-a2*b1)-c1/b1;
	return	ret;
}

// aPoint가 bPoint와 같은 점인지 확인함
bool	isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint)
{
	if ( (abs (aPoint.x - bPoint.x) < EPS) && (abs (aPoint.y - bPoint.y) < EPS) && (abs (aPoint.z - bPoint.z) < EPS) &&
		(abs (bPoint.x - aPoint.x) < EPS) && (abs (bPoint.y - aPoint.y) < EPS) && (abs (bPoint.z - aPoint.z) < EPS) ) {
		return true;
	} else
		return false;
}

// curPoint에 가까운 점이 p1, p2 중 어떤 점입니까?
short	moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2)
{
	double dist1, dist2;

	dist1 = GetDistance (curPoint, p1);
	dist2 = GetDistance (curPoint, p2);

	// curPoint와 p1가 더 가까우면 1 리턴
	if ((dist2 - dist1) > EPS)	return 1;
	
	// curPoint와 p2가 더 가까우면 2 리턴
	if ((dist1 - dist2) > EPS)	return 2;

	// 그 외에는 0 리턴
	return 0;
}

////////////////////////////////////////////////// 기하 (슬래브 관련)
// aPoint가 pointList에 보관이 되었는지 확인함
bool	isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd)
{
	short	xx;

	for (xx = startInd ; xx <= endInd ; ++xx) {
		// 모든 좌표 값이 일치할 경우, 이미 포함된 좌표 값이라고 인정함
		if ( (abs (aPoint.x - pointList [xx].x) < EPS) && (abs (aPoint.y - pointList [xx].y) < EPS) && (abs (aPoint.z - pointList [xx].z) < EPS) &&
			(abs (pointList [xx].x - aPoint.x) < EPS) && (abs (pointList [xx].y - aPoint.y) < EPS) && (abs (pointList [xx].z - aPoint.z) < EPS) ) {
			return true;
		}
	}

	return false;
}

// nextPoint가 curPoint의 다음 점입니까?
bool	isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint)
{
	bool	cond1 = false;
	bool	cond2_1 = false;
	bool	cond2_2 = false;

	// curPoint와 nextPoint가 같은 Z값을 갖는가?
	if ( (abs (curPoint.z - nextPoint.z) < EPS) && (abs (nextPoint.z - curPoint.z) < EPS) )
		cond1 = true;

	// 예전 점과 현재 점이 Y축 상에 있을 경우, 현재 점과 다음 점은 X축 상에 있어야 하고, 현재 점과 다음 점 간에는 X값의 차이가 있어야 함
	if ((abs (curPoint.x - prevPoint.x) < EPS) && (abs (prevPoint.x - curPoint.x) < EPS) &&
		(abs (curPoint.y - nextPoint.y) < EPS) && (abs (nextPoint.y - curPoint.y) < EPS) &&
		((abs (curPoint.x - nextPoint.x) > EPS) || (abs (nextPoint.x - curPoint.x) > EPS)))
		cond2_1 = true;

	// 예전 점과 현재 점이 X축 상에 있을 경우, 현재 점과 다음 점은 Y축 상에 있어야 하고, 현재 점과 다음 점 간에는 Y값의 차이가 있어야 함
	if ((abs (curPoint.y - prevPoint.y) < EPS) && (abs (prevPoint.y - curPoint.y) < EPS) &&
		(abs (curPoint.x - nextPoint.x) < EPS) && (abs (nextPoint.x - curPoint.x) < EPS) &&
		((abs (curPoint.y - nextPoint.y) > EPS) || (abs (nextPoint.y - curPoint.y) > EPS)))
		cond2_2 = true;

	// 같은 Z값이면서 동일 축 상의 떨어진 거리의 점인 경우
	if (cond1 && (cond2_1 || cond2_2))
		return true;
	else
		return false;
}

// 회전이 적용되지 않았을 때의 위치 (배치되어야 할 본래 위치를 리턴), 각도는 Degree
API_Coord3D		getUnrotatedPoint (API_Coord3D rotatedPoint, API_Coord3D axisPoint, double ang)
{
	API_Coord3D		unrotatedPoint;

	unrotatedPoint.x = axisPoint.x + ((rotatedPoint.x - axisPoint.x)*cos(DegreeToRad (ang)) - (rotatedPoint.y - axisPoint.y)*sin(DegreeToRad (ang)));
	unrotatedPoint.y = axisPoint.y + ((rotatedPoint.x - axisPoint.x)*sin(DegreeToRad (ang)) + (rotatedPoint.y - axisPoint.y)*cos(DegreeToRad (ang)));
	unrotatedPoint.z = rotatedPoint.z;

	return unrotatedPoint;
}

// 회전이 적용되지 않았을 때의 위치 (배치되어야 할 본래 위치를 리턴), 각도는 Degree
API_Coord	getUnrotatedPoint (API_Coord rotatedPoint, API_Coord axisPoint, double ang)
{
	API_Coord		unrotatedPoint;

	unrotatedPoint.x = axisPoint.x + ((rotatedPoint.x - axisPoint.x)*cos(DegreeToRad (ang))) - ((rotatedPoint.y - axisPoint.y)*sin(DegreeToRad (ang)));
	unrotatedPoint.y = axisPoint.y + ((rotatedPoint.x - axisPoint.x)*sin(DegreeToRad (ang))) + ((rotatedPoint.y - axisPoint.y)*cos(DegreeToRad (ang)));

	return unrotatedPoint;
}

////////////////////////////////////////////////// 산술
// 반올림
double	round (double x, int digit)
{
	return	floor((x) * pow (float (10), digit) + 0.5f) / pow (float (10), digit);
}

////////////////////////////////////////////////// 문자열
// std::string 변수 값에 formatted string을 입력 받음
std::string	format_string (const std::string fmt, ...)
{
	int			size = ((int)fmt.size ()) * 2;
	va_list		ap;
	std::string	buffer;

	while (1) {
		buffer.resize(size);
		va_start (ap, fmt);
		int n = vsnprintf ((char*)buffer.data (), size, fmt.c_str (), ap);
		va_end (ap);

		if (n > -1 && n < size) {
			buffer.resize (n);
			return buffer;
		}

		if (n > -1)
			size = n + 1;
		else
			size *= 2;
	}

	return buffer;
}

// 문자열 s가 숫자로 된 문자열인지 알려줌 (숫자는 1, 문자열은 0)
short	isStringDouble (char *str)
{
	size_t size = strlen (str);
	
	if (size == 0)
		return 0;		// 0바이트 문자열은 숫자가 아님
	
	for (size_t i = 0 ; i < size ; i++) {
		if (str [i] == '.' || str [i] == '-' || str [i] == '+')
			continue;		// .-+ 문자는 무시함
		if (str [i] < '0' || str [i] > '9')
			return 0;		// 알파벳 등이 있으면 숫자 아님
	}
	
	return 1;	// 그밖의 경우는 숫자
}

////////////////////////////////////////////////// 객체 배치
// 좌표 라벨을 배치함
GSErrCode	placeCoordinateLabel (double xPos, double yPos, double zPos, bool bComment, std::string comment, short layerInd, short floorInd)
{
	GSErrCode	err = NoError;
	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const	GS::uchar_t* gsmName = NULL;
	double	aParam;
	double	bParam;
	Int32	addParNum;

	// 라이브러리 이름 선택
	gsmName = L("좌표 19.gsm");

	// 객체 로드
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return err;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	element.object.libInd = libPart.index;
	element.object.pos.x = xPos;
	element.object.pos.y = yPos;
	element.object.level = zPos;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.header.layer = layerInd;
	element.header.floorInd = floorInd;
	memo.params [0][15].value.real = bComment;
	GS::ucscpy (memo.params [0][16].value.uStr, GS::UniString (comment.c_str ()).ToUStr ().Get ());

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

////////////////////////////////////////////////// 라이브러리 변수 접근 (Getter/Setter)
// pName 파라미터의 값을 value로 설정함 (실수형) - 성공하면 true, 실패하면 false
bool	setParameterByName (API_ElementMemo* memo, char* pName, double value)
{
	for (short xx = 0 ; xx < 500 ; ++xx) {
		if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
			memo->params [0][xx].value.real = value;
			return	true;
		}
	}

	return	false;
}

// pName 파라미터의 값을 value로 설정함 (문자열) - 성공하면 true, 실패하면 false
bool	setParameterByName (API_ElementMemo* memo, char* pName, char* value)
{
	for (short xx = 0 ; xx < 500 ; ++xx) {
		if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
			GS::ucscpy (memo->params [0][xx].value.uStr, GS::UniString (value).ToUStr ().Get ());
			return	true;
		}
	}

	return	false;
}

// pName 파라미터의 값을 가져옴 (실수형) - 성공하면 true, 실패하면 false
double	getParameterValueByName (API_ElementMemo* memo, char* pName)
{
	for (short xx = 0 ; xx < 500 ; ++xx) {
		if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
			return memo->params [0][xx].value.real;
		}
	}

	return 0.0;
}

// pName 파라미터의 값을 가져옴 (문자열) - 성공하면 true, 실패하면 false
const char*	getParameterStringByName (API_ElementMemo* memo, char* pName)
{
	const char*	retStr = NULL;

	for (short xx = 0 ; xx < 500 ; ++xx) {
		if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
			retStr = GS::UniString (memo->params [0][xx].value.uStr).ToCStr ().Get ();
			return retStr;
		}
	}

	return	retStr;
}

////////////////////////////////////////////////// 기하 (이동)
// X, Y, Z축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
void		moveIn3D (char direction, double ang, double offset, API_Coord3D* curPos)
{
	if (direction == 'x' || direction == 'X') {
		curPos->x = curPos->x + (offset * cos(ang));
		curPos->y = curPos->y + (offset * sin(ang));
	}

	if (direction == 'y' || direction == 'Y') {
		curPos->x = curPos->x - (offset * sin(ang));
		curPos->y = curPos->y + (offset * cos(ang));
	}

	if (direction == 'z' || direction == 'Z') {
		curPos->z = curPos->z + offset;
	}
}

// X, Y, Z축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
void		moveIn3D (char direction, double ang, double offset, double* curX, double* curY, double* curZ)
{
	if (direction == 'x' || direction == 'X') {
		*curX = *curX + (offset * cos(ang));
		*curY = *curY + (offset * sin(ang));
	}

	if (direction == 'y' || direction == 'Y') {
		*curX = *curX - (offset * sin(ang));
		*curY = *curY + (offset * cos(ang));
	}

	if (direction == 'z' || direction == 'Z') {
		*curZ = *curZ + offset;
	}
}

// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
void		moveIn2D (char direction, double ang, double offset, API_Coord* curPos)
{
	if (direction == 'x' || direction == 'X') {
		curPos->x = curPos->x + (offset * cos(ang));
		curPos->y = curPos->y + (offset * sin(ang));
	}

	if (direction == 'y' || direction == 'Y') {
		curPos->x = curPos->x - (offset * sin(ang));
		curPos->y = curPos->y + (offset * cos(ang));
	}
}

// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
void		moveIn2D (char direction, double ang, double offset, double* curX, double* curY)
{
	if (direction == 'x' || direction == 'X') {
		*curX = *curX + (offset * cos(ang));
		*curY = *curY + (offset * sin(ang));
	}

	if (direction == 'y' || direction == 'Y') {
		*curX = *curX - (offset * sin(ang));
		*curY = *curY + (offset * cos(ang));
	}
}
