
void tlcs90SetReadHandler(UINT8 (*pread)(UINT32));
void tlcs90SetWriteHandler(void (*pwrite)(UINT32, UINT8));
void tlcs90SetReadPortHandler(UINT8 (*pread)(UINT16));
void tlcs90SetWritePortHandler(void (*pwrite)(UINT16, UINT8));

INT32 tlcs90Init(INT32 nCpu, INT32 clock);
void tlcs90Open(INT32 nCpu);
INT32 tlcs90Run(INT32 nCycles);
INT32 tlcs90Reset();
void tlcs90Close();
INT32 tlcs90Exit();

#define TLCS90_IRQSTATUS_NONE	0
#define TLCS90_IRQSTATUS_ACK	1
#define TLCS90_IRQSTATUS_AUTO	2

void tlcs90SetIRQLine(INT32 line, INT32 state);

void tlcs90BurnCycles(INT32 nCpu, INT32 cycles);

#define TLCS90_ROM	1
#define TLCS90_WRITE	2
#define TLCS90_RAM	3

void tlcs90MapMemory(UINT8 *rom, UINT32 start, UINT32 end, INT32 flags);

void tlcs90NewFrame();
void tlcs90RunEnd();
INT32 tlcs90TotalCycles();

INT32 tlcs90Scan(INT32 nAction);
