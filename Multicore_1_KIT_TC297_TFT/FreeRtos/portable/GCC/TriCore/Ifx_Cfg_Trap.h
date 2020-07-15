void prvTrapYield( int iTrapIdentification );

#define IFX_CFG_CPU_TRAP_SYSCALL_CPU0_HOOK(trapInfo) prvTrapYield((trapInfo).tId)
#define IFX_CFG_CPU_TRAP_SYSCALL_CPU1_HOOK(trapInfo) prvTrapYield((trapInfo).tId)
#define IFX_CFG_CPU_TRAP_SYSCALL_CPU2_HOOK(trapInfo) prvTrapYield((trapInfo).tId)
#define IFX_CFG_CPU_TRAP_SYSCALL_CPU3_HOOK(trapInfo) prvTrapYield((trapInfo).tId)
#define IFX_CFG_CPU_TRAP_SYSCALL_CPU4_HOOK(trapInfo) prvTrapYield((trapInfo).tId)
#define IFX_CFG_CPU_TRAP_SYSCALL_CPU5_HOOK(trapInfo) prvTrapYield((trapInfo).tId)
