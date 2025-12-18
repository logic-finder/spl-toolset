#include "core.h"

/* The Infamous Hello World Program */

int main(void) {
   int dpsz = 4;
   bool cond = false;
   stage_t *stage = stage_create(dpsz);
   persona_t *personae = init_personae(dpsz);
   memory_t *memories = init_memories(dpsz);

   stage_setname(stage, 0, "Romeo");
   stage_setname(stage, 1, "Juliet");
   stage_setname(stage, 2, "Ophelia");
   stage_setname(stage, 3, "Hamlet");

Act_I:;

Act_I_Scene_I:;
   stage_enter(stage, 3);
   stage_enter(stage, 0);
   assert_offstage(stage, 3);
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 3)] = (2 * 2 * 2 * 2 * 2 * 2 * 1);
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 3)] = ((2 * 2 * 2 * 1)) - ((personae[stage_whoareyou(stage, 3)]));
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 3));
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 3)] = ((2 * 2 * 2 * 2 * 2 * 2 * 2 * 1)) + ((2 * 2 * 2 * 2 * 2 * 2 * 1));
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 3)] = (((2 * 2 * 1)) + ((1))) - ((personae[stage_whoareyou(stage, 3)]));
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 3));
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 3)] = ((personae[stage_whoareyou(stage, 3)])) + (((2 * 2 * 2 * 1)) - ((1)));
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 3));
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 3));
   stage_exit(stage, 0);

Act_I_Scene_II:;
   stage_enter(stage, 1);
   assert_offstage(stage, 3);
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 3)] = (((personae[0])) + ((1))) + ((2 * 1));
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 3));
   stage_exit(stage, 1);

Act_I_Scene_III:;
   stage_enter(stage, 2);
   assert_offstage(stage, 3);
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 3)] = ((2 * 2 * 1)) * ((2 * 2 * 2 * 1));
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 3));
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 3)] = ((2 * 2 * 2 * 1)) * (((1)) + ((2 * 1)));
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 3)] = ((personae[1])) - ((personae[stage_whoareyou(stage, 3)]));
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 3));
   stage_exit(stage, 2);
   stage_exit(stage, 3);

Act_II:;

Act_II_Scene_I:;
   stage_enter(stage, 0);
   stage_enter(stage, 1);
   assert_offstage(stage, 0);
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 0));
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 0)] = ((personae[stage_whoareyou(stage, 0)])) + (((2 * 2 * 1)) - ((1)));
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 0));
   assert_offstage(stage, 1);
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 1));
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 1)] = (personae[3]);
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 1)] = (op_squr(((2 * 1)) - ((2 * 2 * 1)))) - (op_cube((2 * 2 * 1)));
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 1));
   stage_exit(stage, 0);

Act_II_Scene_II:;
   stage_enter(stage, 2);
   assert_offstage(stage, 1);
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 1)] = div(((personae[0])), (((2 * 2 * 1)) + ((1)))).quot;
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 1));
   assert_offstage(stage, 2);
   assert_onlytwo(stage);
   personae[stage_whoareyou(stage, 2)] = div(((personae[0])), ((2 * (((1)) - ((2 * 2 * 1)))))).quot;
   assert_onlytwo(stage);
   io_outc(personae, stage_whoareyou(stage, 2));
   stage_exeunt(stage);

   stage_destroy(stage);
   free(personae);
   cleanup_memories(memories, dpsz);

   return 0;
}
