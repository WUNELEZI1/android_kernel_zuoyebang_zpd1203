#ifndef __LCT_TP_EARPHONE_H__
#define __LCT_TP_EARPHONE_H__

extern int tp_earphone_init(int (*set_cb)(bool));
extern void tp_earphone_uninit(void);
extern void set_lct_tp_work_status(bool en);
extern bool get_lct_tp_work_status(void);

#endif //__LCT_TP_EARPHONE_H__
