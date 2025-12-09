#include <stddef.h>
#include "msg.h"

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

   msgs.err.syn.exit.incomp = "incomplete Exit";
   msgs.err.syn.exit.nochar = "Exit has no character specified";

   msgs.err.syn.exeunt.incomp = "incomplete Exeunt";
   msgs.err.syn.exeunt.onechar = "Exeunt can't have one character";

   //msgs.err.syn.line.name_incomp = "Line has an incomplete name";
   msgs.err.syn.line.incomp = "incomplete Line";
   msgs.err.syn.line.nostmt = "no statement present in Line";

   msgs.err.syn.cnst.incomp = "incomplete constant";
   msgs.err.syn.cnst.deco = "incorrect use of possessive or article";

   //msgs.err.syn.op.badop = "incomprehensible operator";
   msgs.err.syn.op.no_of = "incorrect operator syntax; \"of\" required here";
   msgs.err.syn.op.no_btw = "incorrect operator syntax; \"between\" required here";
   msgs.err.syn.op.incomp = "incomplete operator";
   msgs.err.syn.op.badsyn = "incorrect operator syntax";


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
   msgs.err.syn.gt.misspell = "misspelling detected; it's Scene, not scene";

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
}
