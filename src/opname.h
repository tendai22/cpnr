static const char *optable[100];
void init_optable(void)
{
        optable[0] = "m_nop";
        optable[1] = "m_halt";
        optable[2] = "m_colon";
        optable[3] = "m_next";
        optable[4] = "m_run";
        optable[5] = "m_semi";
        optable[6] = "m_jnz";
        optable[7] = "m_jmp";
        optable[8] = "m__state";
        optable[9] = "m_create";
        optable[10] = "m_key";
        optable[11] = "m_emit";
        optable[12] = "m_not";
        optable[13] = "m_xor";
        optable[14] = "m_or";
        optable[15] = "m_and";
        optable[16] = "m_h";
        optable[17] = "m_last";
        optable[18] = "m_base";
        optable[19] = "m_type";
        optable[20] = "m_space";
        optable[21] = "m_bl";
        optable[22] = "m_cr";
        optable[23] = "m_period";
        optable[24] = "m_div";
        optable[25] = "m_mul";
        optable[26] = "m_sub";
        optable[27] = "m_add";
        optable[28] = "m_rot";
        optable[29] = "m_swap";
        optable[30] = "m_drop";
        optable[31] = "m_over";
        optable[32] = "m_dup";
        optable[33] = "m_comma";
        optable[34] = "m_bytedeposite";
        optable[35] = "m_exclamation";
        optable[36] = "m_bytefetch";
        optable[37] = "m_atfetch";
        optable[38] = "m_literal";
        optable[39] = "m_execute";
        optable[40] = "m_start_colondef";
        optable[41] = "m_end_colondef";
        optable[42] = "m_dictdump";
        optable[43] = "m_docons";
        optable[44] = "m_constant";
        optable[45] = "m_find";
}
