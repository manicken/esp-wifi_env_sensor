void SystemSetup();

#if defined(ESP32)
bool InitSD_MMC();
void Start_MDNS();
#endif

void failsafeLoop();