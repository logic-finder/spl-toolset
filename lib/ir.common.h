#ifndef IR_COMMON_H
#define IR_COMMON_H

/*
 * INTERMEDIATE REPRESENTATION VARIABLES
 *    dpsz   - the number of dramatis personae
 *    teller - the one who is speaking
 *    hearer - the one who is listening
 *    temp1  - temporary variable 1
 *    temp2  - temporary variable 2
 */
typedef enum ir_var {
   Ir_var_dpsz,
   Ir_var_teller,
   Ir_var_hearer,
   // Ir_var_temp1,
   // Ir_var_temp2
} ir_var_t;

/*
 * INTERMEDIATE REPRESENTATION SYNTAX
 *    SET <var> <value>
 *       assigns a value to a variable
 *    ACT <value>
 *       stores the value of the act counter
 *    SCENE <value>
 *       stores the value of the scene counter
 *    ENTER <value>
 *       enters a character
 *    EXIT <value>
 *       exits a character
 *    EXEUNT
 *       exits every onstage character
 */
typedef enum ir_inst {
   Ir_inst_set,
   Ir_inst_mult,
   Ir_inst_act,
   Ir_inst_scene,
   Ir_inst_enter,
   Ir_inst_exit,
   Ir_inst_exeunt,
   Ir_inst_out
} ir_inst_t;

#endif
