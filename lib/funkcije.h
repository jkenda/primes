/* Copyright (C) 2019 Jakob Kenda */

unsigned long get_avail_mem() 
{
  long total_memory;
  #ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    total_memory = status.ullTotalPhys;
  #else
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    total_memory = pages * page_size;
  #endif
    return total_memory;
}

char* prettify_size(long bytes) 
{
  static char velikost_enota[10];
  float bytesFloat = (float) bytes;
  char enota[3] = "B";
  if (bytesFloat >= 1000) {
    bytesFloat /= 1000; strcpy(enota, "kB");
  }
  if (bytesFloat >= 1000) {
    bytesFloat /= 1000; strcpy(enota, "MB");
  }
  if (bytesFloat >= 1000) {
    bytesFloat /= 1000; strcpy(enota, "GB");
  }
  sprintf(velikost_enota, "%.2f %s", bytesFloat, enota);
  return velikost_enota;
}

char* grammar(unsigned char stevilo) 
{
  static char beseda[4];
  #if LANGUAGE == sl
    if (stevilo == 2) sprintf(beseda, "sta");
    else if (stevilo == 3 || stevilo == 4) sprintf(beseda, "so");
    else sprintf(beseda, "je");
  #else
    if (stevilo == 1) sprintf(beseda, "is");
    else sprintf(beseda, "are");
  #endif
  return beseda;
}

char* d_h_m_s(int s) 
{
  static char cas[23];
  int d = s / 86400; s %= 86400;
  int h = s / 3600; s %= 3600;
  int m = s / 60; s %= 60;
  if (d > 0)      sprintf(cas, "%dd %dh %dm %ds", d, h, m, s);
  else if (h > 0) sprintf(cas, "%dh %dm %ds", h, m, s);
  else if (m > 0) sprintf(cas, "%dm %ds", m, s);
  else            sprintf(cas, "%ds", s);
  return cas;
}
