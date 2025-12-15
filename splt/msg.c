#include <stddef.h>
#include "msg.h"
#include "colorcode.h"

/* External variables */
msg_t msgs;
const char *reason;

extern void init_msg(void) {
   /* Parser Errors */
   msgs.err.syn.eot = "unwelcome end of token";
   msgs.err.syn.incomprehensible = "incomprehensible token";

   msgs.err.syn.title.incomp = "incomplete title";

   msgs.err.syn.dp.noname = "no name specified before period";
   msgs.err.syn.dp.chardecl_incomp = "incomplete character declaration";
   msgs.err.syn.dp.desc_incomp = "incomplete character description";
   msgs.err.syn.dp.incomp = "incomplete dramatis personae";
   msgs.err.syn.dp.nonext = "nothing after a character description";

   msgs.err.syn.act.incomp = "incomplete Act";
   msgs.err.syn.act.badsyn = "no colon after Act number";
   msgs.err.syn.act.desc_incomp = "incomplete Act description";
   msgs.err.syn.act.noscene = "no scene seen next";

   msgs.err.syn.scene.incomp = "incomplete Scene";
   msgs.err.syn.scene.badsyn = "no colon after Scene number";
   msgs.err.syn.scene.desc_incomp = "incomplete Scene description";

   msgs.err.syn.enter.incomp = "incomplete Enter";
   msgs.err.syn.enter.nochar = "Enter has no character specified";
   msgs.err.syn.enter.exceed = "Enter has too many characters";
   msgs.err.syn.enter.badname = "incomprehensible name in Enter";

   msgs.err.syn.exit.incomp = "incomplete Exit";
   msgs.err.syn.exit.nochar = "Exit has no character specified";
   msgs.err.syn.exit.exceed = "Exit has too many characters";
   msgs.err.syn.exit.badname = "incomprehensible name in Exit";

   msgs.err.syn.exeunt.incomp = "incomplete " Cbyellow "Exeunt" Creset;
   msgs.err.syn.exeunt.exceed = "Exeunt has too many characters";
   msgs.err.syn.exeunt.onechar = Cbyellow "Exeunt" Creset " must have either 0 or 2 characters; currently 1";
   msgs.err.syn.exeunt.badname = "incomprehensible name in Exeunt";

   msgs.err.syn.line.incomp = "incomplete Line";
   msgs.err.syn.line.nostmt = "no statement present in Line";

   msgs.err.syn.cnst.incomp = "incomplete constant";
   msgs.err.syn.cnst.deco = "incorrect use of possessive or article";

   msgs.err.syn.op.incomp = "incomplete operator";
   msgs.err.syn.op.no_and = "'and' must be between two constants";
   msgs.err.syn.op.sum  = "incorrect operator syntax; " Cbyellow "sum" Creset " needs '" Cbwhite "of" Creset "' here";
   msgs.err.syn.op.diff = "incorrect operator syntax; " Cbyellow "difference" Creset " needs '" Cbwhite "between" Creset "' here";
   msgs.err.syn.op.prod = "incorrect operator syntax; " Cbyellow "product" Creset " needs '" Cbwhite "of" Creset "' here";
   msgs.err.syn.op.quot = "incorrect operator syntax; " Cbyellow "quotient" Creset " needs '" Cbwhite "between" Creset "' here";
   msgs.err.syn.op.rem  = "incorrect operator syntax; " Cbyellow "remainder" Creset " needs '" Cbwhite "of" Creset "' here";
   msgs.err.syn.op.sqrt = "incorrect operator syntax; " Cbyellow "square root" Creset " needs '" Cbwhite "of" Creset "' here";
   msgs.err.syn.op.squr = "incorrect operator syntax; " Cbyellow "square" Creset " needs '" Cbwhite "of" Creset "' here";
   msgs.err.syn.op.cube = "incorrect operator syntax; " Cbyellow "cube" Creset " needs '" Cbwhite "of" Creset "' here";
   msgs.err.syn.op.fact = "incorrect operator syntax; " Cbyellow "factorial" Creset " needs '" Cbwhite "of" Creset "' here";
   msgs.err.syn.op.rem_quot_1 = "incorrect " Cbyellow "remainder" Creset " operator syntax; '" Cbwhite "the" Creset "' required here";
   msgs.err.syn.op.rem_quot_2 = "incorrect " Cbyellow "remainder" Creset " operator syntax; '" Cbwhite "quotient" Creset "' required here";

   msgs.err.syn.asgn.incomp = "incomplete assignment statement";
   msgs.err.syn.asgn.not_conj = "incorrect conjugation";
   msgs.err.syn.asgn.no_as = "\"as\" not present in this assignment";
   msgs.err.syn.asgn.no_adj = "no adjective following \"as\"";

   msgs.err.syn.out.incomp = "incomplete output statement";
   msgs.err.syn.out.badsyn = "incorrect output statement syntax";
   msgs.err.syn.out.unmatched = "incomprehensible output statement";

   msgs.err.syn.in.incomp = "incomplete input statement";
   msgs.err.syn.in.badsyn = "incorrect input statement syntax";
   msgs.err.syn.in.unmatched = "incomprehensible input statement";

   msgs.err.syn.gt.incomp = "incomplete goto statement";
   msgs.err.syn.gt.badsyn = "incorrect goto statement syntax";
   msgs.err.syn.gt.unmatched = "incomprehensible goto statement";
   msgs.err.syn.gt.act_misspell = "misspelling detected; it's Act, not act";
   msgs.err.syn.gt.scene_misspell = "misspelling detected; it's Scene, not scene";

   msgs.err.syn.cond.incomp = "incomplete interrogative sentence";
   msgs.err.syn.cond.unmatched = "incomprehensible interrogative sentence";
   msgs.err.syn.cond.badsyn = "incorrect interrogative sentence syntax";

   msgs.err.syn.ifstmt.incomp = "incomplete conditional statement";
   msgs.err.syn.ifstmt.badsyn = "incorrect conditional statement syntax";
   msgs.err.syn.ifstmt.conseq_incomp = "incomplete consequent";
   msgs.err.syn.ifstmt.bad_conseq = "incomprehensible consequent";
   msgs.err.syn.ifstmt.conseq_cap = "consequent begins with an uppercase";

   msgs.err.syn.push.incomp = "incomplete push statement";
   msgs.err.syn.push.badsyn = "incorrect push statement syntax";

   msgs.err.syn.pop.incomp = "incomplete pop statement";

   msgs.err.sem.bad_adj = "incomprehensible adjective";
   msgs.err.sem.bad_noun = "incomprehensible noun";
   msgs.err.sem.bad_comp = "incomprehensible comparative";
   msgs.err.sem.bad_rnum = "incomprehensible roman numeral";

   msgs.err.sem.emptystage = "attempt to Exeunt with empty stage";
   msgs.err.sem.already_onstage = "attempt to Enter " Cbyellow "%s" Creset " who is already onstage; this is going to cause a runtime error";
   msgs.err.sem.already_offstage = "attempt to Exit " Cbyellow "%s" Creset " who is already offstage; this is going to cause a runtime error";
   msgs.err.sem.not_onstage = "Line by offstage character " Cbyellow "%s" Creset;
   msgs.err.sem.solitary  = "the reference by 2nd pronoun 'you' is unclear in this context since there is only " Cbyellow "%d" Creset " onstage character";
   msgs.err.sem.vague_ref = "the reference by 2nd pronoun 'you' is unclear in this context since there are " Cbyellow "%d" Creset " onstage characters";
   msgs.err.sem.solpush = "tried to push with only " Cbyellow "%d" Creset " onstage character";
   msgs.err.sem.mulpush = "tried to push with " Cbyellow "%d" Creset " characters onstage";
   msgs.err.sem.solpop = "tried to pop with only " Cbyellow "%d" Creset " onstage character";
   msgs.err.sem.mulpop = "tried to pop with " Cbyellow "%d" Creset " characters onstage";
   msgs.err.sem.no_such_act = "attempt of jump to nonexistent Act " Cbyellow "%s" Creset;
   msgs.err.sem.no_such_scene = "attempt of jump to nonexistent Scene " Cbyellow "%s" Creset;
   msgs.err.sem.unreachable = "unconditional goto nullifies statements following it";
}
