#include<mmcsd/itmmcsd.h>
#include<asm/types.h>
#include<part.h>

#define mb() __asm__ __volatile__ ("" : : : "memory")
#define rmb() mb()
#define wmb() mb()

#define mmc_outb(v,a)	(*((volatile u8  *)(a)) = (v))
#define mmc_outw(v,a)	(*((volatile u16 *)(a)) = (v))
#define mmc_outl(v,a)	(*((volatile u32   *)(a)) = (v))

#define mmc_inb(a)		(*(volatile u8  *)(a))
#define mmc_inw(a)		(*(volatile u16 *)(a))
#define mmc_inl(a)		(*(volatile u32   *)(a))

struct mmc_request;

struct mmc_command {
	//u32			opcode;
	u16			opcode;
	u32			arg;
	//u32			resp[4];
	u16			resp[8];
	u16 			status0;//Manoharan
	u16 			status1;//Manoharan
	u16 			cmd_index;//Manoharan
	
	unsigned int		flags;		/* expected response type */
	
#define MMC_RSP_NONE	(0 << 0)
#define MMC_RSP_SHORT	(1 << 0)
#define MMC_RSP_LONG	(2 << 0)
#define MMC_RSP_MASK	(3 << 0)
#define MMC_RSP_CRC	(1 << 3)		/* expect valid crc */
#define MMC_RSP_BUSY	(1 << 4)		/* card may send busy */

/*
 * These are the response types, and correspond to valid bit
 * patterns of the above flags.  One additional valid pattern
 * is all zeros, which means we don't expect a response.
 */
#if 1  //trying not to use
#define MMC_RSP_R1	(MMC_RSP_SHORT|MMC_RSP_CRC)
#define MMC_RSP_R1B	(MMC_RSP_SHORT|MMC_RSP_CRC|MMC_RSP_BUSY)
#define MMC_RSP_R2	(MMC_RSP_LONG|MMC_RSP_CRC)
#define MMC_RSP_R3	(MMC_RSP_SHORT)
#define MMC_RSP_R6	(MMC_RSP_SHORT|MMC_RSP_CRC)
#endif

/* Response Macros */
#define MMCSD_RSPNONE                   (0x0000)
#define MMCSD_RSP1                      (0x0200)
#define MMCSD_RSP2                      (0x0400)
#define MMCSD_RSP3                      (0x0600)
#define MMCSD_RSP4                       MMCSD_RSP1
#define MMCSD_RSP5                       MMCSD_RSP1
#define MMCSD_RSP6                       MMCSD_RSP1
#define MMCSD_RSP7                       MMCSD_RSP1


	unsigned int		retries;	/* max number of retries */
	unsigned int		error;		/* command error */

#define MMC_ERR_NONE	0
#define MMC_ERR_TIMEOUT	1
#define MMC_ERR_BADCRC	2
#define MMC_ERR_FIFO	3
#define MMC_ERR_FAILED	4
#define MMC_ERR_INVALID	5

	//struct mmc_data		*data;		/* data segment associated with cmd */
	struct mmc_request	*mrq;		/* assoicated request */
};


struct mmc_request {
	struct mmc_command	*cmd;
	//struct mmc_data		*data;
	struct mmc_command	*stop;

	void			*done_data;	/* completion data */
	void			(*done)(struct mmc_request *);/* completion function */
};


static void host_configuration(void);

struct mmc_card *mmc_setup(void);

static void mmc_read_csds(struct mmc_card *);

static void mmc_decode_csd(struct mmc_card *);

static void mmc_decode_cid(struct mmc_card *);

static void mmc_idle_cards(void);

static void mmci_set_ios(char);

struct mmc_card *mmc_discover_cards(unsigned char);

static struct mmc_card *mmc_alloc_card(u16 *, unsigned int *);

static int mmc_send_app_op_cond(u32, u32 *);

static int mmc_send_op_cond(u32, u32 *);

int mmc_wait_for_app_cmd(unsigned int,	struct mmc_command *, int);

int mmc_wait_for_cmd(struct mmc_command *, int);

int mmc_wait_for_req(struct mmc_request *);

void mmc_start_request(struct mmc_request *);

void mmci_start_command(struct mmc_command *,u32);

static void mmci_request(struct mmc_request *);

void mmc_cmd_err(struct mmc_command*,int);

void change_clk25m(void);

unsigned long read_from_card(int dev,unsigned long,lbaint_t,unsigned long *);

void it_mmcsd_get_status(struct mmc_command *);

void it_mmcsd_clear_response_reg(struct mmc_command *);

int stuff_bits (unsigned short *,int,int);

static int mmc_select_card(struct mmc_card *);

int mmc_detect(void);

unsigned int bus_change_to_4_bit(struct mmc_card *);

