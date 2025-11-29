#include <stddef.h>
#include "msg.h"

/* External variables */
msg_t msgs;
const char *reason;

extern void init_msg(void) {
   msgs.err.eof = "unwelcome end of file";
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
