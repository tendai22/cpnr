static const char *optable[100];
void init_optable(void)
{
        optable[0] = "m_nop";
        optable[1] = "m_halt";
        optable[2] = "m_colon";
        optable[3] = "m_next";
        optable[4] = "m_run";
        optable[5] = "m_startdoes";
        optable[6] = "m_semi";
        optable[7] = "m_execute";
        optable[16] = "m_literal";
        optable[17] = "m_qbranch";
        optable[18] = "m_branch";
        optable[24] = "m_bytedeposite";
        optable[25] = "m_exclamation";
        optable[26] = "m_bytefetch";
        optable[27] = "m_atfetch";
        optable[32] = "m_rot";
        optable[33] = "m_swap";
        optable[34] = "m_drop";
        optable[35] = "m_over";
        optable[36] = "m_dup";
        optable[37] = "m_rsp";
        optable[38] = "m_r2s";
        optable[39] = "m_s2r";
        optable[40] = "m_r22s";
        optable[41] = "m_s2r2";
        optable[48] = "m_not";
        optable[49] = "m_xor";
        optable[50] = "m_or";
        optable[51] = "m_and";
        optable[52] = "m_div";
        optable[53] = "m_mul";
        optable[54] = "m_sub";
        optable[55] = "m_add";
        optable[56] = "m_gt";
        optable[64] = "m_getch";
        optable[65] = "m_emit";
        optable[66] = "m_space";
        optable[67] = "m_cr";
        optable[68] = "m_period";
        optable[69] = "m_type";
        optable[70] = "m_h";
        optable[71] = "m_last";
        optable[72] = "m_base";
        optable[73] = "m_bl";
        optable[74] = "m_comma";
        optable[75] = "m_create";
        optable[77] = "m_start_colondef";
        optable[78] = "m_end_colondef";
        optable[79] = "m_dictdump";
        optable[80] = "m_docons";
        optable[81] = "m_constant";
        optable[82] = "m_start_compile";
        optable[83] = "m_end_compile";
        optable[84] = "m_add_rsp";
        optable[85] = "m_find";
        optable[86] = "m_compile";
        optable[87] = "m_word";
        optable[88] = "m__state";
}
