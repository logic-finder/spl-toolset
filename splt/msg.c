#include <stddef.h>
#include "msg.h"

/* External variables */
msg_t msgs;
const char *reason;

extern void init_msg(void) {
   // lexer errors
   msgs.err.lex_eof = "unwelcome end of file";

   // parser errors
   msgs.err.syn_eot = "unwelcome end of token";
   msgs.err.syn_incomprehensible = "incomprehensible token";

   msgs.err.syn_title_incomp = "incomplete title";

   msgs.err.syn_dp_noname = "no name specified before period";
   msgs.err.syn_dp_chardecl_incomp = "incomplete character declaration";
   msgs.err.syn_dp_desc_incomp = "incomplete character description";
   msgs.err.syn_dp_incomp = "incomplete dramatis personae";
   msgs.err.syn_dp_nonext = "nothing after a character description";

   msgs.err.syn_act_incomp = "incomplete Act";
   msgs.err.syn_act_badsyn = "no colon after Act number";
   msgs.err.syn_act_desc_incomp = "incomplete Act description";
   msgs.err.syn_act_noscene = "no scene seen next";

   msgs.err.syn_scene_noact = "Scene appeared without Act";
   msgs.err.syn_scene_incomp = "incomplete Scene";
   msgs.err.syn_scene_badsyn = "no colon after Scene number";
   msgs.err.syn_scene_desc_incomp = "incomplete Scene description";

   msgs.err.syn_enter_noscene = "Enter appeared without Scene";
   msgs.err.syn_enter_incomp = "incomplete Enter";
   msgs.err.syn_enter_nochar = "Enter has no character specified";

   msgs.err.syn_exit_noscene = "Exit appeared without Scene";
   msgs.err.syn_exit_incomp = "incomplete Exit";
   msgs.err.syn_exit_nochar = "Exit has no character specified";

   msgs.err.syn_exeunt_noscene = "Exeunt appeared without Scene";
   msgs.err.syn_exeunt_incomp = "incomplete Exeunt";
   msgs.err.syn_exeunt_onechar = "Exeunt can't have one character";

   msgs.err.syn_line_noscene = "Line appeared without Scene";
   msgs.err.syn_line_name_incomp = "Line has an incomplete name";
   msgs.err.syn_line_incomp = "incomplete Line";

   msgs.err.syn_const_incomp = "incomplete constant";
   msgs.err.syn_const_deco = "incorrect use of possessive or article";

   msgs.err.syn_op_badop = "incomprehensible operator";
   msgs.err.syn_op_incomp = "incomplete operator";
   msgs.err.syn_op_badsyn = "incorrect operator syntax";

   msgs.err.syn_asgn_incomp = "incomplete assignment statement";
   msgs.err.syn_asgn_not_conj = "incorrect conjugation";
   msgs.err.syn_asgn_noas = "\"as\" not present in this assignment";
   msgs.err.syn_asgn_noadj = "no adjective following \"as\"";

   msgs.err.syn_out_incomp = "incomplete output statement";
   msgs.err.syn_out_badsyn = "incorrect output statement syntax";
   msgs.err.syn_out_unmatched = "incomprehensible output statement";

   msgs.err.syn_in_incomp = "incomplete input statement";
   msgs.err.syn_in_badsyn = "incorrect input statement syntax";
   msgs.err.syn_in_unmatched = "incomprehensible input statement";

   msgs.err.syn_goto_incomp = "incomplete goto statement";
   msgs.err.syn_goto_badsyn = "incorrect goto statement syntax";
   msgs.err.syn_goto_unmatched = "incomprehensible goto statement";

   msgs.err.syn_cond_incomp = "incomplete interrogative sentence";
   msgs.err.syn_cond_unmatched = "incomprehensible interrogative sentence";
   msgs.err.syn_cond_badsyn = "incorrect interrogative sentence syntax";

   msgs.err.syn_if_incomp = "incomplete conditional statement";
   msgs.err.syn_if_badsyn = "incorrect conditional statement syntax";

   msgs.err.syn_push_incomp = "incomplete push statement";
   msgs.err.syn_push_badsyn = "incorrect push statement syntax";

   msgs.err.syn_pop_incomp = "incomplete pop statement";




   msgs.err.inctok = "incomprehensible token";
   msgs.err.notitle = "no title present";
   msgs.err.ontitle = "title not finished";
   msgs.err.onname = "name not finished in character declaration";
   msgs.err.onchardesc = "character description not finished";
   msgs.err.act_unfinished = "Act not finished";
   msgs.err.act_no_space = "no space following Act";
   msgs.err.act_no_colon = "no colon following numeral in this Act";
   msgs.err.act_desc_unfinished = "Act description not finished";
   msgs.err.act_no_period = "Act description not finished with period";
   msgs.err.onscenedesc = "scene description not finished";
   msgs.err.enter_on_stmt = "Enter not closed";
   msgs.err.noenterchar = "no character specified in Enter";
   msgs.err.nowsafterenter = "no whitespace following Enter";
   msgs.err.enter_no_connective = "no \"and\" before last character in Enter";
   msgs.err.exit_on_stmt = "Exit not closed";
   msgs.err.exit_no_char = "no character specified in Exit";
   msgs.err.exit_no_space_after = "no whitespace following Exit";
   msgs.err.exeunt_on_stmt = "Exeunt not closed";
   msgs.err.exeunt_no_space_after = "no whitespace following Exeunt";
   msgs.err.exeunt_no_connective = "no \"and\" before last character in Exeunt";
   msgs.err.exeunt_only_one_name = "only one character specified in Exeunt";
   msgs.err.asgn_i_unfinished = "assignment not finished";
   msgs.err.asgn_ii_unfinished = "assignment not finished";
   msgs.err.asgn_ii_no_space = "no space following";
   msgs.err.asgn_ii_invalid_adj = "incomprehensible adjective";
   msgs.err.asgn_ii_no_as = "no \"as\" following the preceding adjective";
   msgs.err.out_i_unfinished = "output statement not finished";
   msgs.err.out_i_wrong_syntax = "output statement with bad syntax";
   msgs.err.out_i_no_space = "no space following in this output statement";
   msgs.err.out_ii_unfinished = "output statement not finished";
   msgs.err.out_ii_wrong_syntax = "output statement with bad syntax";
   msgs.err.out_ii_no_space = "no space following in this output statement";
   msgs.err.in_i_unfinished = "input statement not finished";
   msgs.err.in_i_wrong_syntax = "input statement with bad syntax";
   msgs.err.in_i_no_space = "no space following in this input statement";
   msgs.err.in_ii_unfinished = "input statement not finished";
   msgs.err.in_ii_wrong_syntax = "input statement with bad syntax";
   msgs.err.goto_unfinished = "goto statement not finished";
   msgs.err.goto_synerr = "goto statement with bad syntax";
   msgs.err.goto_no_space = "no space following in this goto statement";
   msgs.err.antec_unfinished = "antecedent not finished";
   msgs.err.antec_not_conjugated = "subject and verb not conjugated in this antecedent";
   msgs.err.antec_no_space = "no space following in this antecedent";
   msgs.err.antec_bad_syntax = "antecedent with bad syntax";
   msgs.err.conse_unfinished = "consequent not finished";
   msgs.err.conse_bad_syntax = "consequent with bad syntax";
   msgs.err.push_unfinished = "push statement not finished";
   msgs.err.push_bad_syntax = "push statement with bad syntax";
   msgs.err.pop_unfinished = "pop statement not finished";
}
