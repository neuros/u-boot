
//#define DMA_TRANSFER 
#define DEBUG_MMC 0
#define DEBUG_MMC_RW 0
#define DEBUG_MMC_COMMAND 0 

	
#if DEBUG_MMC
        #define mmc_debug_msg(fmt, arg...) printf("%s:%d> " \
        fmt, __FUNCTION__, __LINE__ , ## arg)
#else
        #define mmc_debug_msg(fmt, arg...) do { /* NO OP */ } while(0)
#endif

#if DEBUG_MMC_RW
        #define mmc_debug_msg_rw(fmt, arg...) printf("%s:%d> " \
        fmt, __FUNCTION__, __LINE__ , ## arg)
#else
        #define mmc_debug_msg_rw(fmt, arg...) do { /* NO OP */ } while(0)
#endif

#if DEBUG_MMC_COMMAND
        #define mmc_debug_msg_command(fmt, arg...) printf(\
        fmt,## arg)
#else
        #define mmc_debug_msg_command(fmt, arg...) do { /* NO OP */ } while(0)
#endif

