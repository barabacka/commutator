/*
 * defs.h
 *
 *  Created on: Dec 16, 2024
 *      Author: Barabacka
 */

#ifndef INC_DEFS_H_
#define INC_DEFS_H_

#define VERSION_MAJOR	1
#define VERSION_MINOR	2
#define VERSION_PATCH	0

typedef enum{
	IS_URX= 0,
	IS_MAX,
}T_IT_SRC;

#define ITSRC(a)	(1<<a)
#define ITSRC_URX	ITSRC(IS_URX)

#define CMD_TV_ON		'T'
#define CMD_MUSIC_ON	'M'
#define CMD_OFF			'O'
#define CMD_GET_STATE	'G'
#define CMD_SMALL(L)	(L +('a'-'A'))


typedef enum{
	TURN_MUSIC_ON = 0,
	TURN_TV_ON,
	TURN_MUSIC_OFF,
	TURN_TV_OFF,
	TURN_ALL_OFF,
	TURN_MAX
}T_SWITCH_CTRL;

void switch_it ( T_SWITCH_CTRL state );

#endif /* INC_DEFS_H_ */
