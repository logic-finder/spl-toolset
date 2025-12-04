#ifndef MSG_H
#define MSG_H

typedef const char *msgstr_t;

typedef struct errmsg {
   msgstr_t
      lex_eof,

      syn_eot,
      syn_incomprehensible,

      syn_title_incomp,

      syn_dp_noname,
      syn_dp_chardecl_incomp,
      syn_dp_desc_incomp,
      syn_dp_incomp,
      syn_dp_nonext,

      syn_act_incomp,
      syn_act_badsyn,
      syn_act_desc_incomp,
      syn_act_noscene,

      syn_scene_noact,
      syn_scene_incomp,
      syn_scene_badsyn,
      syn_scene_desc_incomp,

      syn_enter_noscene,
      syn_enter_incomp,
      syn_enter_nochar,

      syn_exit_noscene,
      syn_exit_incomp,
      syn_exit_nochar,

      syn_exeunt_noscene,
      syn_exeunt_incomp,
      syn_exeunt_onechar,

      syn_line_noscene,
      syn_line_name_incomp,
      syn_line_incomp,

      syn_const_incomp,
      syn_const_deco,

      syn_op_badop,
      syn_op_incomp,
      syn_op_badsyn,

      syn_asgn_incomp,
      syn_asgn_not_conj,
      syn_asgn_noas,
      syn_asgn_noadj,

      syn_out_incomp,
      syn_out_badsyn,
      syn_out_unmatched,

      syn_in_incomp,
      syn_in_badsyn,
      syn_in_unmatched,

      syn_goto_incomp,
      syn_goto_badsyn,
      syn_goto_unmatched,

      syn_cond_incomp,
      syn_cond_unmatched,
      syn_cond_badsyn,

      syn_if_incomp,
      syn_if_badsyn,

      syn_push_incomp,
      syn_push_badsyn,

      syn_pop_incomp,



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
