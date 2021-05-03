/** ------------------------------------------------------------
 ** Debug ID's
 ** ------------------------------------------------------------
 ** $Id: msgdbids.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef         MSG_DEBUG_IDS_HEADER__
#define         MSG_DEBUG_IDS_HEADER__

#include        "log.h"


/**
 ** MACRO DEFINITIONS
 **/


    /** id strings      **/
#define         DB_CTRL         "ctrl"
#define         DB_REGISTER     "register"
#define         DB_RDSK         "readsock"
#define         DB_EVENT        "events"
#define         DB_WRSK         "writesock"
#define         DB_EXEC         "exec"
#define         DB_SIG          "signal"
#define         DB_FTRAN        "file trans"
#define         DB_MSG          "message"
#define         DB_MSGTYP       "msg type"
#define         DB_WRBLK        "writeblock"
#define         DB_STRTOOL      "str tools"
#define         DB_ISORT        "isort"
#define         DB_RESTART      "restart"
#define         DB_PORT         "port"
#define         DB_SELTIME      "seltime"
#define         DB_ACK          "ack"
#define         DB_SCKPT        "sock port"
#define         DB_INTLOOP      "intloop"
#define         DB_SCKTAG       "sock tag"
#define         DB_TASK_TRACK   "task tracking"
#define         DB_EXIT         "exit"


    /** flags           **/
#define         DB_CTRL_F       0x00000001
#define         DB_REGISTER_F   0x00000002
#define         DB_RDSK_F       0x00000004
#define         DB_EVENT_F      0x00000008
#define         DB_WRSK_F       0x00000010
#define         DB_EXEC_F       0x00000020
#define         DB_SIG_F        0x00000040
#define         DB_FTRAN_F      0x00000080
#define         DB_MSG_F        0x00000100
#define         DB_MSGTYP_F     0x00000200
#define         DB_WRBLK_F      0x00000400
#define         DB_STRTOOL_F    0x00000800
#define         DB_ISORT_F      0x00001000
#define         DB_RESTART_F    0x00002000
#define         DB_PORT_F       0x00004000
#define         DB_SELTIME_F    0x00008000
#define         DB_ACK_F        0x00010000
#define         DB_SCKPT_F      0x00020000
#define         DB_INTLOOP_F    0x00040000
#define         DB_SCKTAG_F     0x00080000
#define         DB_TASK_TRACK_F 0x00100000
#define         DB_EXIT_F       0x00200000

extern          int             debug;


#endif  /* MSG_DEBUG_IDS_HEADER__       */


