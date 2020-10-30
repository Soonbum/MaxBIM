#ifndef	__BEAM_EUROFORM_PLACER__
#define __BEAM_EUROFORM_PLACER__

#include "MaxBIM.hpp"

namespace beamPlacerDG {
	// 객체 타입
	enum	libPartObjType {
		NONE,			// 없음
		EUROFORM,		// 유로폼v2.0
		PLYWOOD,		// 합판v1.0
		WOOD,			// 목재v1.0
		OUTCORNER_ANGLE	// 아웃코너앵글v1.0
	};

	// 부착되는 면 위치
	enum	attachedSide {
		BOTTOM_SIDE,
		LEFT_SIDE,
		RIGHT_SIDE
	};
}

// 보 관련 정보
struct InfoBeam
{
	API_Guid	guid;	// 보의 GUID

	short	floorInd;	// 층 인덱스
	double	height;		// 보 높이
	double	width;		// 보 너비
	double	offset;		// 중심으로부터 보의 레퍼런스 라인의 오프셋입니다.
	double	level;		// 바닥 레벨에 대한 보의 위쪽면 높이입니다.

	API_Coord	begC;	// 보 시작 좌표
	API_Coord	endC;	// 보 끝 좌표
};

// 그리드 각 셀 정보
struct CellForBeam
{
	short		objType;	// enum libPartObjType 참조

	API_Guid	guid;		// 객체의 GUID

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	dirLen;			// 보 설치방향 길이
	double	perLen;			// 보 직각방향 길이
	short	attachd_side;	// 하부(BOTTOM_SIDE), 좌측면(LEFT_SIDE), 우측면(RIGHT_SIDE)

	union {
		Euroform		form;
		Plywood			plywood;
		Wood			wood;
		OutcornerAngle	outangle;
	} libPart;
};

// 보 영역 정보
struct BeamPlacingZone
{
	// 보의 시작 좌표는 아래, 끝 좌표는 위라고 가정함

	// 보 기하 정보
	double	level;				// 고도
	double	ang;				// 회전 각도 (단위: Radian, 회전축: Z축)
	double	leftBottomX;		// 유로폼 시작 좌표 X
	double	leftBottomY;		// 유로폼 시작 좌표 Y
	double	leftBottomZ;		// 유로폼 시작 좌표 Z

	// 검토할 사항
	double	remain_hor_at_bottom_beg;	// 하부 보 시작 부분 남은 길이
	double	remain_hor_at_bottom_end;	// 하부 보 끝 부분 남은 길이
	double	remain_hor_at_Lside_beg;	// 좌측 보 시작 부분 남은 길이
	double	remain_hor_at_Lside_end;	// 좌측 보 끝 부분 남은 길이
	double	remain_hor_at_Rside_beg;	// 우측 보 시작 부분 남은 길이
	double	remain_hor_at_Rside_end;	// 우측 보 끝 부분 남은 길이

	double	remain_hor_at_bottom_beg_updated;	// 하부 보 시작 부분 남은 길이 (업데이트 후)
	double	remain_hor_at_bottom_end_updated;	// 하부 보 끝 부분 남은 길이 (업데이트 후)
	double	remain_hor_at_Lside_beg_updated;	// 좌측 보 시작 부분 남은 길이 (업데이트 후)
	double	remain_hor_at_Lside_end_updated;	// 좌측 보 끝 부분 남은 길이 (업데이트 후)
	double	remain_hor_at_Rside_beg_updated;	// 우측 보 시작 부분 남은 길이 (업데이트 후)
	double	remain_hor_at_Rside_end_updated;	// 우측 보 끝 부분 남은 길이 (업데이트 후)

	short	eu_count_at_bottom;		// 하부 유로폼 개수
	short	eu_count_at_Lside;		// 좌측 유로폼 개수
	short	eu_count_at_Rside;		// 우측 유로폼 개수

	double	moveOffset_at_bottom;	// 하부 이동 오프셋 (+는 끝 좌표 방향)
	double	moveOffset_at_Lside;	// 좌측 이동 오프셋 (+는 끝 좌표 방향)
	double	moveOffset_at_Rside;	// 우측 이동 오프셋 (+는 끝 좌표 방향)

	std::string		eu_wid;			// 유로폼 너비
	std::string		eu_hei;			// 유로폼 높이
	double	eu_wid_numeric;			// 유로폼 너비 (실수형)
	double	eu_hei_numeric;			// 유로폼 높이 (실수형)
};

// 유로폼 보 배치 함수
GSErrCode	placeEuroformOnBeam (void);				// 3번 메뉴: 보에 유로폼을 배치하는 통합 루틴

#endif
