#include "msg.h"
#include "msg.internals.h"

extern void init_msg(void) {
   /* DB Errors */
   msgs.sys.db.archived = "archived database; restore first";
   msgs.sys.db.corrupted = "corrupted database";

   /* Parser Errors */
   msgs.err.syn.eot = "unwelcome end of token";
   msgs.err.syn.incomprehensible = "incomprehensible token";
   msgs.err.syn.name_not_lowcase = "the first letter of this name is not lowercase";

   msgs.err.syn.title.incomp = "incomplete title";

   msgs.err.syn.dp.noname = "no name specified before period";
   msgs.err.syn.dp.chardecl_incomp = "incomplete character declaration";
   msgs.err.syn.dp.desc_incomp = "incomplete character description";
   msgs.err.syn.dp.incomp = "incomplete dramatis personae";
   msgs.err.syn.dp.nonext = "nothing after a character description";

   msgs.err.syn.act.incomp = "incomplete Act";
   msgs.err.syn.act.nornum = "no roman number present";
   msgs.err.syn.act.nocolon = "no colon after Act number";
   msgs.err.syn.act.desc_incomp = "incomplete Act description";
   msgs.err.syn.act.noscene = "no scene seen next";

   msgs.err.syn.scene.incomp = "incomplete Scene";
   msgs.err.syn.scene.nornum = "no roman number present";
   msgs.err.syn.scene.nocolon = "no colon after Scene number";
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
   msgs.err.syn.line.nostmt = "no comprehensible statement present in Line";

   msgs.err.syn.cnst.incomp = "incomplete constant";
   msgs.err.syn.cnst.no_end_symbol = "a constant must be terminated with '.', '!', '?', 'not', 'as', '<comparative>', more, 'less', or 'and'";
   msgs.err.syn.cnst.no_noun = "a constant must have a noun or a noun phrase";

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

   msgs.err.syn.cond.incomp = "incomplete interrogative sentence";
   msgs.err.syn.cond.not_conj = "not conjugated interrogative sentence";
   msgs.err.syn.cond.badsyn = "incorrect interrogative sentence syntax";

   msgs.err.syn.ifstmt.incomp = "incomplete conditional statement";
   msgs.err.syn.ifstmt.badsyn = "incorrect conditional statement syntax";
   msgs.err.syn.ifstmt.conseq_incomp = "incomplete consequent";
   msgs.err.syn.ifstmt.bad_conseq = "incomprehensible consequent";
   msgs.err.syn.ifstmt.conseq_cap = "consequent begins with an uppercase";

   msgs.err.syn.push.incomp = "incomplete push statement";
   msgs.err.syn.push.badsyn = "incorrect push statement syntax";

   msgs.err.syn.pop.incomp = "incomplete pop statement";

   /* Type Errors */
   msgs.err.sem.bad_name = "undefined name";
   msgs.err.sem.bad_adj = "undefined adjective";
   msgs.err.sem.bad_noun = "undefined noun";
   msgs.err.sem.bad_comp = "undefined comparative";
   msgs.err.sem.bad_rnum = "incomprehensible roman numeral";

   /* Context Errors */
   msgs.err.sem.no_such_act = "attempt of jump to nonexistent " Ccyan "Act %s" Creset;
   msgs.err.sem.no_such_scene = "attempt of jump to nonexistent " Ccyan "Scene %s" Creset;

   /* Optimizer Warnings */
   msgs.warn.optimizer.flow = "this adjective causes over/underflow. Stop constant folding.";
}
