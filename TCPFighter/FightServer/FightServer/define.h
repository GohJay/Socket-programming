#pragma once

//-----------------------------------------------------------------
// 기본 정보
//-----------------------------------------------------------------
#define dfSERVER_NAME			"FightServer"
#define dfSERVER_PORT			5000
#define dfFRAME					50
#define dfINTERVAL				1000 / dfFRAME
#define dfMAX_PLAYER			60
#define dfPLAYER_HP				100

//-----------------------------------------------------------------
// 프레임당 이동 단위
//-----------------------------------------------------------------
#define dfPLAYER_SPEED_X		3
#define dfPLAYER_SPEED_Y		2

//-----------------------------------------------------------------
// 화면 이동영역
//-----------------------------------------------------------------
#define dfRANGE_MOVE_TOP		50
#define dfRANGE_MOVE_LEFT		10
#define dfRANGE_MOVE_RIGHT		630
#define dfRANGE_MOVE_BOTTOM		470

//-----------------------------------------------------------------
// 이동 오류체크 범위
//-----------------------------------------------------------------
#define dfERROR_RANGE			50

//-----------------------------------------------------------------
// 공격 범위
//-----------------------------------------------------------------
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20

//-----------------------------------------------------------------
// 공격 데미지
//-----------------------------------------------------------------
#define dfATTACK1_DAMAGE		1
#define dfATTACK2_DAMAGE		2
#define dfATTACK3_DAMAGE		3
