/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2019 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 * 
 *  上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  $Id: sample1.c 1237 2019-07-10 01:00:30Z ertl-hiro $
 */

/* 
 *  サンプルプログラム(1)の本体
 *
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "sample1.h"

#include <mruby.h>
#include <mruby/compile.h>
#include "mruby/irep.h"
#include "mruby/debug.h"
#include "mruby/opcode.h"
#include "mruby/value.h"
#include "mruby/string.h"
#include "mruby/array.h"
#include "mruby/proc.h"
#include "mruby_tlsf.h"


DECLARE_RUNTIME_MRBGEM(mruby_f401re);

#define RETURN_BUF_SIZE 4096
#define BUF_SIZE 256
extern struct mrb_state mrb_state_nv;
mrb_state *mrb;
mrb_state *mrb_task1;
struct mrb_tlsf_t *t;
struct mrb_tlsf_t *t2;

#define MEMORY_POOL_SIZE	(TOPPERS_ROUND_SZ(50*1024, sizeof(intptr_t)))
							/* 10*1024の部分は，適切なサイズに変更する */
intptr_t memory_pool[MEMORY_POOL_SIZE / sizeof(intptr_t)];

/*
 *  サービスコールのエラーのログ出力
 */
Inline void
svc_perror(const char *file, int_t line, const char *expr, ER ercd)
{
	if (ercd < 0) {
		t_perror(LOG_ERROR, file, line, expr, ercd);
	}
}

#define	SVC_PERROR(expr)	svc_perror(__FILE__, __LINE__, #expr, (expr))

/*
 *  プロセッサ時間の消費
 *
 *  ループによりプロセッサ時間を消費する．最適化ができないように，ルー
 *  プ内でvolatile変数を読み込む．
 */
static volatile long_t	volatile_var;

static void
consume_time(ulong_t ctime)
{
	ulong_t		i;

	for (i = 0; i < ctime; i++) {
		(void) volatile_var;
	}
}

/*
 *  並行実行されるタスクへのメッセージ領域
 */
char	message[3];

/*
 *  ループ回数
 */
ulong_t	task_loop;		/* タスク内でのループ回数 */

/*
 *  並行実行されるタスク
 */
void
task1(intptr_t exinf)
{
//	syslog(LOG_INFO, "task1 start\n");
  // mruby use_presym version
  mrb_task1 = mrb_load_state_core(&mrb_state_nv, mrb_tlsf_allocf, (void *)t);

  if (!mrb_task1) {
		//TODO エラー処理
	  syslog(LOG_INFO, "t1 ruby cannot opened...\n");
//    exit(EXIT_FAILURE);
  }
  mrb_value v = mrb_true_value();
  mrb_value v_size = mrb_tlsf_used_memory(mrb_task1, v);
  mrb_int size = mrb_fixnum(v_size);
//  syslog(LOG_INFO, "t1 used:%d", size);

  LOAD_RUNTIME_MRBGEM(mrb_task1, mruby_tlsf);
  LOAD_RUNTIME_MRBGEM(mrb_task1, mruby_f401re);
//  LOAD_RUNTIME_MRBGEM(mruby_print);

	struct RClass * f401 = mrb_class_get(mrb_task1, "F401re");

	mrb_define_const(mrb_task1, f401, "TASK1_ID", mrb_fixnum_value(TASK1));
	mrb_define_const(mrb_task1, f401, "MAIN_TASK_ID", mrb_fixnum_value(MAIN_TASK));

  #include "task1.h"
  mrb_value ret = mrb_load_irep(mrb_task1, task1_code);
	if(mrb_task1->exc){
		syslog(LOG_INFO, "---task1 load irep OK ---\r\n");
		if(!mrb_undef_p(ret)){
			mrb_value s = mrb_funcall(mrb_task1, mrb_obj_value(mrb_task1->exc), "inspect", 0);
			if (mrb_string_p(s)) {
			  char *p = RSTRING_PTR(s);
			  syslog(LOG_INFO, "t1 mruby err msg:%s", p);
			} else {
			  syslog(LOG_INFO, "t1 error unknown\r\n");
			}
		}else{
			syslog(LOG_INFO, "--- mrb_undef_p(t1 ret) ---\r\n");
		}
	}else{
    mrb_value v_size = mrb_tlsf_used_memory(mrb_task1, v);
    mrb_int size = mrb_fixnum(v_size);
    syslog(LOG_INFO, "task1 total used:%d", size);
	  syslog(LOG_INFO, "--- (!mrb_task1->exe) ---\r\n");
	}

  mrb_close_tlsf(mrb_task1);

}


/*
 *  割込みサービスルーチン
 *
 *  HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY の各優先度のレディキュー
 *  を回転させる．
 */
#ifdef INTNO1

void
intno1_isr(intptr_t exinf)
{
	intno1_clear();
	SVC_PERROR(rot_rdq(HIGH_PRIORITY));
	SVC_PERROR(rot_rdq(MID_PRIORITY));
	SVC_PERROR(rot_rdq(LOW_PRIORITY));
}

#endif /* INTNO1 */

/*
 *  CPU例外ハンドラ
 */
ID	cpuexc_tskid;		/* CPU例外を起こしたタスクのID */

#ifdef CPUEXC1

void
cpuexc_handler(void *p_excinf)
{
	syslog(LOG_NOTICE, "CPU exception handler (p_excinf = %08p).", p_excinf);
	if (sns_ctx() != true) {
		syslog(LOG_WARNING,
					"sns_ctx() is not true in CPU exception handler.");
	}
	if (sns_dpn() != true) {
		syslog(LOG_WARNING,
					"sns_dpn() is not true in CPU exception handler.");
	}
	syslog(LOG_INFO, "sns_loc = %d, sns_dsp = %d, xsns_dpn = %d",
								sns_loc(), sns_dsp(), xsns_dpn(p_excinf));

	if (xsns_dpn(p_excinf)) {
		syslog(LOG_NOTICE, "Sample program ends with exception.");
		SVC_PERROR(ext_ker());
		assert(0);
	}

	SVC_PERROR(get_tid(&cpuexc_tskid));
	SVC_PERROR(act_tsk(EXC_TASK));
}

#endif /* CPUEXC1 */

/*
 *  周期ハンドラ
 *
 *  HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY の各優先度のレディキュー
 *  を回転させる．
 */
void
cyclic_handler(intptr_t exinf)
{
	SVC_PERROR(rot_rdq(HIGH_PRIORITY));
	SVC_PERROR(rot_rdq(MID_PRIORITY));
	SVC_PERROR(rot_rdq(LOW_PRIORITY));
}

/*
 *  アラームハンドラ
 *
 *  HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY の各優先度のレディキュー
 *  を回転させる．
 */
void
alarm_handler(intptr_t exinf)
{
	SVC_PERROR(rot_rdq(HIGH_PRIORITY));
	SVC_PERROR(rot_rdq(MID_PRIORITY));
	SVC_PERROR(rot_rdq(LOW_PRIORITY));
}

/*
 *  例外処理タスク
 */
void
exc_task(intptr_t exinf)
{
	SVC_PERROR(ras_ter(cpuexc_tskid));
}

/*
 *  メインタスク
 */
void
main_task(intptr_t exinf)
{
	char	c;
	ID		tskid = TASK1;
	int_t	tskno = 1;
	ER_UINT	ercd;
	PRI		tskpri;
#ifndef TASK_LOOP
	SYSTIM	stime1, stime2;
#endif /* TASK_LOOP */
	HRTCNT	hrtcnt1, hrtcnt2;

	//TLSFを使ったメモリプールの初期化
	init_memory_pool(MEMORY_POOL_SIZE, memory_pool);

	SVC_PERROR(syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
	syslog(LOG_NOTICE, "Sample program starts (exinf = %d).", (int_t) exinf);

	/*
	 *  シリアルポートの初期化
	 *
	 *  システムログタスクと同じシリアルポートを使う場合など，シリアル
	 *  ポートがオープン済みの場合にはここでE_OBJエラーになるが，支障は
	 *  ない．
	 */
	ercd = serial_opn_por(TASK_PORTID);
	if (ercd < 0 && MERCD(ercd) != E_OBJ) {
		syslog(LOG_ERROR, "%s (%d) reported by `serial_opn_por'.",
									itron_strerror(ercd), SERCD(ercd));
	}
	SVC_PERROR(serial_ctl_por(TASK_PORTID,
							(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV)));

	/*
 	 *  ループ回数の設定
	 *
	 *  並行実行されるタスク内でのループの回数（task_loop）は，ループ
	 *  の実行時間が約0.4秒になるように設定する．この設定のために，
	 *  LOOP_REF回のループの実行時間を，その前後でget_timを呼ぶことで
	 *  測定し，その測定結果から空ループの実行時間が0.4秒になるループ
	 *  回数を求め，task_loopに設定する．
	 *
	 *  LOOP_REFは，デフォルトでは1,000,000に設定しているが，想定した
	 *  より遅いプロセッサでは，サンプルプログラムの実行開始に時間がか
	 *  かりすぎるという問題を生じる．逆に想定したより速いプロセッサで
	 *  は，LOOP_REF回のループの実行時間が短くなり，task_loopに設定す
	 *  る値の誤差が大きくなるという問題がある．そこで，そのようなター
	 *  ゲットでは，target_test.hで，LOOP_REFを適切な値に定義すること
	 *  とする．
	 *
	 *  また，task_loopの値を固定したい場合には，その値をTASK_LOOPにマ
	 *  クロ定義する．TASK_LOOPがマクロ定義されている場合，上記の測定
	 *  を行わずに，TASK_LOOPに定義された値をループの回数とする．
	 *
	 *  ターゲットによっては，ループの実行時間の1回目の測定で，本来より
	 *  も長めになるものがある．このようなターゲットでは，MEASURE_TWICE
	 *  をマクロ定義することで，1回目の測定結果を捨てて，2回目の測定結
	 *  果を使う．
	 */
#ifdef TASK_LOOP
	task_loop = TASK_LOOP;
#else /* TASK_LOOP */

#ifdef MEASURE_TWICE
	SVC_PERROR(get_tim(&stime1));
	consume_time(LOOP_REF);
	SVC_PERROR(get_tim(&stime2));
#endif /* MEASURE_TWICE */

	SVC_PERROR(get_tim(&stime1));
	consume_time(LOOP_REF);
	SVC_PERROR(get_tim(&stime2));
	task_loop = LOOP_REF * 400LU / (ulong_t)(stime2 - stime1) * 1000LU;

#endif /* TASK_LOOP */

  // mruby use_presym version
  t = mrb_tlsf_init((void *)memory_pool, MEMORY_POOL_SIZE);
  mrb = mrb_load_state_core(&mrb_state_nv, mrb_tlsf_allocf, (void *)t);
  if (!mrb) {
		//TODO エラー処理
	  syslog(LOG_INFO, "ruby cannot opened...\n");
//    exit(EXIT_FAILURE);
  }
  mrb_value v = mrb_true_value();

  mrb_value v_size = mrb_tlsf_used_memory(mrb, v);
  mrb_int size = mrb_fixnum(v_size);
  syslog(LOG_INFO, "used:%d", size);

  LOAD_RUNTIME_MRBGEM(mrb, mruby_tlsf);
  LOAD_RUNTIME_MRBGEM(mrb, mruby_f401re);

	struct RClass * f401 = mrb_class_get(mrb, "F401re");

	mrb_define_const(mrb, f401, "TASK1_ID", mrb_fixnum_value(TASK1));
	mrb_define_const(mrb, f401, "MAIN_TASK_ID", mrb_fixnum_value(MAIN_TASK));

  #include "main_task.h"
  mrb_value ret = mrb_load_irep(mrb, code);
	if(mrb->exc){
		syslog(LOG_INFO, "--- load irep OK ---\r\n");
		if(!mrb_undef_p(ret)){
			mrb_value s = mrb_funcall(mrb, mrb_obj_value(mrb->exc), "inspect", 0);
			if (mrb_string_p(s)) {
			  char *p = RSTRING_PTR(s);
			  syslog(LOG_INFO, "mruby err msg:%s", p);
			} else {
			  syslog(LOG_INFO, "error unknown\r\n");
			}
		}else{
			syslog(LOG_INFO, "--- mrb_undef_p(ret) ---\r\n");
		}
	}else{
    mrb_value v_size = mrb_tlsf_used_memory(mrb, v);
    mrb_int size = mrb_fixnum(v_size);
    syslog(LOG_INFO, "total used:%d", size);
	  syslog(LOG_INFO, "--- (!mrb->exe) ---\r\n");
	}

  mrb_close_tlsf(mrb);
  syslog(LOG_NOTICE, "Sample program ends.");
	SVC_PERROR(ext_ker());
	assert(0);
}
