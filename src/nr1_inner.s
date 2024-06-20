
COLON:
    m_colon1
    m_jmp     NEXT
SEMI:   
    .dw .+2
    m_semi1
NEXT:   
    m_next1
RUN:    
    m_run
DODOES:
    m_exch
    m_jmp   COLON
