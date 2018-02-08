void DACUpdate(INT16* Buffer, INT32 Length);
void DACWrite(INT32 Chip, UINT8 Data);
void DACWrite16(INT32 Chip, INT16 Data);
void DACWrite16Stereo(INT32 Chip, INT16 Data, INT16 Data2);
void DACSignedWrite(INT32 Chip, UINT8 Data);
void DACInit(INT32 Num, UINT32 Clock, INT32 bAdd, INT32 (*pSyncCB)());
void DACSetRoute(INT32 Chip, double nVolume, INT32 nRouteDir);
void DACStereoMode(INT32 Chip);
void DACReset();
void DACExit();
void DACScan(INT32 nAction,INT32 *pnMin);
