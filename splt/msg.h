#ifndef MSG_H
#define MSG_H

typedef const char *msgstr_t;
typedef struct msg msg_t;

typedef struct msg_err msg_err_t;

typedef struct msg_err_syn msg_err_syn_t;
typedef struct msg_err_sem msg_err_sem_t;

typedef struct msg_err_syn_title msg_err_syn_title_t;
typedef struct msg_err_syn_dp msg_err_syn_dp_t;
typedef struct msg_err_syn_act msg_err_syn_act_t;
typedef struct msg_err_syn_scene msg_err_syn_scene_t;
typedef struct msg_err_syn_enter msg_err_syn_enter_t;
typedef struct msg_err_syn_exit msg_err_syn_exit_t;
typedef struct msg_err_syn_exeunt msg_err_syn_exeunt_t;
typedef struct msg_err_syn_line msg_err_syn_line_t;
typedef struct msg_err_syn_const msg_err_syn_const_t;
typedef struct msg_err_syn_op msg_err_syn_op_t;
typedef struct msg_err_syn_asgn msg_err_syn_asgn_t;
typedef struct msg_err_syn_out msg_err_syn_out_t;
typedef struct msg_err_syn_in msg_err_syn_in_t;
typedef struct msg_err_syn_goto msg_err_syn_goto_t;
typedef struct msg_err_syn_cond msg_err_syn_cond_t;
typedef struct msg_err_syn_if msg_err_syn_if_t;
typedef struct msg_err_syn_push msg_err_syn_push_t;
typedef struct msg_err_syn_pop msg_err_syn_pop_t;

struct msg_err_syn_title {
   msgstr_t
      incomp;
};

struct msg_err_syn_dp {
   msgstr_t
      noname,
      chardecl_incomp,
      desc_incomp,
      incomp,
      nonext;
};

struct msg_err_syn_act {
   msgstr_t
      incomp,
      badsyn,
      desc_incomp,
      noscene;
};

struct msg_err_syn_scene {
   msgstr_t
      incomp,
      badsyn,
      desc_incomp;
};

struct msg_err_syn_enter {
   msgstr_t
      incomp,
      nochar;
};

struct msg_err_syn_exit {
   msgstr_t
      incomp,
      nochar;
};

struct msg_err_syn_exeunt {
   msgstr_t
      incomp,
      onechar;
};

struct msg_err_syn_line {
   msgstr_t
      //name_incomp,
      incomp,
      nostmt;
};

struct msg_err_syn_const {
   msgstr_t
      incomp,
      deco;
};

struct msg_err_syn_op {
   msgstr_t
      //badop,
      no_of,
      no_btw,
      incomp,
      badsyn;
};

struct msg_err_syn_asgn {
   msgstr_t
      incomp,
      not_conj,
      no_as,
      no_adj;
};

struct msg_err_syn_out {
   msgstr_t
      incomp,
      badsyn,
      unmatched;
};

struct msg_err_syn_in {
   msgstr_t
      incomp,
      badsyn,
      unmatched;
};

struct msg_err_syn_goto {
   msgstr_t
      incomp,
      badsyn,
      unmatched,
      misspell;
};

struct msg_err_syn_cond {
   msgstr_t
      incomp,
      unmatched,
      badsyn;
};

struct msg_err_syn_if {
   msgstr_t
      incomp,
      badsyn,
      conseq_incomp,
      bad_conseq,
      conseq_cap;
};

struct msg_err_syn_push {
   msgstr_t
      incomp,
      badsyn;
};

struct msg_err_syn_pop {
   msgstr_t
      incomp;
};

struct msg_err_sem {
   msgstr_t
      badsyn;
};

struct msg_err_syn {
   msgstr_t
      eot,
      incomprehensible;
   msg_err_syn_title_t title;
   msg_err_syn_dp_t dp;
   msg_err_syn_act_t act;
   msg_err_syn_scene_t scene;
   msg_err_syn_enter_t enter;
   msg_err_syn_exit_t exit;
   msg_err_syn_exeunt_t exeunt;
   msg_err_syn_line_t line;
   msg_err_syn_const_t cnst;
   msg_err_syn_op_t op;
   msg_err_syn_asgn_t asgn;
   msg_err_syn_out_t out;
   msg_err_syn_in_t in;
   msg_err_syn_goto_t gt;
   msg_err_syn_cond_t cond;
   msg_err_syn_if_t ifstmt;
   msg_err_syn_push_t push;
   msg_err_syn_pop_t pop;
};

struct msg_err {
   msg_err_syn_t syn;
   msg_err_sem_t sem;
};

struct msg {
   msg_err_t err;
};

extern msg_t msgs;
extern msgstr_t reason;

void init_msg(void);

#endif
