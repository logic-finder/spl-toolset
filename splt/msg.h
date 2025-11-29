#ifndef MSG_H
#define MSG_H

typedef const char *msgstr_t;

typedef struct errmsg {
   msgstr_t
      eof,
      inctok,
      notitle,
      ontitle,
      onname,
      onchardesc,
      act_unfinished,
      act_no_space,
      act_no_colon,
      act_desc_unfinished,
      act_no_period,
      onscenedesc,
      enter_on_stmt,
      noenterchar,
      nowsafterenter,
      enter_no_connective,
      exit_on_stmt,
      exit_no_char,
      exit_no_space_after,
      exeunt_on_stmt,
      exeunt_no_space_after,
      exeunt_no_connective,
      exeunt_only_one_name,
      asgn_i_unfinished,
      asgn_ii_unfinished,
      asgn_ii_no_space,
      asgn_ii_invalid_adj,
      asgn_ii_no_as,
      out_i_unfinished,
      out_i_wrong_syntax,
      out_i_no_space,
      out_ii_unfinished,
      out_ii_wrong_syntax,
      out_ii_no_space,
      in_i_unfinished,
      in_i_wrong_syntax,
      in_i_no_space,
      in_ii_unfinished,
      in_ii_wrong_syntax,
      goto_unfinished,
      goto_synerr,
      goto_no_space,
      antec_unfinished,
      antec_not_conjugated,
      antec_no_space,
      antec_bad_syntax,
      conse_unfinished,
      conse_bad_syntax,
      push_unfinished,
      push_bad_syntax,
      pop_unfinished,
      dummy;
} errmsg_t;

typedef struct msg {
   errmsg_t err;
} msg_t;

extern msg_t msgs;
extern const char *reason;

void init_msg(void);

#endif
