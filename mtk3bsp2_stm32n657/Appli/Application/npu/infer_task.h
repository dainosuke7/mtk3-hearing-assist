#ifndef NPU_INFER_TASK_H
#define NPU_INFER_TASK_H

#include <tk/tkernel.h>

/*
 * 推論タスク (優先度15)。NPU ランタイムを持つ唯一のタスク。
 *
 * 今の中身 (Phase 1 タスク7・8): 前処理の表作りと NPU の初期化・自己テストの後、
 * タップリングから窓を取り出し、窓の番号・音量・前の窓とのつなぎ目を確かめて表示する。
 * パススルーが立ち上がってから1回、前処理 (Application/aed/preproc.c) の結果を PC と
 * 突き合わせる (タスク8。PREPROC_TEST)。
 * タスク9 で、取り出した窓に前処理と推論を載せる。
 */

/*
 * 対照実験の切替 (フェーズ3 タスク3-1)。既定値は本番 (条件 A)。
 * 条件を変えるビルドで書き換えるのは下の 2 行のどちらか 1 行だけ。共通のコードは変えない。
 * どの条件で走ったかは usermain が起動ログの先頭に "CONFIG: ..." の 1 行で出す。
 * 期待する結果は CLAUDE.md「対照実験のスイッチ」
 *
 *   INFER_PRIO_INVERT  0: 本番。推論タスクは優先度 15 (音声 5/10 より下、reporter 20 より上)
 *                      1: 条件 B。推論タスクを優先度 3 にして task_pcm (5) より上に置く
 *   INFER_SLOW_X       1: 本番
 *                      X (2 以上): 条件 D。窓ごとの推論のあとに DWT で (前処理+推論の実測)×(X-1) だけ
 *                         空回しし、窓 1 つの処理を窓の周期 (960ms) より長くする (X=10 で約 1.1 秒)
 */
#define INFER_PRIO_INVERT	(0)
#define INFER_SLOW_X		(1)

#if INFER_PRIO_INVERT && (INFER_SLOW_X > 1)
#error "INFER_PRIO_INVERT and INFER_SLOW_X: set only one (conditions B and D are exclusive)"
#endif
#if INFER_SLOW_X < 1
#error "INFER_SLOW_X must be 1 (production) or greater"
#endif

/* 推論タスクの優先度 (infer_task.c の T_CTSK と usermain の CONFIG 行が使う) */
#if INFER_PRIO_INVERT
#define INFER_TASK_PRI		(3)
#else
#define INFER_TASK_PRI		(15)
#endif

/* 起動ログの先頭に出す条件名 (数値は usermain が %d で添える) */
#if INFER_PRIO_INVERT
#define INFER_CONFIG_NAME	"B prio-invert"
#elif INFER_SLOW_X > 1
#define INFER_CONFIG_NAME	"D slow-infer"
#else
#define INFER_CONFIG_NAME	"A production"
#endif

/*
 * 推論タスクを作って起動し、NPU の初期化と自己テストが終わるまで待つ。
 * usermain から、tap_ring_init() と npu_hw_init() の後・音声を始める前に1回だけ呼ぶ。
 * 戻り値: E_OK 自己テストまで終わった (結果は UART) / E_TMOUT 待ちの上限を超えた /
 *         その他 タスクを作れなかった
 */
EXPORT ER infer_task_start(void);

#endif	/* NPU_INFER_TASK_H */
